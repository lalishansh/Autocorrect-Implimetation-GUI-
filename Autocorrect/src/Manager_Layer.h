#pragma once
#include "Core.h"
#include "GLCore.h"

class Text_Entity;
class Manager_Layer : public GLCore::Layer
{
private:
	static Manager_Layer *s_Singleton;
	Manager_Layer (const int argc, char *argv[], char *envv[]);
public: // constructor(s), destructor(s) and mandatory stuff
	Get ()
	{
#ifdef _DEBUG
		if (s_Singleton == nullptr) __debugbreak ();
#endif // _DEBUG
		return s_Singleton;
	}
	Manager_Layer *Create (const int argc, char *argv[], char *envv[]) {
		if (s_Singleton == nullptr)
			s_Singleton = new Manager_Layer (argc, argv, envv);
		return s_Singleton;
	};
	~Manager_Layer () { if (s_Singleton) { s_Singleton->OnDelete (); delete s_Singleton; } s_Singleton = nullptr; };
	void OnDelete ();
	virtual void OnEvent (GLCore::Event &event) override;
	virtual void OnUpdate (GLCore::Timestep ts) override;
	virtual void OnImGuiRender() override;
	virtual void OnAttach() override;
	virtual void OnDetach() override;
private: // Resource
	char *m_SignalHint = "";
	float m_SignalHintPersistsMoreFor = -1.0f; // in seconds, Max persist duration in settings

	std::queue<Signal> m_Signals;
	std::queue<QueryBox> m_QueryBoxes;

	std::list<SymSpell_Dictionary> m_Dictionaries;
	std::vector<Text_Entity> m_TextEntities;
	
	// vector.back() is on top
	std::vector<uint16_t> m_FocusStack;


	struct _Settings
	{
		bool _unified_dictionary      = true;
		bool _use_ctrl_for_suggestion_nav  = true;

		bool _autosave_enabled        = false;
		float _autosave_every         = 60.0f; // in seconds
		
		uint8_t _minWord_length_to_invoke_lookup = 3;
		
		float _signal_hint_max_persist_durn = 4.0f; // duration in seconds

		std::vector<std::string> AllowedExtensions;
	};
	bool m_SettingsChanged = false;
	_Settings m_Settings_main, m_Settings_temp;
private:
	void ImGuiRenderDockables ();
	void ImGuiMainMenuBarItems ();
	void SettingsImGuiRender ();

	bool TrySaveSettings ();
	void DiscardSettings ();

	// guaranteed that state is changing, returns new state, 
	bool ChangeUnifiedDictionaryState (bool from, bool& to);
public:
	bool IsFocused (Text_Entity *ptr);
	void ChangeFocusTo (Text_Entity *ptr);
	void CloseTextEntity (Text_Entity *ptr);

	void SetSignalHint (char *hint); // Doesn't takes owner ship of data, reccommended to only paas char*, that are going to last at-least max-persist amount

	template<typename CLASS> // For the sake of coherence
	void RaiseQuery (std::string message, std::vector<std::string, bool> options, void *extra_data, CLASS *target, void(*CLASS::static_callbck)(void *, void *, uint8_t *))
	{
		// QueryBox (std::queue<Signal> *signalQueue, std::string message, std::vector<std::pair<std::string, bool>> optionsWithDefaultState, void* data_ex, void* target_obj, void(*callbackFunc)(void *, void *, void *))
		m_QueryBoxes.emplace (&m_Signals, std::move (message), std::move (options), extra_data, target, CLASS::static_callbck);// static_callbck);
	}
	static void DummySignalConsume (void *this_obj, void *a, uint8_t *b)
	{
#ifdef _DEBUG
		if(a) __debugbreak ();
		if(b) __debugbreak ();
#endif // _DEBUG
		if(b) delete[] b;
	}

	void FileProcessCallback (void *, void *, uint8_t *);

	// returns false if file cannot be loaded.
	bool LoadTextFileToEntity(char *filePath);
	
	// returns false if handle is busy.
	bool ExtendDictionaryADictionary(SymSpell_Dictionary& handle, char *from_FilePath);
};