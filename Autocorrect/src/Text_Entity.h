#pragma once
#include "Core.h"

class Manager_Layer;
struct ImGuiInputTextCallbackData;
class Text_Entity
{
public:
	Text_Entity (std::string data, std::string filePath);
	Text_Entity (Text_Entity& other);
	Text_Entity operator=(Text_Entity& other);
	//Text_Entity (const Text_Entity& other);
	~Text_Entity () = default;
	static int text_input_callback (ImGuiInputTextCallbackData *data);
	static bool IsAnyKeyboardFocused () { return s_IsAnyTextInputFocused; }

	void OnUpdate ();
	void OnImGuiRender ();
	void StateReset ();
	void OnEvent (char);
private:
	static bool s_IsAnyTextInputFocused;
	bool   m_ResetKeyboardFocus = false;
	bool   m_CancelLookup       = false;
	bool   m_RestartLookup      = false;
	bool   m_SelectSuggestion   = false;

	uint32_t m_SelectSuggestionIDX = 0; // 0 for no selection, val mapping to m_SuggestionsRTS.Index is [1...] = [0...]

	std::string m_Buffer = std::string();

	SymSpell_Dictionary *m_MyDictionary = nullptr;
	std::vector<SuggestItem> *m_SuggestionsRTS = nullptr; // Real-Time suggestion Stream
	std::vector<SuggestItem> m_FinalSuggestions; // a warehouse to future suggestions
	std::future<std::vector<SuggestItem>> m_FutureSuggestions = std::async (std::launch::deferred,
																		[]() {
																		 return std::vector<SuggestItem> ();
																		}); // will store results obtained
	
	std::string_view m_TargetWord = std::string_view("  ",2);

	std::string m_FilePathOnDisk = std::string(".\\[Untitled]");
	
	char *m_FileName = &m_FilePathOnDisk[2]; // extracted from file-path
	
	friend class Manager_Layer;
public:
	const char *FileName () { return m_FileName; }
	const char *FilePath () { return m_FilePathOnDisk.c_str (); }
	std::string_view FileDir (); // Though no use case

	void MyDictionary (SymSpell_Dictionary *ptr) { 
		m_MyDictionary = ptr; 
	}
	SymSpell_Dictionary *MyDictionary () { 
		return m_MyDictionary; 
	}

	void SaveFile (); // Save File
};