#pragma once
#include <string>
#include <vector>
#include "SymSpell/include/SymSpell.h"
#include <future>
#include <optional>
#include "Settings_editor_methods.h"

class ImGuiInputTextCallbackData;
class Text_Object
{
private:
public:
	static std::optional<Text_Object> OpenFileAsText (const char *filePath /*= ""*/);

	Text_Object (const Text_Object &other)
		: m_FilePath (other.m_FilePath), m_Buffer_Size (other.m_Buffer_Size)
	{
		ResizeBuffer (m_Buffer_Size + 100);
		memcpy_s (m_CharecBuffer, m_Buffer_Capacity, other.m_CharecBuffer, other.m_Buffer_Size);

		// name of file
		uint32_t i = 0;
		while (m_FilePath[m_FilePath.size () - i - 1] != '\\' && m_FilePath[m_FilePath.size () - i - 1] != '/')
			i++;
		m_FileName = &m_FilePath[m_FilePath.size () - i];
	}

	Text_Object (const char* filepath, const char* data, uint32_t size)
		: m_FilePath(filepath), m_Buffer_Size(size)
	{
		ResizeBuffer (m_Buffer_Size + 100);
		memcpy_s (m_CharecBuffer, m_Buffer_Capacity, data, size);

		// name of file
		uint32_t i = 0;
		while (m_FilePath[m_FilePath.size () - i - 1] != '\\')
			i++;
		m_FileName = &m_FilePath[i];
	}
	~Text_Object ()
	{
		delete[] m_CharecBuffer; m_CharecBuffer = nullptr;
		*(uint32_t *)((void *)&m_Buffer_Capacity) = 0;
		*(uint32_t *)((void *)&m_Buffer_Size) = 0;
	}
	void ImGuiTextRender (bool suggestion_nav_with_ctrl);
	void OnUpdate ();
	void OnEvent (char event);
private:
	void ResizeBuffer (uint32_t new_size);
public:
	static int text_input_callback(ImGuiInputTextCallbackData *data);
	static Text_Object *s_FocusedTextObject;
private:
	// Text-Box related
	bool m_ResetFocus = false;
	bool m_CancelLookup = false; // Think of ways to keep it alive in L3 cache // to be passed to lookup(modified) as pointer so we can cancel lookup
	bool m_ReStartLookup = false;
	int m_SelectSuggestion = -1;

	// Buffer
	const uint32_t m_Buffer_Capacity = 0; // will only be modified when resizing
	const uint32_t m_Buffer_Size = 0; // update only through call-back
	char *m_CharecBuffer = nullptr;

	std::string_view m_TargetWord = std::string_view(" ", 1); // directly extracted from call-back

	// SymSpell
	SymSpell *m_Symspell = nullptr;
	std::vector<std::string> *m_LoadedDictionaries = nullptr;
	
	//// results
	std::vector<SuggestItem> *m_SuggestionsRef = nullptr; // will be used for output
	std::future<std::vector<SuggestItem>> m_Suggestions_future = std::async (std::launch::deferred, 
																	  []() {
																		  return std::vector<SuggestItem> ();
																	  }); // will store results obtained
	std::vector<SuggestItem> m_Suggestions;

	std::string m_FilePath;
public:
	const char* m_FileName;

private:
	friend class new_MVC_Layer;
	friend bool ChangeUnifiedDictionaryState (const bool, new_MVC_Layer &);
};