#include "new_MVC_Layer.h"
#include "Text_Object.h"

#include "GLCore/Core/Input.h"
#include "GLCore/Core/KeyCodes.h"
#include "GLCore/Core/MouseButtonCodes.h"
#include "SymSpell/include/SymSpell.h"
#include "imgui/imgui.h"
#include <thread>
//#include <Windows.h>

// TODO Have you seen m_Dictionary.push_back, it a good candidate to break code.
// FIX IT

using namespace GLCore;
using namespace GLCore::Utils;

const char *new_MVC_Layer::s_AppDataLocation = "C:\\Autocorrect_Text_Editor_Cache\\cache.txt";

bool new_MVC_Layer::s_Settings_changed = false;
new_MVC_Layer::_Settings new_MVC_Layer::s_Settings_Global = {};
new_MVC_Layer::_Settings new_MVC_Layer::s_Settings_temp = {};

new_MVC_Layer::new_MVC_Layer (int argc, char *argv[])
{
	m_Dictionaries.push_back ({});
	
	if (argc > 1) { // received input

		for (uint32_t i = 1; i < argc; i++) {
			if (argv[i][1] == ':' && (argv[i][2] == '\\' || argv[i][2] == '/')) { // i.e c:\---- absolute path
				OpenFileAsText (argv[i]);
			} else { // relative path
				std::string path = std::filesystem::current_path ().u8string ();
				path += '\\';
				path += argv[i];
				OpenFileAsText (path.data ());
			}
		}

	}
}
void new_MVC_Layer::OnAttach()
{}

void new_MVC_Layer::OnDetach()
{}

void new_MVC_Layer::OnEvent(Event& event)
{
	EventDispatcher dispatcher(event);
	dispatcher.Dispatch<KeyReleasedEvent> (
		[&](KeyReleasedEvent &event) -> bool {
			bool ctrl  = Input::IsKeyPressed (HZ_KEY_RIGHT_CONTROL) || Input::IsKeyPressed (HZ_KEY_LEFT_CONTROL);
			bool shift = Input::IsKeyPressed (HZ_KEY_RIGHT_SHIFT) || Input::IsKeyPressed (HZ_KEY_LEFT_SHIFT);
			
			if (Text_Object::s_FocusedTextObject && 
				(s_Settings_Global.UseCtrlForSuggestionNav ? ctrl : !ctrl)) 
			{
				if(event.GetKeyCode () == HZ_KEY_UP)
						Text_Object::s_FocusedTextObject->OnEvent ('U');
				else if(event.GetKeyCode () == HZ_KEY_DOWN)
						Text_Object::s_FocusedTextObject->OnEvent ('D');
			}
			if (ctrl) { // Shortcuts
				if (event.GetKeyCode () == HZ_KEY_O) {
					Defocus_Text_object ();

					OpenFileAsText (GLCore::Utils::FileDialogs::OpenFile ("all files (*.*)\0*.txt\0*.*\0").c_str ());
				}
				if (Text_Object::s_FocusedTextObject != nullptr) {
					if (event.GetKeyCode () == HZ_KEY_A && shift)
						Extending_Dictionary_of (Text_Object::s_FocusedTextObject);

					else if (event.GetKeyCode () == HZ_KEY_S)
						Text_Object::s_FocusedTextObject->OnEvent ('S');
				}
			}
			return true;
		});
}

static float on_update_autosave_timer = 0.0f;
void new_MVC_Layer::OnUpdate (Timestep ts)
{
	if (Text_Object::s_FocusedTextObject) {

		Text_Object::s_FocusedTextObject->OnUpdate ();

		on_update_autosave_timer  += (s_Settings_Global.autosave_enabled ? ts : 0.0f);
		if (on_update_autosave_timer > s_Settings_Global.autosave_every_in_seconds) {
			on_update_autosave_timer = 0.0f;
			Text_Object::s_FocusedTextObject->OnEvent ('S');
		}
	}
	if (Text_Object::s_SignalHint != NULL) {
		Text_Object::s_SignalPersists_for -= ts;
		if (Text_Object::s_SignalPersists_for < 0.0f)
			Text_Object::s_SignalHint = NULL;
	}
}

void new_MVC_Layer::OnImGuiRender()
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
		// all active windows docked into it will lose their parent and become un-docked.
		// any change of dock-space/settings would lead to windows being stuck in limbo and never being visible.
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
			m_DockspaceID = ImGui::GetID ("MyDockSpace");
			ImGui::DockSpace (m_DockspaceID, ImVec2 (0.0f, 0.0f), dockspaceFlags);
		}

		style.WindowMinSize.x = defaultMinWinSize;

		float width = ImGui::GetContentRegionAvailWidth ();
		// DockSpace's MenuBar
		if (ImGui::BeginMenuBar ()) {
			if (ImGui::BeginMenu ("Main")) {
				Defocus_Text_object ();

				if (ImGui::MenuItem ("Style Editor")) 
					m_ShowAppSettingsEditor = true;
				if (ImGui::MenuItem ("Exit")) 
					Application::Get ().ApplicationClose ();
				
				
				ImGui::EndMenu ();
			}
			MenuBarItems ();

			if (Text_Object::s_SignalHint != NULL) {
				ImGui::SameLine ();
				ImGui::SetCursorPosX (width - 90);
				ImGui::PushStyleVar (ImGuiStyleVar_Alpha, Text_Object::s_SignalPersists_for / Text_Object::s_SignalPersist_Amt);
				ImGui::Button (Text_Object::s_SignalHint);
				ImGui::PopStyleVar ();
			}
			ImGui::EndMenuBar ();
		}

		// Here goes Stuff that will be put inside DockSpace
		ImGuiRenderDockables ();

		ImGui::End ();
	}
}

