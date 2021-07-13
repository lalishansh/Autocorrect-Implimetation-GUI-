#include "new_MVC_Layer.h"
#include "Text_Object.h"

#include <fstream>
#include <istream>
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <thread>
#include <GLCoreUtils.h>



Text_Object *Text_Object::s_FocusedTextObject = nullptr;
char *Text_Object::s_SignalHint = nullptr;
float Text_Object::s_SignalPersists_for = 0;

std::vector<SuggestItem> Lookup_MT (SymSpell &dictionary, std::string input, Verbosity verbosity, int maxEditDistance, bool includeUnknown, vector<SuggestItem> **instant_access_to_suggestions, bool *safe_cancel_signal)
{
	std::vector<SuggestItem> tmp = std::move (dictionary.Lookup (input, verbosity, maxEditDistance, includeUnknown, instant_access_to_suggestions, safe_cancel_signal));
	*instant_access_to_suggestions = &tmp;
	return tmp;
}

std::optional<Text_Object> Text_Object::OpenFileAsText (const char *filePath /*= ""*/)
{
	std::string result;
	std::ifstream in (filePath, std::ios::in | std::ios::binary);
	if (in) {
		in.seekg (0, std::ios::end);
		size_t size = size_t (in.tellg ());

		// Doesn't matter if empty
		result.resize (size + 1); // also changes size

		in.seekg (0, std::ios::beg);
		in.read (&result[0], size);
	} else {
		//LOG_ERROR ("Unable To Open File '{0}'", filePath);
		return {};
	}
	return std::make_optional<Text_Object> (filePath, result.c_str(), result.size ());
}

void Text_Object::ResizeBuffer (uint32_t new_size)
{
	if (new_size > m_Buffer_Capacity) {
		char *temp = new char[new_size];
		if (m_CharecBuffer) {
			memcpy_s (temp, new_size, m_CharecBuffer, m_Buffer_Size);
			delete[] m_CharecBuffer;
		}
		m_CharecBuffer = temp;
		*(uint32_t *)((void *)&m_Buffer_Capacity) = new_size;
	}
}

void Text_Object::OnUpdate ()
{
	if (m_ReStartLookup && !m_MyDictionary->Lock) {
		m_ReStartLookup = false;
		if (m_MyDictionary->Sources.size () == 0) return;

		if (m_Suggestions_future.valid ()) {
			m_CancelLookup = true;

			m_Suggestions_future.wait ();

			m_CancelLookup = false;
		};
		m_Suggestions_future = std::async(std::launch::async, Lookup_MT, m_MyDictionary->symspell, std::string(m_TargetWord), Verbosity::All, symspell_Max_Edit_Distance, true, &m_SuggestionsRef, &m_CancelLookup);
	}
}

void Text_Object::ImGuiTextRender (bool nav_suggestions_with_ctrl)
{
	if (ImGui::IsWindowFocused () && s_FocusedTextObject != this) {
		new_MVC_Layer::Defocus_Text_object ();
		s_FocusedTextObject = this;
	}

	if (m_ResetFocus)
		ImGui::SetKeyboardFocusHere (), m_ResetFocus = false;
	
	int Key_Up = ImGui::GetIO ().KeyMap[ ImGuiKey_UpArrow ];
	int Key_Dn = ImGui::GetIO ().KeyMap[ImGuiKey_DownArrow];
	if(nav_suggestions_with_ctrl ? ImGui::GetIO ().KeyCtrl : !ImGui::GetIO ().KeyCtrl)
		ImGui::GetIO ().KeyMap[ImGuiKey_UpArrow] = (m_SuggestionsRef->size () > 1 && m_SelectSuggestion != -1 ? -1 : Key_Up), ImGui::GetIO ().KeyMap[ImGuiKey_DownArrow] = (m_SuggestionsRef->size () > 1 ? -1 : Key_Dn);
	ImGui::InputTextMultiline ("TextBox", m_CharecBuffer, m_Buffer_Capacity, ImVec2 (-1, -1), ImGuiInputTextFlags_CallbackAlways | ImGuiInputTextFlags_CallbackEdit | ImGuiInputTextFlags_CallbackResize | ImGuiInputTextFlags_CallbackCompletion, Text_Object::text_input_callback, this);
	ImGui::GetIO ().KeyMap[ImGuiKey_UpArrow]   = Key_Up;
	ImGui::GetIO ().KeyMap[ImGuiKey_DownArrow] = Key_Dn;

	// Display
	if(m_Suggestions_future.valid())
		if (m_Suggestions_future.wait_for (std::chrono::seconds(0)) == future_status::ready) {
			m_Suggestions = std::move (m_Suggestions_future.get ());
			m_SuggestionsRef = &m_Suggestions;
		}

	if (m_SuggestionsRef->size () > 1 && this == s_FocusedTextObject) {
		ImVec2 suggestionsDrawPosn = ImGui::GetCurrentContext ()->PlatformImeLastPos;
		suggestionsDrawPosn.y += 15;
		ImGui::SetNextWindowPos (suggestionsDrawPosn);
		ImGui::Begin ("Suggestions", NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoNavInputs);
		for (uint32_t i = 0; i < m_SuggestionsRef->size (); i++) {
			auto word = m_SuggestionsRef->at (i);
			bool dummy = i == m_SelectSuggestion;
			if (ImGui::MenuItem (word.term.c_str (), NULL, &dummy))
				m_SelectSuggestion = i, m_ResetFocus = true;

			if (i > 10) break; // 1st 10 suggestions
		}
		ImGui::End ();
	}
}

