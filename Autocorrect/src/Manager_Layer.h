#pragma once
#include <GLCore.h>

#include "Core.h"

class Text_Entity;
typedef unsigned int ImGuiID;
class Manager_Layer : public GLCore::Layer
{
private:
	static Manager_Layer *s_Singleton;
	Manager_Layer (const int argc, char *argv[], char *envv[]);
public: // constructor(s), destructor(s) and mandatory stuff
	static Manager_Layer* Get ()
	{
		MY_ASSERT (s_Singleton != nullptr);
		return s_Singleton;
	}
	static Manager_Layer *Create (const int argc, char *argv[], char *envv[]) {
		if (s_Singleton == nullptr)
			s_Singleton = new Manager_Layer (argc, argv, envv);
		return s_Singleton;
	};
	~Manager_Layer () {};
	virtual void OnEvent (GLCore::Event &event) override;
	virtual void OnUpdate (GLCore::Timestep ts) override;
	virtual void OnImGuiRender() override;
	virtual void OnAttach() override;
	virtual void OnDetach() override;
private: // Resource
	ImGuiID m_DockspaceID;

	char *m_SignalHint = "";
	float m_SignalHintPersistsMoreFor = -1.0f; // in seconds, Max persist duration in settings

	std::queue<Signal> m_Signals;
	std::queue<QueryBox> m_QueryBoxes;

	std::list<SymSpell_Dictionary> m_Dictionaries;
	std::vector<Text_Entity> m_TextEntities;
	
	// vector.back() is on top
	std::vector<uint16_t> m_FocusStack;

	bool m_SettingsWindowOpen = false;
	bool m_SettingsChanged = false;
	struct _Settings
	{
		bool _unified_dictionary      = true;
		bool _use_ctrl_for_suggestion_nav  = true;

		bool _autosave_enabled        = false;
		float _autosave_every         = 60.0f; // in seconds
		
		int _minWord_length_to_invoke_lookup = 3;
		
		float _signal_hint_max_persist_durn = 4.0f; // duration in seconds
		float _window_opacity = 0.9f; // duration in seconds

		std::vector<std::string> AllowedExtensions;
	};
	_Settings m_Settings_main, m_Settings_temp;

private:
	void ImGuiRenderDockables ();
	void ImGuiMainMenuBarItems ();
	void SettingsImGuiRender ();

	void TrySaveSettings ();
	void DiscardSettings ();
public:
	// Getters
	std::list<SymSpell_Dictionary> &Dictionaries () { return m_Dictionaries; }
	std::queue<Signal> &SignalQueue () { return m_Signals; }
	std::queue<QueryBox> &PromptQueue () { return m_QueryBoxes; }
	const bool SuggestionNavWithCtrl () { return m_Settings_main._use_ctrl_for_suggestion_nav; };
	const uint8_t MinimumWordLengthToInvokeLookup () { return uint8_t(m_Settings_main._minWord_length_to_invoke_lookup); };

	// Helpers
	static bool IsFocused (Text_Entity *ptr);
	static void ChangeFocusTo (Text_Entity *ptr);
	
	void CloseTextEntity (Text_Entity *ptr);

	void SetSignalHint (char *hint); // Doesn't takes owner ship of data, reccommended to only paas char*, that are going to last at-least max-persist amount

	void SignalInvalidator (void *signals_with_data);

	void RaiseQuery (std::string message, std::vector<std::pair<std::string, bool>> options, void *data1, void *data2, void(*static_callbck)(void *, void *, uint8_t *, bool));
	static void DummySignalCallback (void *this_obj, void *a, uint8_t *b, bool is_valid_signal)
	{
		MY_ASSERT (a == nullptr); // pass nullptr, as dummy cannot determine the type of [*a] like specialized functions, i.e memory leak
		MY_ASSERT (b);
		delete[] b;
	}

	// Make it lambda since it will only be used when files are drag and dropped
	// takes ownership of only ptr <1> & <2>, not <0>
	static void file_process_callback (void *, void *, uint8_t *, bool is_valid_signal);

	// [MT inside] takes ownership of filepath, returns false if file cannot be loaded.
	bool LoadTextFileToEntity(std::string filePath);
	// [MT inside] takes ownership of filepath, returns false if handle is busy.
	void ExtendDictionaryOf(SymSpell_Dictionary& handle, std::string from_FilePath);
	
	static void DestroyDictionary (void *_dict_location, void *dummy, uint8_t *options_data, bool is_valid_signal);
	static void LazyDestroyDictionary (void *_dict_location, void *dummy1, uint8_t *dummy2, bool signal_is_valid);
};