void new_MVC_Layer::Extending_Dictionary_of (Text_Object *object)
{
	if (!object->m_MyDictionary->Lock) {
		
		std::string filePath = GLCore::Utils::FileDialogs::OpenFile ("all files (*.*)\0*.txt\0");
		if (filePath.empty ()) return;

		std::thread dict_add (
			[](SymSpell_Dictionary *dictionary, std::string filePath) {
				dictionary->Lock = true;
				bool unique = true;

				for (std::string &strs : dictionary->Sources)
					if (strs == filePath) {
						unique = false; break;
					}

				if (unique) {
					if (dictionary->symspell.LoadDictionary (filePath.c_str (), 0, 1, XL (' ')))
						dictionary->Sources.push_back (filePath), Text_Object::SetSignal ("Extended Dictionary");
					else Text_Object::SetSignal ("Dictionary already loaded");
				}

				dictionary->Lock = false;
			}, object->m_MyDictionary, std::move (filePath));
		dict_add.detach ();

		//std::thread test_add (
		//	[]() {
		//		while (true)
		//		{
		//			std::cout << "\nAlive ID: " << this_thread::get_id ();
		//			Sleep (200);
		//		}
		//	});
		//test_add.detach ();

	} else Text_Object::SetSignal ("Dictionary inaccessible");
}

void new_MVC_Layer::MenuBarItems ()
{
	if (ImGui::BeginMenu ("File")) {
		Defocus_Text_object ();

		if(ImGui::MenuItem ("Open"))
			OpenFileAsText(GLCore::Utils::FileDialogs::OpenFile ("all files (*.*)\0*.txt\0*.*\0").c_str());

		if(ImGui::MenuItem ("New"))
			NewUnTitledTextObject ();
		
		ImGui::EndMenu ();
	}
	ImGui::SameLine ();
	ImGui::Bullet ();

	if (Text_Object::s_FocusedTextObject != nullptr) {
		if (ImGui::BeginMenu ("Dictionary")) {

			if (ImGui::MenuItem ("Add A Dictionary"))
				Extending_Dictionary_of (Text_Object::s_FocusedTextObject);

			if (ImGui::BeginMenu ("Loaded Dictionary")) {

				for (std::string &dict_path : Text_Object::s_FocusedTextObject->m_MyDictionary->Sources)
					ImGui::MenuItem (dict_path.c_str ());
				
				ImGui::EndMenu ();
			}

			ImGui::EndMenu ();
		}
		ImGui::SameLine ();
		ImGui::Text ("  Selected: ");
		ImGui::SameLine ();
		ImGui::Text (Text_Object::s_FocusedTextObject->m_FileName);
	}
}
void new_MVC_Layer::ImGuiRenderDockables ()
{
	for (uint32_t i = 0; i < m_TextFileObjects.size (); i++) {
		ImGui::SetNextWindowDockID (m_DockspaceID, (i == 0) ? ImGuiCond_Always : ImGuiCond_Once);
		
		ImGui::PushID (i);
		ImGui::Begin (GLCore::ImGuiLayer::UniqueName (m_TextFileObjects[i].m_FileName), NULL, ImGuiWindowFlags_NoBringToFrontOnFocus); // NOTE: they may have same name

		m_TextFileObjects[i].ImGuiTextRender (s_Settings_Global.UseCtrlForSuggestionNav);

		ImGui::End ();
		ImGui::PopID ();
	}

	App_Settings  ();
}

void new_MVC_Layer::Save_settings ()
{
	if (s_Settings_changed) {
		s_Settings_changed = false;
		/*############################
		### Check & Apply Settings */
		// For unified dictionary
		if (s_Settings_Global.UnifiedDictionary != s_Settings_temp.UnifiedDictionary) {
			s_Settings_Global.UnifiedDictionary = ChangeUnifiedDictionaryState (s_Settings_temp.UnifiedDictionary, *this);
		}
		// For Suggestions
		if (s_Settings_Global.UseCtrlForSuggestionNav != s_Settings_temp.UseCtrlForSuggestionNav) {
			s_Settings_Global.UseCtrlForSuggestionNav  = s_Settings_temp.UseCtrlForSuggestionNav;
		}
		// For AutoSave
		if (s_Settings_Global.autosave_enabled != s_Settings_temp.autosave_enabled) {
			s_Settings_Global.autosave_enabled  = s_Settings_temp.autosave_enabled;
		}
		if (s_Settings_Global.autosave_every_in_seconds != s_Settings_temp.autosave_every_in_seconds) {
			s_Settings_Global.autosave_every_in_seconds  = s_Settings_temp.autosave_every_in_seconds;
		}

	}
}
void new_MVC_Layer::Discard_settings_changes ()
{
	s_Settings_temp.UnifiedDictionary = s_Settings_Global.UnifiedDictionary;
	s_Settings_temp.UseCtrlForSuggestionNav = s_Settings_Global.UseCtrlForSuggestionNav;
	s_Settings_temp.autosave_enabled = s_Settings_Global.autosave_enabled;
	s_Settings_temp.autosave_every_in_seconds = s_Settings_Global.autosave_every_in_seconds;

	s_Settings_changed = false;
}

