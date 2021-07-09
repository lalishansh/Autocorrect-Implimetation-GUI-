#pragma once
#include <string>
#include <vector>
#include "SymSpell/include/SymSpell.h"
#include <future>
#include <optional>

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
		while (m_FilePath[m_FilePath.size () - i - 1] != '\\')
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
	void ImGuiTextRender ();
private:
	void ResizeBuffer (uint32_t new_size);
public:
	static int text_input_callback(ImGuiInputTextCallbackData *data);
	static Text_Object *s_FocusedTextObject;
private:
	// Text-Box related
	bool m_Focused = false; // use focus to handle ImGui call-back
	bool m_ResetFocus = false;
	bool m_CancelLookup = false;

	uint32_t m_CursorPosnX = 0; // set via call-back
	uint32_t m_CursorPosnY = 0; // set via call-back

	// Buffer
	const uint32_t m_Buffer_Capacity = 0; // will only be modified when resizing
	const uint32_t m_Buffer_Size = 0; // update only through call-back
	char *m_CharecBuffer = nullptr;

	std::string m_TargetWord = std::string();

	// SymSpell
	static const int s_MaxEditDistance = 3, s_PrefixLength = 4;
	SymSpell m_Symspell = SymSpell (1, s_MaxEditDistance, s_PrefixLength);
	//// results
	std::vector<SuggestItem> *m_SuggestionsRef; // will be used for output
	std::future<std::vector<SuggestItem>> m_Suggestions; // will store results obtained
	
	std::vector<std::string> m_LoadedDictionaries;
	std::string m_FilePath;
public:
	const char* m_FileName;
};