const uint8_t min_word_width = 3;
int Text_Object::text_input_callback (ImGuiInputTextCallbackData *data)
{
	if(s_FocusedTextObject != (Text_Object *)data->UserData) {
		new_MVC_Layer::Defocus_Text_object ();
		s_FocusedTextObject = s_FocusedTextObject;
	}
	switch (data->EventFlag) {
		case ImGuiInputTextFlags_CallbackCompletion:
			if (s_FocusedTextObject->m_SelectSuggestion > -1) {
				std::string word = s_FocusedTextObject->m_SuggestionsRef->at (s_FocusedTextObject->m_SelectSuggestion).term;

				uint16_t replace_At = &s_FocusedTextObject->m_TargetWord[0] - data->Buf;
				uint16_t size = s_FocusedTextObject->m_TargetWord.size ();
				s_FocusedTextObject->m_TargetWord = word;

				data->DeleteChars (replace_At, size);
				data->InsertChars (replace_At, word.c_str ());

				s_FocusedTextObject->m_TargetWord = std::string_view (&data->Buf[replace_At], word.size ());
				
				s_FocusedTextObject->m_Suggestions.clear ();
				data->CursorPos = replace_At + word.size ();

				s_FocusedTextObject->m_SelectSuggestion = -1;
			}else data->InsertChars (data->CursorPos, "    ");

			break;
		case ImGuiInputTextFlags_CallbackEdit:
			*(uint32_t *)((void *)&s_FocusedTextObject->m_Buffer_Size) = data->BufTextLen + 1;
			break;
		case ImGuiInputTextFlags_CallbackResize:
			s_FocusedTextObject->ResizeBuffer (data->BufSize);
			data->Buf = s_FocusedTextObject->m_CharecBuffer;
			break;
		default: // callback_always
			uint32_t cursor_posn = MAX(data->CursorPos - 1, 1);
			if (data->Buf[cursor_posn] != ' ' && data->Buf[cursor_posn] != '\0' && data->Buf[cursor_posn] != '\n' && data->Buf[cursor_posn] != '\r') {
				uint8_t left = 0;
				uint8_t right = 0;
				while (data->Buf[cursor_posn - left] != ' ' && data->Buf[cursor_posn - left] != '\n') {
					left++;
					if ((cursor_posn - left) == 0) { left++; break; }
				}
				while (data->Buf[cursor_posn + right] != ' ' && data->Buf[cursor_posn + right] != '\0' && data->Buf[cursor_posn + right] != '\n' && data->Buf[cursor_posn + right] != '\r')
					right++;

				uint32_t size = (left - 1) + right;
				std::string_view candidate_word (&data->Buf[cursor_posn - left + 1], size);
				if (s_FocusedTextObject->m_TargetWord != candidate_word){
					s_FocusedTextObject->m_TargetWord  = candidate_word;
					if (size >= min_word_width)
						s_FocusedTextObject->m_ReStartLookup = true;// , LOG_TRACE ("Restart Lookup");
					else
						s_FocusedTextObject->m_Suggestions.clear ();
				}
			}else 
				s_FocusedTextObject->m_TargetWord = std::string_view (""), s_FocusedTextObject->m_Suggestions.clear ();
	}
	return 0;
}

void Text_Object::OnEvent (char event)
{
	switch (event) {
		case 'U':
			m_SelectSuggestion = MAX(-1, m_SelectSuggestion - 1); break;
		case 'D':
			if(m_SuggestionsRef != nullptr)
				m_SelectSuggestion = MIN (m_SuggestionsRef->size () - 1, m_SelectSuggestion + 1); break;
		case 'S':
			if (m_FilePath.empty())
			{
				m_FilePath = GLCore::Utils::FileDialogs::SaveFile ("all files (*.*)\0*.txt\0*.*\0");
				if(m_FilePath.empty())
					break;

				uint32_t i = 0;
				while (m_FilePath[m_FilePath.size () - i - 1] != '\\' && m_FilePath[m_FilePath.size () - i - 1] != '/')
					i++;
				m_FileName = &m_FilePath[m_FilePath.size () - i];
			}
			std::ofstream fout(m_FilePath, std::ios::out | std::ios::binary);
			fout << m_CharecBuffer;
			fout.close ();
			Text_Object::SetSignal ("Saved File");
			break;
	}
}