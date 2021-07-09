#include "MVC_Layer.h"
#include "GLCore/Core/Input.h"
#include "GLCore/Core/KeyCodes.h"
#include "GLCore/Core/MouseButtonCodes.h"
#include "SymSpell/include/SymSpell.h"
#include "imgui/imgui_internal.h"
#include <thread>

using namespace GLCore;
using namespace GLCore::Utils;

const int g_InitialCapacity = 0;
const int g_MaxEditDistance = 3;
const int g_PrefixLength = 4;
SymSpell g_SymSpell (1, g_MaxEditDistance, g_PrefixLength);
std::vector<SuggestItem> g_Suggestions;
std::vector<std::string> g_LoadedDictionaryLocation;
int g_SuggestionIndex = 0;
struct
{
private:
	char *data = 0;
	uint32_t size = 0;
	uint32_t capacity = 0;
	uint32_t lineNum = 0;
	uint32_t columnNum = 0;
	uint32_t lastWordStartIndex = 0;
	uint32_t lastWordEndIndex = 0;
public:
	char &operator[](uint32_t at)
	{
		return data[at];
	}

	void Resize (uint32_t newCapacity)
	{
		if (size > newCapacity) {
			LOG_WARN ("Cannot Resize, current array size > capacity being resized to");
			return;
		}
		char *newData = new char[newCapacity];
		if (data) {
			memcpy (newData, data, size);
			delete data;
		} else {
			newData[0] = '\0';
		}

		data = newData;
		capacity = newCapacity;
	}
	char *RawData () { return data; }
	void RecalculateSize ()
	{
		lineNum = 0;
		columnNum = 0;
		for (uint32_t i = 0; i < capacity; i++) {
			if (data[i] == '\0') {
				size = i;
				break;
			}
			if (data[i] == '\n')
				lineNum++, columnNum = 0;
			else
				columnNum++;
			LOG_INFO ("cursor In Buff: {0}, {1}; {2} ", lineNum, columnNum, size);
		}

		RecalcLastWord ();
	}
	const uint32_t Size () { return size; }
	const uint32_t LineNum () { return lineNum; }
	const uint32_t ColumnNum () { return columnNum; }
	const uint32_t Capacity () { return capacity; }
	const uint32_t LastWordStartIndex () { return lastWordStartIndex; }
	const uint32_t LastWordEndIndex () { return lastWordEndIndex; }
private:
	void RecalcLastWord ()
	{
		{
			int temp = size - 1;
			lastWordEndIndex = temp > 0 ? temp : 0;
		}
		while (data[lastWordEndIndex] == ' ') {
			if (lastWordEndIndex != 0) {
				lastWordEndIndex--;
			} else
				break;
		};
		lastWordStartIndex = lastWordEndIndex;
		while (data[lastWordStartIndex] != ' ' && data[lastWordStartIndex] != '\n') {
			if (lastWordStartIndex != 0) {
				lastWordStartIndex--;
			} else
				break;
		};
		if (lastWordStartIndex) lastWordStartIndex++;
	}
} g_ResizeableCharBuffer;
std::string g_LastWordFromCharBuffer;
bool g_SelectSuggestion = false;
//std::thread g_WorkerThread;

bool MVC_Layer::s_TextBoxSelected = false;
bool MVC_Layer::s_ShowAllLoadedDictionaries = false;
bool MVC_Layer::s_ResetFocusOnTextbox = true;
ImGuiID MVC_Layer::s_DockspaceID = 0;
MVC_Layer::MVC_Layer()
{
	g_ResizeableCharBuffer.Resize (1024);

	//LoadADictionary("assets/dictionary/frequency_dictionary_en_1000.txt");
}

MVC_Layer::~MVC_Layer()
{
}

void MVC_Layer::OnAttach()
{
	EnableGLDebugging();
}

void MVC_Layer::OnDetach()
{}