void new_MVC_Layer::OpenFileAsText (const char *filePath /*= ""*/)
{
	// TODO : Check duplicate, if yes focus on it

	auto Obj = Text_Object::OpenFileAsText (filePath);
	if (Obj) {
		uint32_t *save_state = new uint32_t[m_TextFileObjects.size () + 1];
		{
			uint32_t i = 0;
			for (auto &obj: m_TextFileObjects) {
				save_state[i] = uint32_t(obj.m_MyDictionary - &m_Dictionaries[0]);
				i++;
			}
		}

		m_TextFileObjects.push_back (std::move(Obj.value ()));
		if (s_Settings_Global.UnifiedDictionary || m_TextFileObjects.size () == 1) {
			save_state[m_TextFileObjects.size () - 1] = 0;
		} else { // not unified
			m_Dictionaries.push_back ({});
			save_state[m_TextFileObjects.size () - 1] = m_Dictionaries.size () - 1;
		}
		for (uint32_t i = 0; i < m_TextFileObjects.size (); i++)
			m_TextFileObjects[i].m_MyDictionary = &m_Dictionaries[save_state[i]];
		
		delete[] save_state;
		Text_Object::SetSignal ("Opened File");
	}
}

void new_MVC_Layer::NewUnTitledTextObject ()
{
	{
		uint32_t *save_state = new uint32_t[m_TextFileObjects.size () + 1];
		{
			uint32_t i = 0;
			for (auto &obj: m_TextFileObjects) {
				save_state[i] = uint32_t(obj.m_MyDictionary - &m_Dictionaries[0]);
				i++;
			}
		}

		m_TextFileObjects.push_back (Text_Object("", " ", 2));
		if (s_Settings_Global.UnifiedDictionary || m_TextFileObjects.size () == 1) {
			save_state[m_TextFileObjects.size () - 1] = 0;
		} else { // not unified
			m_Dictionaries.push_back ({});
			save_state[m_TextFileObjects.size () - 1] = m_Dictionaries.size () - 1;
		}
		for (uint32_t i = 0; i < m_TextFileObjects.size (); i++)
			m_TextFileObjects[i].m_MyDictionary = &m_Dictionaries[save_state[i]];
		
		delete[] save_state;
		Text_Object::SetSignal ("New Unsaved File");
	}
}

void new_MVC_Layer::Defocus_Text_object ()
{
	if (Text_Object::s_FocusedTextObject != nullptr) {

		// Other Operations like auto-saving

		// AutoSave
		if (s_Settings_Global.autosave_enabled) {
			Text_Object::s_FocusedTextObject->OnEvent ('S');
			on_update_autosave_timer = 0.0f;
		}

		Text_Object::s_FocusedTextObject = nullptr;
		// LOG_TRACE ("De-Focused");
	}
}

void new_MVC_Layer::App_Settings ()
{
	if (m_ShowAppSettingsEditor) {
		ImGui::Begin ("App Settings", &m_ShowAppSettingsEditor);
		
		if (s_Settings_changed) {

			float draw_width = ImGui::GetContentRegionAvailWidth () - 5;

			if (ImGui::Button ("Save Changes", ImVec2 (draw_width / 2, 30))) Save_settings ();
			ImGui::SameLine ();
			if (ImGui::Button ("Discard Changes", ImVec2 (draw_width / 2, 30))) Discard_settings_changes ();

		} else {
			ImGui::Button ("No Change Occurred", ImVec2 (-1, 30));
		}

		// ImGui::ShowStyleEditor ();
		if (ImGui::BeginTabBar ("Sections")) {
			if (ImGui::BeginTabItem ("Core")) {

				s_Settings_changed |= ImGui::Checkbox ("Unified Dictionary(if unchecked uses seperate dictionary for Every instance)", &s_Settings_temp.UnifiedDictionary);
				s_Settings_changed |= ImGui::Checkbox ("Use Ctrl + Up/Dn instead of just Up/Dn for suggestions navigation", &s_Settings_temp.UseCtrlForSuggestionNav);

				s_Settings_changed |= ImGui::Checkbox ("Enable Auto-Save", &s_Settings_temp.autosave_enabled); ImGui::SameLine ();
				s_Settings_changed |= ImGui::SliderFloat ("Auto-Save Delay", &s_Settings_temp.autosave_every_in_seconds, 1.0f, 100.0f);

				ImGui::EndTabItem ();
			}

			ImGui::EndTabBar ();
		}


		ImGui::End ();
	}
}