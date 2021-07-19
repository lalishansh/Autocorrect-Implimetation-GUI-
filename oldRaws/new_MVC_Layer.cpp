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

// TODO Add multiple file support for opening, extending_dictionary (not saving)

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
				OpenFileAsText (&argv[i], 1);
			} else { // relative path TODO Improve rectification
				std::string path = std::filesystem::current_path ().u8string ();
				path += '\\';
				path += argv[i];
				OpenFileAsText (&path.data (), 1);
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

					OpenFileAsText (&GLCore::Utils::FileDialogs::OpenFile ("all files (*.*)\0*.txt\0*.*\0").c_str (), 1);
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
	dispatcher.Dispatch<FileDropEvent> (
		[&](FileDropEvent &e) {
			LOG_TRACE ("File Count: {0}", e.FileCount ());
			uint32_t i = 0;
			while (e[i] != nullptr) {
				std::cout << "#" << i << "  " << e[i] << "\n";
				m_File_Queue.push (e[i]); 
				i++;
			}
			// check extn
			std::string &filepath = m_File_Queue.front ();
			i = filepath.size () - 1;
			while (filepath[i] != '.' && filepath[i] != '\\' && filepath[i] != '/')
				i--;
			std::string extn = &filepath[i + 1];
			bool not_allowNxtFileLoad = true;
			for (const char *c_str : s_Settings_Global.AllowedExtensions)
				if (extn == c_str) {
					not_allowNxtFileLoad = false; break;
				}
			bool *newbool = new bool[2]{0};
			//
			if (not_allowNxtFileLoad) // raise query
				if(m_TextFileObjects.empty ())
					RaiseQuery ("The file being loaded is of unspecified extension, Load Anyway?", { "Apply to All" }, this, new_MVC_Layer::LoadFileCallback);
				else 
					RaiseQuery ("The file being loaded is of unspecified extension, Load Anyway?", { "Apply to All", "Load As Dictionary" }, this, new_MVC_Layer::LoadFileCallback);
			else if(m_TextFileObjects.empty ()) new_MVC_Layer::LoadFileCallback (this, false, 1, newbool);
			else newbool[1] = true, newbool[0] = true, RaiseQuery ("For the files being loaded", { "Apply to All", "Load As Dictionary" }, this, new_MVC_Layer::LoadFileCallback, newbool);
			delete[] newbool;
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

		static constexpr bool optFullscreenPersistant = true;
		static bool dockspaceOpen = true;
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
		ImGuiRenderQueryBox  ();

		ImGui::End ();
	}


}

void new_MVC_Layer::ImGuiRenderQueryBox ()
{
	// Query Box
	if (!m_UserQueries.empty ()) {
		ImGui::OpenPopup (ImGui::GetID ("User Query Window"));
	}
	if (ImGui::BeginPopupModal ("User Query Window", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize)) {
		
		UserQuery &currQuery = m_UserQueries.front ();
		
		ImGui::Text (currQuery.Message ());
		if (currQuery.OptionSize () > 0) {
			for (uint32_t i = 0; i < currQuery.OptionSize (); i++) {
				auto [c_str, boolptr] = currQuery.OptionAt (i);
				ImGui::Checkbox (c_str, boolptr);
			}
		}

		float width = ImGui::GetContentRegionAvailWidth () / 2; bool launchedCallback = false;
		if (ImGui::Button ("Yes", ImVec2 (width - 5, 0))) {
			currQuery.Callback (true);
			launchedCallback = true;
		}
		ImGui::SameLine ();
		if (ImGui::Button ("No", ImVec2 (width - 5, 0))) {
			currQuery.Callback (false);
			launchedCallback = true;
		}

		if (launchedCallback) { m_UserQueries.front ().Clear (); m_UserQueries.pop (); }
		ImGui::EndPopup ();
	}
}
bool new_MVC_Layer::Extending_Dictionary_of(Text_Object *object, const char **filePath_c_str /*= nullptr*/, uint32_t count /*= 0*/)
{
	std::vector<std::string_view> filePaths_view;
	if (filePath_c_str) {
		filePaths_view.reserve (count);
		while (count > 0) {
			filePaths_view.emplace_back (std::string_view (filePath_c_str[count]));
			count--;
		}
	} else filePaths_view.emplace_back (GLCore::Utils::FileDialogs::OpenFile ("all files (*.*)\0*.txt\0"));
	
	for(uint32_t i = filePaths_view.size () - 1; i >= 0; i--)
		if (filePaths_view[i].empty ()) filePaths_view.erase (filePaths_view.begin () + i); // removing empty ones
	if (filePaths_view.empty ()) return;

	std::vector<std::string> filePaths;
	filePaths.reserve (filePaths_view.size ());
	for (std::string_view item: filePaths_view)
		filePaths.push_back (item);

	object->QueueDictExtensionSrcs (filePaths_view);
}

void new_MVC_Layer::MenuBarItems ()
{
	if (ImGui::BeginMenu ("File")) {
		Defocus_Text_object ();

		if(ImGui::MenuItem ("Open"))
			OpenFileAsText(&GLCore::Utils::FileDialogs::OpenFile ("all files (*.*)\0*.txt\0*.*\0").c_str(), 1);

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

void new_MVC_Layer::OpenFileAsText(const char **filePaths, const uint32_t count)
{

	// TODO : refine this
	uint16_t *save_state = new uint32_t[m_TextFileObjects.size () + count];
	{
		uint32_t i = 0;
		for (auto &obj: m_TextFileObjects) {
			save_state[i] = uint16_t(int(obj.m_MyDictionary - &m_Dictionaries[0]));
			i++;
		}
	}
	const uint32_t initial_size = m_TextFileObjects.size ();

	for (uint32_t i = 0; i < count; i++) {
		const char *filePath = filePaths[i];
		{
			// Check duplicate, if yes focus on it
			bool duplicate = false;
			for (uint32_t i = 0; i < initial_size; i++)
				if (m_TextFileObjects[i].m_FilePath == filePath) {
					Text_Object::s_FocusedTextObject = &m_TextFileObjects[i];
					Text_Object::s_LastFocusedTextObject = Text_Object::s_FocusedTextObject;
					Text_Object::s_FocusedTextObject->m_ResetFocus = true;
					duplicate = true;
					break;
				}
			if (duplicate) continue;
		}

		std::string result;
		std::ifstream in (filePath, std::ios::in | std::ios::binary);
		if (in) {
			in.seekg (0, std::ios::end);
			size_t size = size_t (in.tellg ());

			// Doesn't matter if empty
			result.resize (size + 1); // also changes size

			in.seekg (0, std::ios::beg);
			in.read (&result[0], size);
		} else {
			return;
			//LOG_ERROR ("Unable To Open File '{0}'", filePath);
		}
		m_TextFileObjects.emplace_back (filePath, result.c_str (), result.size ());
		if (s_Settings_Global.UnifiedDictionary || initial_size == 1) {
			save_state[m_TextFileObjects.size () - 1] = 0;
		} else { // not unified
			m_Dictionaries.push_back ({});
			save_state[m_TextFileObjects.size () - 1] = m_Dictionaries.size () - 1;
		}
	}
	
	for (uint32_t i = 0; i < m_TextFileObjects.size (); i++)
		m_TextFileObjects[i].m_MyDictionary = &m_Dictionaries[save_state[i]];

	delete[] save_state;
	Text_Object::SetSignal ("Opened File");
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
		Text_Object::s_LastFocusedTextObject = Text_Object::s_FocusedTextObject;
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

void new_MVC_Layer::RaiseQuery(const char *message, std::vector<std::string_view> query_options, void *target_object, void(*callback_func)(void *, bool, uint32_t, const bool *), const bool* default_state)
{
	MY_ASSERT (callback_func != nullptr) //{ LOG_ERROR ("Query Callback Function is NULLPTR"); }
	m_UserQueries.push (UserQuery::Create (this, callback_func, message, std::move(query_options), default_state));
}
// Query Functions

void new_MVC_Layer::LoadFileCallback(void* target_obj, bool yes_no, uint32_t option_count, const bool *option_state)
{
	new_MVC_Layer &this_obj = *((new_MVC_Layer*)(target_obj));
	bool dont_load_all = true, loadAsDictionary = false;
	if (option_state) {
		// 1st Option
		dont_load_all = !*option_state;
		loadAsDictionary = !*(option_state + 1);
	}
	if (!this_obj.m_File_Queue.empty () && !yes_no && count != -1) // little Hack so that i can bypass this step on 1st iteration
		this_obj.m_File_Queue.pop ();

	std::vector<char *> filePaths;
	while (!this_obj.m_File_Queue.empty ()) {
		std::string& filepath = this_obj.m_File_Queue.front ();
		
		if(dont_load_all){
			// check if its desired extn
			uint32_t i = filepath.size () - 1;
			while (filepath[i] != '.' && filepath[i] != '\\' && filepath[i] != '/')
				i--;
			std::string extn = &filepath[i + 1];
			bool not_allowNxtFileLoad = true;
			for (const char *c_str : s_Settings_Global.AllowedExtensions)
				if (extn == c_str) {
					not_allowNxtFileLoad = false; break;
				}
			if (not_allowNxtFileLoad && !yes_no) // yes then this part will fall through to load file				
				break;
			yes_no = false;
		}

		// Load File
		char *path = std::move (filepath);
		filePaths.push_back (path);

		this_obj.m_File_Queue.pop ();
	}

	// Operate
	if (loadAsDictionary)
		Extending_Dictionary_of (Text_Object::s_LastFocusedTextObject, filePaths.data (), filePaths.size ());
	else this_obj.OpenFileAsText (filePaths.data (), filePaths.size ());

	if (!this_obj.m_File_Queue.empty ()) {// Relaunch Query
		this_obj.RaiseQuery ("The file being loaded is of unspecified extension, Load Anyway?", { "Apply to All" }, target_obj, new_MVC_Layer::LoadFileCallback);
	}
}