void MVC_Layer::OnEvent(Event& event)
{
	EventDispatcher dispatcher(event);
	dispatcher.Dispatch<KeyPressedEvent>(
		[&](KeyPressedEvent& e)
		{
			if (MVC_Layer::s_TextBoxSelected) {
				if ((Input::IsKeyPressed (HZ_KEY_RIGHT_CONTROL) || Input::IsKeyPressed (HZ_KEY_LEFT_CONTROL)) &&
					!(Input::IsKeyPressed (HZ_KEY_RIGHT_SHIFT) || Input::IsKeyPressed (HZ_KEY_LEFT_SHIFT)) &&
					e.GetKeyCode () == HZ_KEY_UP) {
					g_SuggestionIndex--; // clamp
					if (g_SuggestionIndex < 0)
					{
						g_SuggestionIndex = 0;
					}
				}
				if ((Input::IsKeyPressed (HZ_KEY_RIGHT_CONTROL) || Input::IsKeyPressed (HZ_KEY_LEFT_CONTROL)) &&
					!(Input::IsKeyPressed (HZ_KEY_RIGHT_SHIFT) || Input::IsKeyPressed (HZ_KEY_LEFT_SHIFT)) &&
					e.GetKeyCode () == HZ_KEY_DOWN) {
					g_SuggestionIndex++; 
					if (g_SuggestionIndex > g_Suggestions.size ()) {
						g_SuggestionIndex = g_Suggestions.size();
					}
				}
				if (e.GetKeyCode () == HZ_KEY_SPACE)
					g_LastWordFromCharBuffer = "", g_SuggestionIndex = 0;;
				if (e.GetKeyCode () == HZ_KEY_BACKSPACE)
					g_SuggestionIndex = 0;
				
				if (e.GetKeyCode () == HZ_KEY_TAB)
				{
					g_SelectSuggestion = true;
				}
			}
			return false;
		});
}

std::string MVC_Layer::ExtractLastWordFromTextBuffer()
{
	uint16_t lastWordEnd = g_ResizeableCharBuffer.LastWordEndIndex ();
	if (lastWordEnd < g_ResizeableCharBuffer.Size() - 1) {
		return "";
	} else {
		uint16_t lastWordStart = g_ResizeableCharBuffer.LastWordStartIndex ();
		uint16_t count = lastWordEnd - lastWordStart + 1;

		std::string lastWord (&g_ResizeableCharBuffer[lastWordStart], count);
		// LOG_WARN ("[{1}, {2} ({3}, {4})] = '{0}'\n", lastWord, lastWordStart, count, m_ResizeableCharBuffer.LineNum (), m_ResizeableCharBuffer.ColumnNum ());
		return lastWord;
	}
}
void MVC_Layer::OnUpdate (Timestep ts)
{}

