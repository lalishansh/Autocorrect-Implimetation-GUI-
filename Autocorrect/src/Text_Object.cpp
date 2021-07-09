#include "Text_Object.h"
#include <fstream>
#include <istream>
#include <imgui/imgui.h>


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
	ImVec2 draw_region = ImGui::GetContentRegionAvail ();
	ImGui::InputTextMultiline ("TextBox", m_CharecBuffer, m_Buffer_Capacity, draw_region, ImGuiInputTextFlags_CallbackEdit | ImGuiInputTextFlags_CallbackResize | ImGuiInputTextFlags_CallbackCompletion, Text_Object::text_input_callback, nullptr);
}

int Text_Object::text_input_callback (ImGuiInputTextCallbackData *data)
{
	if (data->EventFlag == ImGuiInputTextFlags_CallbackCompletion) {
		data->InsertChars (data->CursorPos, "    ");
	}
	if (data->EventFlag == ImGuiInputTextFlags_CallbackEdit) {
		*(uint32_t *)((void *)&s_FocusedTextObject->m_Buffer_Capacity) = data->BufTextLen + 1;
	}
	if (data->EventFlag == ImGuiInputTextFlags_CallbackResize) {
		s_FocusedTextObject->ResizeBuffer (data->BufSize);
		data->Buf = s_FocusedTextObject->m_CharecBuffer;
	}
	return 0;
}