#pragma once

#include <GLCore.h>
#include <GLCoreUtils.h>
#include <imgui/imgui.h>
#include <filesystem>
#include "Settings_editor_methods.h"

class Text_Object;
class SymSpell;
// Singleton
class new_MVC_Layer : public GLCore::Layer
{
public:
	new_MVC_Layer (int argc, char *argv[]);
	~new_MVC_Layer () = default;

	virtual void OnAttach() override;
	virtual void OnDetach() override;
	virtual void OnEvent(GLCore::Event& event) override;
	
	virtual void OnUpdate (GLCore::Timestep ts) override;
	virtual void OnImGuiRender() override;

	static void Extending_Dictionary_of (Text_Object *object);
	void OpenFileAsText (const char *filePath = "");
	void App_Settings ();
	static void Defocus_Text_object ();
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
		bool UnifiedDictionary       = true;
		bool UseCtrlForSuggestionNav = true;
	};
	static _Settings s_Settings_Global, s_Settings_temp;

	static bool s_Settings_changed;
private:
	// settings manuplator
	friend bool ChangeUnifiedDictionaryState (const bool, new_MVC_Layer &);
};