void MVC_Layer::OnImGuiRender()
{
	{// DockSpace

		static bool dockspaceOpen = true;
		static constexpr bool optFullscreenPersistant = true;
		bool optFullscreen = optFullscreenPersistant;

		static ImGuiDockNodeFlags dockspaceFlags = ImGuiDockNodeFlags_None;

		// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
		// because it would be confusing to have two docking targets within each others.
		ImGuiWindowFlags windowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
		if (optFullscreen) {
			ImGuiViewport *viewPort = ImGui::GetMainViewport ();
			ImGui::SetNextWindowPos (viewPort->Pos);
			ImGui::SetNextWindowSize (viewPort->Size);
			ImGui::SetNextWindowViewport (viewPort->ID);
			ImGui::PushStyleVar (ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar (ImGuiStyleVar_WindowBorderSize, 0.0f);

			windowFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		}

		// when using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background and handle the pass-thru hole, so we ask Begin() to not render background.
		if (dockspaceFlags & ImGuiDockNodeFlags_PassthruCentralNode)
			windowFlags |= ImGuiWindowFlags_NoBackground;

		// Important: note that we proceed even if Begin() return false (i.e window is collapsed).
		// This is because we want to keep our DockSpace() active. If a DockSpace is inactive,
		// all active windows docked into it will lose their parent and become undocked.
		// any change of dockspce/settings would lead towindows being stuck in limbo and never being visible.
		ImGui::PushStyleVar (ImGuiStyleVar_WindowPadding, ImVec2 (0.0f, 0.0f));
		ImGui::Begin ("Main DockSpace", &dockspaceOpen, windowFlags);
		ImGui::PopStyleVar ();

		if (optFullscreen)
			ImGui::PopStyleVar (2);

		// DockSpace
		ImGuiIO &io = ImGui::GetIO ();
		ImGuiStyle &style = ImGui::GetStyle ();
		float defaultMinWinSize = style.WindowMinSize.x;
		style.WindowMinSize.x = 280;

		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
			s_DockspaceID = ImGui::GetID ("MyDockSpace");
			ImGui::DockSpace (s_DockspaceID, ImVec2 (0.0f, 0.0f), dockspaceFlags);
		}

		style.WindowMinSize.x = defaultMinWinSize;

		// DockSpace's MenuBar
		if (ImGui::BeginMenuBar ()) {
			if (ImGui::BeginMenu ("Main")) {
				// Disabling fullscreen would allow the window to be moved to the front of other windows, 
				// which we can't undo at the moment without finer window depth/z control.
				//ImGui::MenuItem("Fullscreen", NULL, &opt_fullscreen_persistant);
				if (ImGui::MenuItem ("Exit")) 
					Application::Get ().ApplicationClose ();
				
				
				ImGui::EndMenu ();
			}
			MenuBarItems ();
			ImGui::EndMenuBar ();
		}

		// Here goes Stuff that will be put inside DockSpace
		ImGuiRenderDockables ();

		ImGui::End ();
	}
}
void MVC_Layer::MenuBarItems ()
{
	if (ImGui::BeginMenu ("File")) {
	
		if (ImGui::MenuItem ("Load A Dictionary"))
			LoadADictionary ();
		if (ImGui::MenuItem ("Loaded Dictionaries"))
			s_ShowAllLoadedDictionaries = true;
	
		ImGui::EndMenu ();
	}
}
void MVC_Layer::ImGuiRenderDockables ()
{
	ImGui::ShowDemoWindow();

	ImGui::SetNextWindowDockID (s_DockspaceID);
	ImGui::Begin ("Text::Area", NULL);
	MVC_Layer::s_TextBoxSelected = false;
	ImVec2 DrawingTextHere = ImGui::GetCursorPos ();
	if (s_ResetFocusOnTextbox)
	{
		ImGui::SetKeyboardFocusHere ();
		s_ResetFocusOnTextbox = false;
	}
	if (ImGui::InputTextMultiline ("Text_Box", g_ResizeableCharBuffer.RawData (), g_ResizeableCharBuffer.Capacity (), ImGui::GetContentRegionAvail (), ImGuiInputTextFlags_CallbackAlways,
			[](ImGuiInputTextCallbackData *data) -> int
			{
				if (data->EventFlag == ImGuiInputTextFlags_CallbackAlways) {
					LOG_TRACE ("cursor_pos {0}", data->CursorPos);
					MVC_Layer::s_TextBoxSelected = true;
				}
				if (g_SelectSuggestion)
				{
					if (g_SuggestionIndex > 0) {
						int index = g_ResizeableCharBuffer.LastWordStartIndex ();
						for (char charec : g_Suggestions[g_SuggestionIndex - 1].term) {
							g_ResizeableCharBuffer[index] = charec;
							data->Buf[index] = charec;
							index++;
						}
						g_ResizeableCharBuffer[index] = '\0';
						data->Buf[index] = '\0';
						g_ResizeableCharBuffer.RecalculateSize ();
						data->BufTextLen = g_ResizeableCharBuffer.Size ();
						data->CursorPos = data->BufTextLen;
						data->BufDirty = true;
						g_LastWordFromCharBuffer = "";
						g_SuggestionIndex = 0;
					}
					g_SelectSuggestion = false;
				}
				return 0;
			}, NULL)) 
	{
		g_ResizeableCharBuffer.RecalculateSize ();
		//// currently doesn't support buffer resize
		//if (g_ResizeableCharBuffer.Size () > g_ResizeableCharBuffer.Capacity () - 5)
		//{
		//	g_ResizeableCharBuffer.Resize (g_ResizeableCharBuffer.Capacity () + 5);
		//}
		g_LastWordFromCharBuffer = ExtractLastWordFromTextBuffer ();

		// lookup word
		if (g_LastWordFromCharBuffer.size () > 2)
		{
			LookupWordInDictionary (g_LastWordFromCharBuffer);
		}
	}
	{
		static bool show = (g_LastWordFromCharBuffer.size () > 2) && g_Suggestions.size () > 0 && (s_TextBoxSelected || g_SelectSuggestion);
		bool temp_show = (g_LastWordFromCharBuffer.size () > 2) && g_Suggestions.size () > 0 && (s_TextBoxSelected || g_SelectSuggestion);

		if (g_Suggestions.size () > 0) {
			if (g_Suggestions[0].term == g_LastWordFromCharBuffer) {
				temp_show = false;
			}
		}
		if (temp_show != show) {
			show = temp_show;
			if (show) {
				s_ResetFocusOnTextbox = true;
			}
		}
		{
			ImGui::SetNextWindowPos (ImVec2 (12*g_ResizeableCharBuffer.ColumnNum () + ImGui::GetWindowPos ().x + DrawingTextHere.x, 12*(g_ResizeableCharBuffer.LineNum ()+1) + ImGui::GetWindowPos ().y + DrawingTextHere.y));
			if (show) {
				ImGui::Begin ("#Suggestions", NULL, ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_AlwaysAutoResize);
				int i = 0;
				for (SuggestItem &item: g_Suggestions) {
					bool SelectIt = (i == (g_SuggestionIndex - 1));
					ImVec2 startDrawPosn = ImGui::GetCursorScreenPos ();
					startDrawPosn.x -= ImGui::GetCursorPos ().x;
					ImGui::MenuItem (item.term.c_str (), nullptr, &SelectIt);
					ImVec2 endDrawPosn = ImGui::GetCursorScreenPos ();
					endDrawPosn.x += ImGui::GetWindowSize ().x - ImGui::GetCursorPos ().x;
					glm::vec2 mouse_cursor = *(glm::vec2 *)(&Input::GetMousePosition ()) + *(glm::vec2 *)(&ImGui::GetMainViewport ()->Pos);
					if ((startDrawPosn.y < mouse_cursor.y && mouse_cursor.y < endDrawPosn.y) && (startDrawPosn.x < mouse_cursor.x && mouse_cursor.x < endDrawPosn.x) && Input::IsMouseButtonPressed (HZ_MOUSE_BUTTON_1)) {
						g_SuggestionIndex = i + 1;
						int index = g_ResizeableCharBuffer.LastWordStartIndex ();
						for (char charec : g_Suggestions[g_SuggestionIndex - 1].term) {
							g_ResizeableCharBuffer[index] = charec;
							index++;
						}
						g_ResizeableCharBuffer[index] = '\0';
						g_ResizeableCharBuffer.RecalculateSize ();
						g_LastWordFromCharBuffer = "";
						g_SuggestionIndex = 0;
					}
					i++;
					if (i>10)
						break;
				}
			ImGui::End ();
			}
		}
	}

	ImGui::End ();

	if (s_ShowAllLoadedDictionaries) {
		ImGui::Begin ("Dictionaries:", &s_ShowAllLoadedDictionaries, ImGuiWindowFlags_NoCollapse);
		for (std::string& dictLoc: g_LoadedDictionaryLocation)
		{
			uint16_t i = dictLoc.size ();
			while (i)
			{
				i--;

				if (dictLoc[i] == '/' || dictLoc[i] == '\\')
				{
					break;
				}
			}
			if (i) i++;
			ImGui::MenuItem (&dictLoc[i], dictLoc.c_str ());
		}
		ImGui::End ();
	}
}
void MVC_Layer::LoadADictionary ()
{
	std::string filePath = GLCore::Utils::FileDialogs::OpenFile ("all files (*.*)\0*.txt\0");
	if (!filePath.empty ()) {
		int start = clock ();
		g_SymSpell.LoadDictionary (filePath, 0, 1, XL (' '));
		int end = clock ();
		float time = (float)((end - start) / (CLOCKS_PER_SEC / 1000));
		LOG_INFO ("Library Loaded in : {0}", time);
		g_LoadedDictionaryLocation.push_back (filePath);
	}
}
void MVC_Layer::LoadADictionary (std::string filePath)
{
	if (!filePath.empty ()) {
		int start = clock ();
		g_SymSpell.LoadDictionary (filePath, 0, 1, XL (' '));
		int end = clock ();
		float time = (float)((end - start) / (CLOCKS_PER_SEC / 1000));
		LOG_INFO ("Library Loaded in : {0}", time);
		g_LoadedDictionaryLocation.push_back (filePath);
	}
}
void MVC_Layer::LookupWordInDictionary (std::string &word)
{
	if (!g_LoadedDictionaryLocation.empty()) {
		//int start = clock ();
		g_Suggestions = g_SymSpell.Lookup (word, Verbosity::All, g_MaxEditDistance, true);
		//int end = clock ();
		//float time = (float)((end - start) / (CLOCKS_PER_SEC / 1000));
		//LOG_INFO ("lookup took : {0}", time);
	}
}