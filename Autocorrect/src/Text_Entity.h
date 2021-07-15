#pragma once
#include "Core.h"
#include <future>

struct ImGuiInputTextCallbackData;
class Text_Entity
{
public:
	Text_Entity (std::string data, std::string filePath = std::string ());
	Text_Entity (Text_Entity& other);
	~Text_Entity();

	static int text_input_callback (ImGuiInputTextCallbackData *data);
	static bool IsAnyKeyboardFocused () { return s_IsAnyTextInputFocused; }

	void OnUpdate ();
	void OnImGuiRender ();
	void OnEvent (char);
private:
	static bool s_IsAnyTextInputFocused;
	bool   m_ResetKeyboardFocus = false;
	bool   m_CancelLookup       = false;
	bool   m_RestartLookup      = false;

	uint32_t m_SelectSuggestionIDX = 0; // 0 for no selection, val mapping to m_SuggestionsRTS.Index is [1...] = [0...]

	struct
	{
		// Give ownership of buffer
		void reset (char *ptr_to_buff, const uint32_t capacity)
		{
			if (_buffer) delete[] _buffer;
			_buffer = ptr_to_buff;
			_buffer_capacity = capacity;
		}
		void reset (const uint32_t capacity)
		{
#ifdef _DEBUG
			if (capacity == 0) __debugbreak ();
#endif // _DEBUG
			if (_buffer) delete[] _buffer;
			_buffer = new char[capacity];
			_buffer_capacity = capacity;
		}
		const uint32_t Capacity () { return _buffer_capacity; }

		operator char *() { return _buffer; }
		const char *Get() { return _buffer; }

		void Destroy ()
		{
			if (_buffer) {
				delete[] _buffer;
#ifdef _DEBUG
				if (_buffer_capacity == 0) __debugbreak ();
#endif // _DEBUG
			}
			_buffer = nullptr;
			_buffer_capacity = 0;
		}
	private:
		uint32_t _buffer_capacity = 0;
		char *_buffer = nullptr;
	}m_Buffer;


	SymSpell_Dictionary *m_MyDictionary = nullptr;
	std::vector<SuggestItem> *m_SuggestionsRTS = nullptr; // Real-Time suggestion Stream
	std::vector<SuggestItem> m_FinalSuggestions; // a warehouse to future suggestions
	std::future<std::vector<SuggestItem>> m_FutureSuggestions = = std::async (std::launch::deferred,
																		[]() {
																		 return std::vector<SuggestItem> ();
																		}); // will store results obtained
	
	std::string_view m_TargetWord = std::string_view(" ");

	std::string m_FilePathOnDisk;
	
	char *m_FileName = "[Untitled]"; // extracted from file-path
	
public:
	const char *FileName () { return m_FileName; }
	const char *FilePath () { return m_FilePathOnDisk.c_str (); }
	const std::string_view FileDir () { return std::string_view (m_FilePathOnDisk, uint32_t (m_FileName - m_FilePathOnDisk.c_str ())); } // Though no use case

	void Dictionary (SymSpell_Dictionary *ptr) { m_MyDictionary = ptr; }
	SymSpell_Dictionary *Dictionary () { return m_MyDictionary; }
};