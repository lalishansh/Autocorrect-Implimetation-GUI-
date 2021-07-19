#include <GLCore.h>
#include <GLCoreUtils.h>
#include <GLCore/Core/Input.h>
#include <GLCore/Events/KeyEvent.h>
#include <GLCore/Core/KeyCodes.h>

#include "Manager_Layer.h"
#include "Text_Entity.h"

//TODO : GLCore::Utils::OpenFile needs to contain all known file extensions.
using namespace GLCore;

Manager_Layer *Manager_Layer::s_Singleton = nullptr;

Manager_Layer::Manager_Layer (const int argc, char *argv[], char *envv[])
{
	if (argc > 1) {
		std::string newFilePath;
		if (strcmp (argv[1], "new") == 0){ // new file, if argc[2] is available && valid create new file there and open it else create new in same directory with name untitled
			if (argc > 2)
				newFilePath = CreateNewFile (std::string (argv[2]));
			else 
				newFilePath = CreateNewFile ();

		} else if (strcmp (argv[1], "open") == 0) {
			if (argc > 2)
				newFilePath = AbsoluteDirectoryPath (std::string(argv[2])).value_or (std::string());
		} else newFilePath = CreateNewFileIfNotExists (std::string (argv[1]));

		if (!newFilePath.empty ()) {
			LoadTextFileToEntity (std::move (newFilePath));
		}
	}
}
void Manager_Layer::OnEvent (GLCore::Event &event)
{
	EventDispatcher dispatcher (event);
	dispatcher.Dispatch<KeyReleasedEvent> (
		[&](KeyReleasedEvent &e) {
			bool ctrl = Input::IsKeyPressed (HZ_KEY_RIGHT_CONTROL) || Input::IsKeyPressed (HZ_KEY_LEFT_CONTROL);
			bool shift = Input::IsKeyPressed (HZ_KEY_RIGHT_SHIFT) || Input::IsKeyPressed (HZ_KEY_LEFT_SHIFT);

			if (ctrl && e.GetKeyCode () == HZ_KEY_O) {
				std::string tmp = Utils::FileDialogs::OpenFile ("all files (*.*)\0*.txt\0*.*\0");
				LoadTextFileToEntity (std::move(tmp));
			}
			else if (!m_FocusStack.empty () && (m_Settings_main._use_ctrl_for_suggestion_nav == ctrl)) {
				if (ctrl && shift && e.GetKeyCode () == HZ_KEY_A) {
					std::string tmp = Utils::FileDialogs::OpenFile ("all files (*.*)\0*.txt\0*.*\0");
					ExtendDictionaryOf (*(m_TextEntities[m_FocusStack.back ()].MyDictionary ()), tmp);
				}
				if (ctrl && e.GetKeyCode () == HZ_KEY_S)
					m_TextEntities[m_FocusStack.back ()].OnEvent ('S');
			}
			return true;
		});
	dispatcher.Dispatch<KeyPressedEvent> (
		[&](KeyPressedEvent &e) {
			if (!m_FocusStack.empty ()) {
				if (e.GetKeyCode () == HZ_KEY_UP)
					m_TextEntities[m_FocusStack.back ()].OnEvent ('U');
				else if (e.GetKeyCode () == HZ_KEY_DOWN)
					m_TextEntities[m_FocusStack.back ()].OnEvent ('D');
			}
			return true;
		});
	dispatcher.Dispatch<FileDropEvent> (
		[&](FileDropEvent &e) {
			uint8_t *data = new uint8_t[5]; // 4 + 1
			*(uint32_t *)(data) = 0;
			*(bool *)(data + 4) = false; // No

			uint32_t *file_count_ptr = new uint32_t(e.FileCount ()); // Although we can use pointer variable itself as container, things can be different in different builds
			char **all_files = new char *[e.FileCount ()];
			for (uint32_t i = 0; i < e.FileCount (); i++) {
				uint32_t length = strlen (e[i]);
				char *tmp = new char[length + 1];
				memcpy_s (tmp, length + 1, e[i], length + 1); // copied with '\0'
				all_files[i] = tmp;
			}
			m_Signals.emplace (Manager_Layer::file_process_callback, file_count_ptr, all_files, data);
			return true;
		});
}
void Manager_Layer::OnUpdate (GLCore::Timestep ts)
{
	while (!m_Signals.empty ())
		m_Signals.pop (); // process signal
	if (m_SignalHint != nullptr) {// NOTE: nullptr is more type safe that predefined macro NULL
		m_SignalHintPersistsMoreFor -= ts;
		if (m_SignalHintPersistsMoreFor < 0.0f)
			m_SignalHint = nullptr;
	}
	if (!m_FocusStack.empty ()) {
		m_TextEntities[m_FocusStack.back ()].OnUpdate ();
		static float autosave_timer = 0.0f;
		if (m_Settings_main._autosave_enabled) {
			autosave_timer += ts;
			if (autosave_timer > m_Settings_main._autosave_every)
				autosave_timer = 0.0f, m_TextEntities[m_FocusStack.back ()].OnEvent ('S');
		}
	}
}
void Manager_Layer::OnImGuiRender ()
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

		// DockSpace's MenuBar
		float width = ImGui::GetContentRegionAvailWidth ();
		if (ImGui::BeginMenuBar ()) {
			if (ImGui::BeginMenu ("Main")) {

				if (ImGui::MenuItem ("Exit"))
					Application::Get ().ApplicationClose ();
				
				if (ImGui::MenuItem ("Settings"))
					m_SettingsWindowOpen = true;


				ImGui::EndMenu ();
			}
			ImGuiMainMenuBarItems ();
			if (m_SignalHint != NULL) { // little overlay {unlinked for now}
				ImGui::SameLine ();
				ImGui::SetCursorPosX (width - (ImGui::GetFontSize ()*strlen (m_SignalHint)));
				ImGui::PushStyleVar (ImGuiStyleVar_Alpha, m_SignalHintPersistsMoreFor / m_Settings_main._signal_hint_max_persist_durn);
				ImGui::Button (m_SignalHint);
				ImGui::PopStyleVar ();
			}

			ImGui::EndMenuBar ();
		}

		// Here goes Stuff that will be put inside DockSpace
		ImGuiRenderDockables ();
		if (!m_QueryBoxes.empty ()) {
			if (m_QueryBoxes.front ().OnImGuiRender ())
				m_QueryBoxes.pop ();
		}

		ImGui::End ();
	}
}
void Manager_Layer::OnAttach ()
{
	// File History & other checks load
}
void Manager_Layer::OnDetach ()
{
	// File History & other checks save
}
void Manager_Layer::ImGuiRenderDockables ()
{
	for (uint32_t i : m_FocusStack) { // focused one at last
		ImGui::SetNextWindowDockID (m_DockspaceID, (i == m_FocusStack.front ()) ? ImGuiCond_Always : ImGuiCond_Once);
		
		ImGui::PushID (i);
		bool isOpen = true;
		ImGui::Begin (ImGuiLayer::UniqueName (m_TextEntities[i].FileName ()), &isOpen); // ImGuiWindowFlags_NoBringToFrontOnFocus can invite lots of problems

		m_TextEntities[i].OnImGuiRender ();

 		ImGui::End ();
		ImGui::PopID ();
		if (!isOpen) CloseTextEntity (&m_TextEntities[i]);
	}
	SettingsImGuiRender ();
}
void Manager_Layer::ImGuiMainMenuBarItems ()
{
	if (ImGui::BeginMenu ("File")) {
		if (ImGui::MenuItem ("Open")) {
			std::string tmp = Utils::FileDialogs::OpenFile ("All files(*.*)\0*.*\0");
			LoadTextFileToEntity (std::move(tmp));
		}
		if (ImGui::MenuItem ("New")) {
			LoadTextFileToEntity (CreateNewFile ());
		}
		ImGui::EndMenu ();
	}

	ImGui::SameLine ();
	ImGui::Bullet ();
	if (!m_FocusStack.empty ()) {
		ImGui::SameLine ();
		ImGui::PushStyleColor (ImGuiCol_Text, ImVec4 (0.7f, 0.7f, 0.2f, 1.0f));
		ImGui::Text (m_TextEntities[m_FocusStack.back ()].FileName ());
		ImGui::PopStyleColor ();

		if (ImGui::BeginMenu ("Dictionary")) {
			
			if (ImGui::MenuItem ("Extend Dictionary")) {
				std::string tmp = Utils::FileDialogs::OpenFile ("txt files(*.txt)\0*.txt\0");
				ExtendDictionaryOf (*(m_TextEntities[m_FocusStack.back ()].MyDictionary ()), std::move(tmp));
			}
			if (ImGui::BeginMenu ("Loaded Dictionary Sources")) {

				for (std::string &src : m_TextEntities[m_FocusStack.back ()].MyDictionary ()->Sources)
					ImGui::MenuItem (src.c_str ());

				ImGui::PushStyleColor (ImGuiCol_Text, ImVec4 (0.9f, 0.3f, 0.1f, 1.0f));
				if (ImGui::MenuItem ("Clear All")) {
					// TODO : fill me
				}
				ImGui::PopStyleColor ();

				ImGui::EndMenu ();
			}
			ImGui::EndMenu ();
		}
	}
}
void Manager_Layer::SettingsImGuiRender ()
{
	if (m_SettingsWindowOpen) {
		bool _applied = false;
		ImGui::SetNextWindowDockID (m_DockspaceID, ImGuiCond_FirstUseEver);
		ImGui::Begin ("Settings Panel", &m_SettingsWindowOpen);
		if (m_SettingsChanged) {
			float draw_width = ImGui::GetContentRegionAvailWidth() - 5;

			if (ImGui::Button ("Save Changes", ImVec2 (draw_width / 2, 30))) TrySaveSettings (), _applied = true;
			ImGui::SameLine ();
			if (ImGui::Button ("Discard Changes", ImVec2 (draw_width / 2, 30)) || !m_SettingsWindowOpen) DiscardSettings (), _applied = true;

		} else {
			ImGui::Button ("No Changes Occurred", ImVec2 (-1, 30));
		}
		const bool applied = _applied;
		if (ImGui::BeginTabBar ("Sections")) {
			if (ImGui::BeginTabItem ("Core")) {

				m_SettingsChanged |= ImGui::Checkbox ("Unified Dictionary(if unchecked uses seperate dictionary for Every instance)", &m_Settings_temp._unified_dictionary);
				m_SettingsChanged |= ImGui::Checkbox ("Use Ctrl + Up/Dn instead of just Up/Dn for suggestions navigation", &m_Settings_temp._use_ctrl_for_suggestion_nav);
				
				m_SettingsChanged |= ImGui::Checkbox ("Enable Auto-Save", &m_Settings_temp._autosave_enabled); ImGui::SameLine ();
				m_SettingsChanged |= ImGui::SliderFloat ("Auto-Save Delay", &m_Settings_temp._autosave_every, 1.0f, 100.0f);
				
				m_SettingsChanged |= ImGui::SliderInt ("Min Word Count to invoke spell-suggestions", &m_Settings_temp._minWord_length_to_invoke_lookup, 2, 6);

				static uint32_t idx = 0;
				static char *new_extn = new char[12]{0};
				
				ImGui::Text ("Known Extensions:\n");
				if (ImGui::Button ("-")) {
					if (idx > 0) {
						m_Settings_temp.AllowedExtensions.erase (m_Settings_temp.AllowedExtensions.begin () + (idx - 1));
						m_SettingsChanged = true;
						idx = 0;
					}
				}ImGui::SameLine ();
				auto availWid = ImGui::GetContentRegionAvailWidth ();
				{
					float perColumnWid = MAX (ImGui::GetFontSize ()*m_Settings_temp.AllowedExtensions.size (), 100);
					if (ImGui::BeginTable ("Allowed Extensions", availWid/perColumnWid, ImGuiTableFlags_BordersInner | ImGuiTableFlags_NoSavedSettings)) {

						for (uint32_t i = 0, j = 0; i < m_Settings_temp.AllowedExtensions.size (); i++, j++) {
							bool selected = (idx - 1) == i;
							ImGui::TableNextColumn ();
							if (uint16_t ((j*perColumnWid)/availWid) && i < (m_Settings_temp.AllowedExtensions.size () - 1))
								j = 0, ImGui::TableNextRow ();
							
							if (ImGui::Selectable (m_Settings_temp.AllowedExtensions[i].c_str (), &selected, ImGuiSelectableFlags_AllowItemOverlap))
								idx = i + 1;
						}

						ImGui::EndTable ();
					}
				}

				if (ImGui::Button ("+") || (GLCore::Input::IsKeyPressed (HZ_KEY_ENTER) && new_extn[0]!='\0')) {
					std::string valid_str = std::string (new_extn);
					// check valid
					for (char tmp : valid_str)
						if (tmp == '.' || tmp == '\\' || tmp == '/') {// TODO compare with real inputs 
							valid_str.clear (); break;
						}
					for (std::string &tmp : m_Settings_temp.AllowedExtensions)
						if (tmp == valid_str) {
							valid_str.clear (); break;
						}

					if (!valid_str.empty ()) {
						m_Settings_temp.AllowedExtensions.push_back (valid_str);
						m_SettingsChanged = true;
						new_extn[0] = '\0';
					}
				}ImGui::SameLine ();
				ImGui::InputText ("<< ADD", new_extn, 11, ImGuiInputTextFlags_CharsNoBlank | ImGuiInputTextFlags_NoUndoRedo );
				if (applied)
					idx = 0, new_extn[0] = '\0';
				
				m_SettingsChanged |= ImGui::SliderFloat ("Signal Hint Persist Duration", &m_Settings_temp._signal_hint_max_persist_durn, 0.0f, 10.0f);

				ImGui::EndTabItem ();
			}
			ImGui::EndTabBar ();
		}
		ImGui::End ();
	}
}
static void invoke_if_not_equals (void* data, bool &dest, bool &src, void(*func)(void */*data*/, bool/*dest*/, bool/*src*/))
{
	if (dest != src) {
		func (data, dest, src);
		dest = src;
	}
}
void Manager_Layer::TrySaveSettings()
{
	if (m_SettingsChanged) {
		m_SettingsChanged = false;

		invoke_if_not_equals (nullptr, m_Settings_main._unified_dictionary, m_Settings_temp._unified_dictionary,
				[](void *, bool, bool new_state) {
					Manager_Layer &_this = *Manager_Layer::Get ();
					if (_this.m_TextEntities.size () > 1) {
						if (new_state) { // Unify all


							for (Text_Entity &entity : _this.m_TextEntities) {
								entity.m_CancelLookup = true;
								//re-link
								entity.MyDictionary (&_this.Dictionaries().front ());
							}

							// Schedule signals to destroy dictionaries
							for (SymSpell_Dictionary &dict : _this.Dictionaries ())
								if (&dict != &_this.Dictionaries ().front ()) {
									
									for (std::string &str: dict.Sources)
										if (!s_Singleton->m_Dictionaries.front ().IsSourceExist (str)) {
											s_Singleton->ExtendDictionaryOf (s_Singleton->m_Dictionaries.front (), std::move (str));
										}
									_this.SignalQueue ().emplace (Manager_Layer::LazyDestroyDictionary, &dict, nullptr, (uint8_t *)(nullptr));
								}
						} else { // Divide all

							int add_extra = _this.m_TextEntities.size () - _this.m_Dictionaries.size ();
							while (add_extra > 0) {
								Text_Entity &entity = _this.m_TextEntities[_this.m_Dictionaries.size ()];
								entity.m_CancelLookup = true;

								s_Singleton->m_Dictionaries.push_back (s_Singleton->m_Dictionaries.front ());
								
								entity.MyDictionary (&s_Singleton->m_Dictionaries.back ());
								add_extra--;
							}
						}

					} // no need for extra stuff

				});
		//// For Suggestions
		//if (m_Settings_main._use_ctrl_for_suggestion_nav != m_Settings_temp._use_ctrl_for_suggestion_nav) {
			m_Settings_main._use_ctrl_for_suggestion_nav  = m_Settings_temp._use_ctrl_for_suggestion_nav;
		//}
		//// For AutoSave
		//if (m_Settings_main._autosave_enabled != m_Settings_temp._autosave_enabled) {
			m_Settings_main._autosave_enabled  = m_Settings_temp._autosave_enabled;
		//}
		//if (m_Settings_main._autosave_every != m_Settings_temp._autosave_every) {
			m_Settings_main._autosave_every  = m_Settings_temp._autosave_every;
		//}

		// Optimize
		m_Settings_main.AllowedExtensions.clear ();
		for (std::string& str : m_Settings_temp.AllowedExtensions)
		{
			m_Settings_main.AllowedExtensions.push_back (str); // no deep copy required as extensions will be smaller than 15
		}


		//if (m_Settings_main._minWord_length_to_invoke_lookup != m_Settings_temp._minWord_length_to_invoke_lookup) {
			m_Settings_main._minWord_length_to_invoke_lookup  = m_Settings_temp._minWord_length_to_invoke_lookup;
		//}
		//if (m_Settings_main._signal_hint_max_persist_durn != m_Settings_temp._signal_hint_max_persist_durn) {
			m_Settings_main._signal_hint_max_persist_durn  = m_Settings_temp._signal_hint_max_persist_durn;
		//}
	}
}
void Manager_Layer::DiscardSettings ()
{
	m_SettingsChanged = false;
	m_Settings_temp._unified_dictionary = m_Settings_main._unified_dictionary;
	m_Settings_temp._autosave_enabled = m_Settings_main._autosave_enabled;
	m_Settings_temp._autosave_every = m_Settings_main._autosave_every;
	m_Settings_temp._minWord_length_to_invoke_lookup = m_Settings_main._minWord_length_to_invoke_lookup;
	m_Settings_temp._signal_hint_max_persist_durn = m_Settings_main._signal_hint_max_persist_durn;
	m_Settings_temp._use_ctrl_for_suggestion_nav = m_Settings_main._use_ctrl_for_suggestion_nav;

	m_Settings_temp.AllowedExtensions.clear ();
	for (std::string &str : m_Settings_main.AllowedExtensions) {
		m_Settings_temp.AllowedExtensions.push_back (str);
	}
}
bool Manager_Layer::IsFocused (Text_Entity *ptr)
{
	MY_ASSERT (uint32_t (ptr - &s_Singleton->m_TextEntities[0]) < s_Singleton->m_FocusStack.size ());
	return s_Singleton->m_FocusStack.back () == uint32_t(ptr - &s_Singleton->m_TextEntities[0]);
}
void Manager_Layer::ChangeFocusTo (Text_Entity *ptr)
{
	uint32_t focus = uint32_t (ptr - &s_Singleton->m_TextEntities[0]);
	MY_ASSERT (focus <= s_Singleton->m_FocusStack.size ());// don't worry it'll wrap around

	for (uint32_t i = s_Singleton->m_FocusStack.size (); i > 0; i--)
		if ((i - 1) == focus) {
			s_Singleton->m_FocusStack.erase (s_Singleton->m_FocusStack.begin () + (i - 1));
			break;
		}
	s_Singleton->m_FocusStack.push_back (focus);
}
void Manager_Layer::CloseTextEntity (Text_Entity *ptr)
{
	uint32_t idx = uint32_t (ptr - &m_TextEntities[0]);
	void *dict = m_TextEntities[0].MyDictionary ();
	LazyDestroyDictionary (dict, nullptr, (uint8_t *)(nullptr), true);
	MY_ASSERT (idx == m_FocusStack.back());
	m_TextEntities.erase (m_TextEntities.begin () + idx);
	m_FocusStack.pop_back ();

}
void Manager_Layer::SetSignalHint (char *hint)
{
	m_SignalHint = hint;
	m_SignalHintPersistsMoreFor = m_Settings_main._signal_hint_max_persist_durn;
}
void Manager_Layer::RaiseQuery (std::string message, std::vector<std::pair<std::string, bool>> options, void *data1, void *data2, void(*static_callbck)(void *, void *, uint8_t *, bool))
{
	m_QueryBoxes.emplace (&m_Signals, std::move (message), std::move (options), data1, data2, static_callbck);
}
void Manager_Layer::file_process_callback (void *_file_count, void *_all_files, uint8_t *option_data, bool is_valid_signal)
{
	if (!is_valid_signal) {
		uint32_t count = *(uint32_t *)(_file_count);
		char** all_files = (char **)(_all_files);
		for (uint32_t i = 0; i < count; i++) {
			MY_ASSERT (all_files[i] != nullptr);
			delete[] all_files[i];
			all_files[i] = nullptr;
		}

		delete (uint32_t *)(_file_count);
		delete[] all_files;
		delete[] option_data;
	}
	bool dont_load_all = true, load_as_dictionary = false;
	const uint32_t option_count = *(uint32_t *)(option_data);

	char **all_files = (char **)(_all_files);
	std::vector<char *> process_files;
	
	{// Special cases
		bool load_first = *(bool *)(option_data + 4);
		if (!load_first) {
			if(option_count != 0){ // file cannot be loaded
				(*(uint32_t *)(_file_count))--;
				delete[] all_files[0];
				all_files[0] = nullptr;
				all_files = &all_files[1];
			}
		} else { // file needs to be loaded
			process_files.push_back (all_files[0]);
			all_files[0] = nullptr;
		}

	}

	const uint32_t file_count = *(uint32_t *)(_file_count);
	
	dont_load_all = (option_count > 0 ? !*(bool *)(option_data + 5) : dont_load_all); // not load_all
	load_as_dictionary = (option_count > 1 ? *(bool *)(option_data + 6) : load_as_dictionary);

	for (uint32_t idx = process_files.size (); idx < file_count; idx++) {
		if (dont_load_all) {
			std::string extn = GetFileExtension (all_files[idx]);
			std::vector<std::string> &all_extensions = Manager_Layer::Get ()->m_Settings_main.AllowedExtensions;
			bool good_extn = false;
			for (std::string &good : all_extensions)
				if (extn == good) {
					good_extn = true; break;
				}

			if (!good_extn)
				break;
		}

		// If reached here, load into process_files
		process_files.push_back (all_files[idx]);
		all_files[idx] = nullptr; // change owner
	}

	const uint32_t num_process_files = process_files.size ();

	while (!process_files.empty ()) {
		if (load_as_dictionary) {
			std::string tmp = std::move (process_files.back ());
			Manager_Layer::Get ()->ExtendDictionaryOf (
				*(Manager_Layer::Get ()->m_TextEntities[Manager_Layer::Get ()->m_FocusStack.back ()].MyDictionary()), 
				std::move(tmp));
		} else {
			std::string tmp = std::move (process_files.back ());
			Manager_Layer::Get ()->LoadTextFileToEntity (std::move(tmp));
		}
		process_files.pop_back (); // ambiguous pointer
	}
	const uint32_t num_files_more_to_process = file_count - num_process_files;
	if (num_files_more_to_process > 0) {
		char **next_files = new char *[num_files_more_to_process];
		for (uint32_t i = 0; i < num_files_more_to_process; i++) {
			next_files[i] = all_files[num_process_files + i];
			all_files[num_process_files + i] = nullptr; // change owner
		}
		*(uint32_t *)(_file_count) = num_files_more_to_process;
		Manager_Layer::Get ()->RaiseQuery ("Trying to load file with unknown extension.\nLoad Anyway? (TIP: Try updating allowed extension)", { {"Load All File Anyway.", !dont_load_all}, {"Load As Dictionary.", load_as_dictionary} }, _file_count, next_files, Manager_Layer::file_process_callback);
	} else {
		delete (uint32_t *)(_file_count);
		_file_count = nullptr;
	}

#ifdef MODE_DEBUG
	for (uint32_t i = 0; i < file_count; i++)
		MY_ASSERT (all_files[i] == nullptr); // check whether anyones left
#endif // _DEBUG

	delete[] (char**)_all_files;
	delete[] option_data;
}
bool Manager_Layer::LoadTextFileToEntity (std::string filePath) // When loading file, keep min " " in side the buffer otherwise ImGui will complain
{
	for (Text_Entity &entity: m_TextEntities) {
		if (filePath == entity.m_FilePathOnDisk) // Duplicate
			return false;
	}
	SymSpell_Dictionary *dict_ptr;
	if (m_Settings_main._unified_dictionary && !m_Dictionaries.empty ()) {
		dict_ptr = &m_Dictionaries.front ();
	} else {
		m_Dictionaries.push_back ({});
		dict_ptr = &m_Dictionaries.back ();
	}
	thread file_addn (
		[](std::vector<Text_Entity> *entities, std::string filePath, SymSpell_Dictionary *dictnry) {
			std::string result;
			std::ifstream in (filePath, std::ios::in | std::ios::binary);
			if (in) {
				in.seekg (0, std::ios::end);
				size_t size = size_t (in.tellg ());

				// Doesn't matter if empty
				result.resize (size + 1); // also changes size

				in.seekg (0, std::ios::beg);
				in.read (&result[0], size);

				entities->emplace_back (std::move (result), std::move (filePath));
				entities->back ().MyDictionary (dictnry);
				Manager_Layer::ChangeFocusTo (&entities->back ());
			} else {
				Manager_Layer::LazyDestroyDictionary (dictnry, nullptr, (uint8_t *)(nullptr), true);
			}
		}, &m_TextEntities, std::move (filePath), dict_ptr);
	file_addn.detach ();
	return true;
}

