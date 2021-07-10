#pragma once

#include <GLCore.h>
#include <GLCoreUtils.h>
#include <imgui/imgui.h>
#include "Text_Object.h"
#include <filesystem>
#include "Settings_editor_methods.h"

// Singleton
class new_MVC_Layer : public GLCore::Layer
{
public:
	new_MVC_Layer (int argc, char *argv[])
	{
		{
			SymSpell tmp1 (1, 3, 4);
			std::vector<std::string> tmp2;
			m_Dictionaries.push_back ({tmp1, tmp2});
		}
		if (argc > 1) { // received input
			
			for (uint32_t i = 1; i < argc; i++) {
				if (argv[i][1] == ':' && (argv[i][2] == '\\' || argv[i][2] == '/')) { // i.e c:\---- absolute path
					OpenFileAsText (argv[i]);
				} else { // relative path
					std::string path = std::filesystem::current_path ().u8string ();
					path+='\\';
					path+=argv[i];
					OpenFileAsText (path.data ());
				}
			}

		}
	}
	~new_MVC_Layer () = default;

	virtual void OnAttach() override;
	virtual void OnDetach() override;
	virtual void OnEvent(GLCore::Event& event) override;
	
	virtual void OnUpdate (GLCore::Timestep ts) override;
	virtual void OnImGuiRender() override;

	void OpenFileAsText (const char* filePath = "");
	void App_Settings ();
	// void HomeScreen () {}
private:
	void ImGuiRenderDockables ();
	void MenuBarItems ();
	void Save_settings ();

	void Discard_settings_changes ();
private:
	bool m_ShowAppSettingsEditor = false;


	ImGuiID m_DockspaceID;
	ImGuiStyle myStyle;
	std::vector<Text_Object> m_TextFileObjects;
	std::vector<std::pair<SymSpell, std::vector<std::string>>> m_Dictionaries;
	// std::vector<std::string> m_FileHistory; // every-time any file is opened, History updates and then immediately saved on disk
	static const char *s_AppDataLocation;

	struct _Settings
	{
		bool UnifiedDictionary = false;
	};
	static _Settings s_Settings_Global, s_Settings_temp;

	static bool s_Settings_changed;
private:
	// settings manuplator
	friend bool ChangeUnifiedDictionaryState (const bool, new_MVC_Layer &);
};