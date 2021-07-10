#include "Text_Object.h"
#include <fstream>
#include <istream>
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>


Text_Object *Text_Object::s_FocusedTextObject = nullptr;

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
void Text_Object::ImGuiTextRender ()
{
	if (ImGui::IsWindowFocused ())
		s_FocusedTextObject = this;

	ImGui::InputTextMultiline ("TextBox", m_CharecBuffer, m_Buffer_Capacity, ImVec2(-1,-1), ImGuiInputTextFlags_CallbackAlways | ImGuiInputTextFlags_CallbackEdit | ImGuiInputTextFlags_CallbackResize | ImGuiInputTextFlags_CallbackCompletion, Text_Object::text_input_callback, this);
	
	ImVec2 suggestionsDrawPosn = ImGui::GetCurrentContext ()->PlatformImeLastPos;
	suggestionsDrawPosn.x += 15;
	ImGui::SetNextWindowPos (suggestionsDrawPosn);
	ImGui::Begin ("Suggestions", NULL, ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize);
	ImGui::Button ("DUMMY");
	ImGui::End ();
}
const uint8_t min_word_width = 3;
int Text_Object::text_input_callback (ImGuiInputTextCallbackData *data)
{
	s_FocusedTextObject = (Text_Object*)data->UserData;
	switch (data->EventFlag) {
		case ImGuiInputTextFlags_CallbackCompletion:
			data->InsertChars (data->CursorPos, "    ");
			break;
		case ImGuiInputTextFlags_CallbackEdit:
			*(uint32_t *)((void *)&s_FocusedTextObject->m_Buffer_Size) = data->BufTextLen + 1;
			break;
		case ImGuiInputTextFlags_CallbackResize:
			s_FocusedTextObject->ResizeBuffer (data->BufSize);
			data->Buf = s_FocusedTextObject->m_CharecBuffer;
			break;
		default: // calback_always
			uint32_t cursor_posn = data->CursorPos;
			if (data->Buf[cursor_posn] != ' ' && data->Buf[cursor_posn] != '\0' && data->Buf[cursor_posn] != '\n') {
				uint8_t left = 0;
				uint8_t right = 0;
				while (data->Buf[cursor_posn - left] != ' ' && data->Buf[cursor_posn - left] != '\n') {
					left++;
					if ((cursor_posn - left) == 0) { left++; break; }
				}
				while (data->Buf[cursor_posn + right] != ' ' && data->Buf[cursor_posn + right] != '\0' && data->Buf[cursor_posn + right] != '\n')
					right++;
				
				uint32_t size = (left - 1) + right;
				std::string_view candidate_word (&data->Buf[cursor_posn - left + 1], size);
				if (s_FocusedTextObject->m_TargetWord != candidate_word && size >= min_word_width)
					s_FocusedTextObject->m_TargetWord = candidate_word, s_FocusedTextObject->m_ReStartLookup = true;
				// std::cout << "\"" << s_FocusedTextObject->m_TargetWord << "\"  ";
			}
	}
	return 0;
}