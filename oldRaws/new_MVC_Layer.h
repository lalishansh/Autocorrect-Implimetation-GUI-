#pragma once

#include <GLCore.h>
#include <GLCoreUtils.h>
#include <imgui/imgui.h>
#include <filesystem>
#include <queue>
#include "Commons.h"

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

	void ImGuiRenderQueryBox ();

	static bool Extending_Dictionary_of (Text_Object *object, const char **filePath_c_str = nullptr, uint32_t count = 0);
	void OpenFileAsText (const char **filePaths, const uint32_t count);
	void NewUnTitledTextObject ();

	void App_Settings ();
	static void Defocus_Text_object ();
	// void HomeScreen () {}

	void RaiseQuery (const char *message, std::vector<std::string_view> query_options, void *target_object, void(*callback_func)(void *, bool, uint32_t, const bool *), const bool *default_state = nullptr);
	// Query methods
	static void LoadFileCallback (void* target_obj,bool yes_no, uint32_t option_count, const bool *option_state);
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

	std::vector<SymSpell_Dictionary> m_Dictionaries;
	
	// File Queue
	std::queue<std::string> m_File_Queue;

	std::queue<UserQuery> m_UserQueries;

	// std::vector<std::string> m_FileHistory; // every-time any file is opened, History updates and then immediately saved on disk
	static const char *s_AppDataLocation;
	struct _Settings
	{
		bool UnifiedDictionary       = true;
		bool UseCtrlForSuggestionNav = true;

		bool autosave_enabled = false;
		float autosave_every_in_seconds = 60.0f;

		std::vector<const char *> AllowedExtensions;
	};
	static _Settings s_Settings_Global, s_Settings_temp;

	static bool s_Settings_changed;
private:
	// settings manipulator
	friend bool ChangeUnifiedDictionaryState (const bool, new_MVC_Layer &);
};