void Manager_Layer::ExtendDictionaryOf(SymSpell_Dictionary& handle, std::string from_FilePath)
{
	if (handle.IsSourceExist (from_FilePath))
		return; // already done
	if (handle.Lock) {
		std::string *tmp = new std::string;
		*tmp = std::move (from_FilePath);
		m_Signals.emplace (
			[](void *dictn, void *data, uint8_t *, bool is_Valid) {
				if (!is_Valid) {
					delete (std::string *)data;
					return;
				}
				s_Singleton->ExtendDictionaryOf (*(SymSpell_Dictionary *)dictn, std::move(*((std::string *)data)));
				delete (std::string *)data;
			}, &handle, tmp, (uint8_t *)nullptr);
		return;
	}

	handle.Lock = true;
	thread dict_addn (
		[](SymSpell_Dictionary *dictionary, std::string source) {

			dictionary->Sources.push_back (std::move (source));

			if (!dictionary->symspell.LoadDictionary (dictionary->Sources.back ().c_str (), 0, 1, XL (' ')))
				dictionary->Sources.pop_back ();
			dictionary->Lock = false; // release lock
			
		}, &handle, std::move (from_FilePath));
	dict_addn.detach ();
}

// Completely Destroys if no text entity is linked
// if one is linked Replaces OldDictionary with Brand NewDictionary
// if more than one are linked checks options_data, if nullptr then raises prompt or query
// else checks option_val(yes/no) if true clears, if no exits
void Manager_Layer::DestroyDictionary (void* _dict_location, void* dummy, uint8_t* options_data, bool signal_is_valid)
{
	SymSpell_Dictionary *dict = (SymSpell_Dictionary *)_dict_location;
	
	(dummy != nullptr);

	if (!signal_is_valid) { 
		delete[] options_data; 
		return;
	}

	if (dict->Lock) { // Locked try again later
		s_Singleton->m_Signals.emplace (Manager_Layer::DestroyDictionary, _dict_location, dummy, options_data);
		return;
	}
	dict->Lock = true;
	
	uint32_t link_count = 0;
	for (Text_Entity &entity : s_Singleton->m_TextEntities)
		link_count = entity.MyDictionary () == dict ? link_count + 1 : link_count;
	
	if (link_count > 1) {
		if (options_data != nullptr) {
			bool replace = *((bool *)(options_data + 4));
		#ifdef MODE_DEBUG
			uint32_t count = *((uint32_t *)options_data);
		#endif
			MY_ASSERT (count == 0);
			if (replace)
					*dict = SymSpell_Dictionary ();
			// else do nothing, operation canceled
		} else {
			dict->Lock = false;
			s_Singleton->RaiseQuery ("More than One Text files opened are linked to this dictionary.\nClear Dictionary Anyway?", {}, nullptr, _dict_location, Manager_Layer::DestroyDictionary);
		}
	} else if (link_count > 0)
		*dict = SymSpell_Dictionary ();
	else {
		auto iterator = s_Singleton->m_Dictionaries.begin ();
		for (SymSpell_Dictionary &this_dict : s_Singleton->m_Dictionaries) {
			if (&this_dict == dict) {
				s_Singleton->m_Dictionaries.erase (iterator);
				break;
			}
			std::advance (iterator, 1);
		}
	}
	// delete
	s_Singleton->SignalInvalidator (_dict_location);
	delete[] options_data;
}
void Manager_Layer::LazyDestroyDictionary (void *_dict_location, void* dummy1, uint8_t* dummy2, bool signal_is_valid)
{
	SymSpell_Dictionary *dict = (SymSpell_Dictionary *)_dict_location;

	if (!signal_is_valid) return;

	MY_ASSERT (_dict_location != nullptr); MY_ASSERT (dummy1 == nullptr); MY_ASSERT (dummy2 == nullptr);
	
	if (dict->Lock) {
		s_Singleton->m_Signals.emplace (Manager_Layer::LazyDestroyDictionary, _dict_location, nullptr, (uint8_t *)(nullptr));
		return;
	}
	dict->Lock = true;

	bool is_linked = false;
	for (Text_Entity &entity : s_Singleton->m_TextEntities)
		if (entity.MyDictionary () == dict) {
			is_linked = true, dict->Lock = false;
			break;
		}
	if (!is_linked) {
		auto iterator = s_Singleton->m_Dictionaries.begin ();
		for (SymSpell_Dictionary &this_dict : s_Singleton->m_Dictionaries) {
			if (&this_dict == dict) {
				s_Singleton->m_Dictionaries.erase (iterator);
				break;
			}
			std::advance (iterator, 1);
		}
	}
	s_Singleton->SignalInvalidator (_dict_location);
}
void Manager_Layer::SignalInvalidator (void *signals_with_data)
{
	uint32_t size = m_Signals.size ();
	while (size > 0) {
		if (m_Signals.front ().Has (signals_with_data))
			m_Signals.front ().Invalidate ();
		else m_Signals.push (std::move(m_Signals.front ()));
		
		m_Signals.pop ();

		size--;
	}
}