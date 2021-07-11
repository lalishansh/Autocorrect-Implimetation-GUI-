#include "new_MVC_Layer.h"
#include "GLCore/Core/Input.h"
#include "GLCore/Core/KeyCodes.h"
#include "GLCore/Core/MouseButtonCodes.h"
#include "SymSpell/include/SymSpell.h"
#include "imgui/imgui.h"
#include <thread>

using namespace GLCore;
using namespace GLCore::Utils;

const char *new_MVC_Layer::s_AppDataLocation = "C:\\Autocorrect_Text_Editor_Cache\\cache.txt";

bool new_MVC_Layer::s_Settings_changed = false;
new_MVC_Layer::_Settings new_MVC_Layer::s_Settings_Global = {true};
new_MVC_Layer::_Settings new_MVC_Layer::s_Settings_temp = {true};

void new_MVC_Layer::OnAttach()
{}

void new_MVC_Layer::OnDetach()
{}

void new_MVC_Layer::OnEvent(Event& event)
{}


void new_MVC_Layer::OnUpdate (Timestep ts)
{
	if (Text_Object::s_FocusedTextObject)
		Text_Object::s_FocusedTextObject->OnUpdate ();
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
			m_DockspaceID = ImGui::GetID ("MyDockSpace");
			ImGui::DockSpace (m_DockspaceID, ImVec2 (0.0f, 0.0f), dockspaceFlags);
		}

		style.WindowMinSize.x = defaultMinWinSize;

		// DockSpace's MenuBar
		if (ImGui::BeginMenuBar ()) {
			if (ImGui::BeginMenu ("Main")) {
				Text_Object::s_FocusedTextObject = nullptr;

				if (ImGui::MenuItem ("Style Editor")) 
					m_ShowAppSettingsEditor = true;
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

void new_MVC_Layer::Extending_Dictionary_of (Text_Object *object)
{
	auto filePath = GLCore::Utils::FileDialogs::OpenFile ("all files (*.*)\0*.txt\0"); bool unique = true;

	for (std::string &strs : *object->m_LoadedDictionaries)
		if (strs == filePath) {
			unique = false; break;
		}

	if (unique) {
		if (object->m_Symspell->LoadDictionary (filePath.c_str (), 0, 1, XL (' ')))
			object->m_LoadedDictionaries->push_back (filePath);
	}
}

void new_MVC_Layer::MenuBarItems ()
{
	if (ImGui::BeginMenu ("File")) {
		Text_Object::s_FocusedTextObject = nullptr;

		if(ImGui::MenuItem ("Open"))
			OpenFileAsText(GLCore::Utils::FileDialogs::OpenFile ("all files (*.*)\0*.txt\0").c_str());
		ImGui::EndMenu ();
	}
	ImGui::SameLine ();
	ImGui::Bullet ();

	if (Text_Object::s_FocusedTextObject != nullptr) {
		if (ImGui::BeginMenu ("Dictionary")) {

			if (ImGui::MenuItem ("Add A Dictionary"))
				Extending_Dictionary_of (Text_Object::s_FocusedTextObject);

			if (ImGui::BeginMenu ("Loaded Dictionary")) {

				for (std::string &dict_path : *Text_Object::s_FocusedTextObject->m_LoadedDictionaries)
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

		m_TextFileObjects[i].ImGuiTextRender ();

		ImGui::End ();
		ImGui::PopID ();
	}

	App_Settings  ();
}

void new_MVC_Layer::Save_settings ()
{
	if (s_Settings_changed) {
		/*############################
		### Check & Apply Settings */
		// For unified dictionary
		if (s_Settings_Global.UnifiedDictionary != s_Settings_temp.UnifiedDictionary) {
			s_Settings_Global.UnifiedDictionary = ChangeUnifiedDictionaryState (s_Settings_temp.UnifiedDictionary, *this);
		}

		s_Settings_changed = false;
	}
}
void new_MVC_Layer::Discard_settings_changes ()
{
	s_Settings_temp.UnifiedDictionary = s_Settings_Global.UnifiedDictionary;

	s_Settings_changed = false;
}

void new_MVC_Layer::OpenFileAsText (const char *filePath /*= ""*/)
{
	auto Obj = Text_Object::OpenFileAsText (filePath);
	if (Obj) {
		uint32_t *save_state = new uint32_t[m_TextFileObjects.size () + 1];
		{
			uint32_t i = 0;
			for (auto &obj: m_TextFileObjects) {
				save_state[i] = uint32_t(obj.m_Symspell - &m_Dictionaries[0].first);
				i++;
			}
		}

		m_TextFileObjects.push_back (std::move(Obj.value ()));
		if (s_Settings_Global.UnifiedDictionary || m_TextFileObjects.size () == 1) {
			save_state[m_TextFileObjects.size () - 1] = 0;
		} else { // not unified
			{
				SymSpell tmp1 (1, symspell_Max_Edit_Distance, symspell_Prefix_length
				);
				std::vector<std::string> tmp2;
				m_Dictionaries.push_back ({tmp1, tmp2});
			}
			save_state[m_TextFileObjects.size () - 1] = m_Dictionaries.size ()-1;
		}
		for (uint32_t i = 0; i < m_TextFileObjects.size (); i++)
			m_TextFileObjects[i].m_Symspell = &m_Dictionaries[save_state[i]].first, m_TextFileObjects[i].m_LoadedDictionaries = &m_Dictionaries[save_state[i]].second;
		
		delete[] save_state;
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

				ImGui::EndTabItem ();
			}

			ImGui::EndTabBar ();
		}


		ImGui::End ();
	}
}