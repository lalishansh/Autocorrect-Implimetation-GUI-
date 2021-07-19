#include <GLCore.h>
#include <GLCoreUtils.h>
#include <imgui/imgui_internal.h>

#include "Text_Entity.h"
#include "Manager_Layer.h"

bool Text_Entity::s_IsAnyTextInputFocused = false;

std::vector<SuggestItem> Lookup_MT (SymSpell &dictionary, std::string input, Verbosity verbosity, int maxEditDistance, bool includeUnknown, vector<SuggestItem> **instant_access_to_suggestions, bool *safe_cancel_signal)
{
	std::vector<SuggestItem> tmp = std::move (dictionary.Lookup (input, verbosity, maxEditDistance, includeUnknown, instant_access_to_suggestions, safe_cancel_signal));
	*instant_access_to_suggestions = &tmp;
	return tmp;
}

Text_Entity::Text_Entity (std::string data, std::string filePath)
	: m_FilePathOnDisk (std::move (filePath))
{
	{
		uint32_t size = data.size ();
		m_Buffer = std::move (data);
	}
	uint32_t idx = m_FilePathOnDisk.size ();
	while (idx > 0 && m_FilePathOnDisk[idx - 1] != '\\') {
		MY_ASSERT (m_FilePathOnDisk[idx - 1] != '/');
		idx--;
	}
	m_FileName = &m_FilePathOnDisk[idx];
}
Text_Entity::Text_Entity (Text_Entity &other)
	: m_FilePathOnDisk (std::move (other.m_FilePathOnDisk)), m_FileName(other.m_FileName), m_MyDictionary(other.m_MyDictionary), m_TargetWord(other.m_TargetWord)
	, m_ResetKeyboardFocus(other.m_ResetKeyboardFocus), m_RestartLookup(other.m_RestartLookup)
{
	other.m_CancelLookup = true;
	std::swap (m_FutureSuggestions, other.m_FutureSuggestions);
	std::swap (m_Buffer, other.m_Buffer);

	m_SuggestionsRTS = &m_FinalSuggestions;
}
//Text_Entity::Text_Entity (const Text_Entity &other)
//	: m_FilePathOnDisk (other.m_FilePathOnDisk), m_FileName(&m_FilePathOnDisk[other.m_FileName - &other.m_FilePathOnDisk[0]]), m_MyDictionary(other.m_MyDictionary), m_TargetWord(&m_FilePathOnDisk[&other.m_TargetWord[0] - &other.m_FilePathOnDisk[0]], other.m_TargetWord.size ()),
//	m_ResetKeyboardFocus(other.m_ResetKeyboardFocus), m_RestartLookup(other.m_RestartLookup)
//{
//	m_Buffer = other.m_Buffer;
//
//	m_SuggestionsRTS = &m_FinalSuggestions;
//}
Text_Entity Text_Entity::operator=(Text_Entity &other)
{
	return Text_Entity (other);
	//m_FilePathOnDisk = std::move (other.m_FilePathOnDisk), m_FileName = other.m_FileName, m_TargetWord = other.m_TargetWord;
	//m_MyDictionary = other.m_MyDictionary;
	//m_FutureSuggestions = std::move (other.m_FutureSuggestions);
	//m_ResetKeyboardFocus = other.m_ResetKeyboardFocus, m_RestartLookup = other.m_RestartLookup;
	//other.m_CancelLookup = true;
	//
	//std::swap (m_Buffer, other.m_Buffer);
	//
	//m_SuggestionsRTS = &m_FinalSuggestions;
}
int Text_Entity::text_input_callback (ImGuiInputTextCallbackData *data)
{
	Text_Entity::s_IsAnyTextInputFocused = true;
	Text_Entity &txt_entity = *((Text_Entity *)data->UserData);
	if (!Manager_Layer::IsFocused(&txt_entity)) {
		Manager_Layer::ChangeFocusTo (&txt_entity);
	}
	switch (data->EventFlag) {
		case ImGuiInputTextFlags_CallbackCompletion:
			if (txt_entity.m_SelectSuggestionIDX > 0) {
				txt_entity.m_SelectSuggestion = true;
			} else data->InsertChars (data->CursorPos, "    ");

			break;
		case ImGuiInputTextFlags_CallbackResize:
			int tmp = &txt_entity.m_TargetWord[0] - &txt_entity.m_Buffer[0];
			txt_entity.m_Buffer.resize (data->BufSize);
			txt_entity.m_TargetWord = std::string_view (&txt_entity.m_Buffer[0] + tmp, txt_entity.m_TargetWord.size ());
			data->Buf = &txt_entity.m_Buffer[0];
			break;
	}

	// Callback always
	if (txt_entity.m_SelectSuggestion) {
		txt_entity.m_SelectSuggestion = false;
	
		std::string word = txt_entity.m_SuggestionsRTS->at (txt_entity.m_SelectSuggestionIDX).term;

		uint16_t replace_At = &txt_entity.m_TargetWord[0] - data->Buf;
		uint16_t size = txt_entity.m_TargetWord.size ();
		txt_entity.m_TargetWord = word;

		data->DeleteChars (replace_At, size);
		data->InsertChars (replace_At, word.c_str ());

		txt_entity.m_TargetWord = std::string_view (&data->Buf[replace_At], word.size ());

		txt_entity.m_FinalSuggestions.clear ();
		data->CursorPos = replace_At + word.size () + 1;

		txt_entity.m_SelectSuggestionIDX = 0;
	}
	
	uint32_t cursor_posn = data->CursorPos;
	bool check = txt_entity.m_TargetWord.empty ();
	if (!check) {
		check = ((cursor_posn - uint32_t (&txt_entity.m_TargetWord[0] - &data->Buf[0])) > (txt_entity.m_TargetWord.size () - 1));
	}
	if(check){
		if (data->Buf[cursor_posn] != ' ' && data->Buf[cursor_posn] != '\0' && data->Buf[cursor_posn] != '\n' && data->Buf[cursor_posn] != '\r') {
			uint8_t left = 0;
			uint8_t right = 0;
			while (data->Buf[cursor_posn - left] != ' ' && data->Buf[cursor_posn - left] != '\n') {
				left++;
				if (cursor_posn < left) { left++; break; }
			}
			while (data->Buf[cursor_posn + right] != ' ' && data->Buf[cursor_posn + right] != '\0' && data->Buf[cursor_posn + right] != '\n' && data->Buf[cursor_posn + right] != '\r')
				right++;

			uint32_t size = (left - 1) + right;
			std::string_view candidate_word (&data->Buf[cursor_posn - left + 1], size);
			if (txt_entity.m_TargetWord != candidate_word) {
				txt_entity.m_TargetWord = candidate_word;
				if (size >= Manager_Layer::Get ()->MinimumWordLengthToInvokeLookup ())
					txt_entity.m_RestartLookup = true;// , LOG_TRACE ("Restart Lookup");
				else
					txt_entity.m_FinalSuggestions.clear (), txt_entity.m_SelectSuggestionIDX = 0;
			}
		} else
			txt_entity.m_TargetWord = std::string_view (&data->Buf[cursor_posn + 1], 1), txt_entity.m_FinalSuggestions.clear ();
	}
	return 0;
}
void Text_Entity::OnUpdate ()
{
	if (m_RestartLookup && !m_MyDictionary->Lock) {
		m_RestartLookup = false;
			
		if (m_MyDictionary->Sources.empty ()) return;

		if (m_FutureSuggestions.valid ()) {
			m_CancelLookup = true;
			m_FutureSuggestions.wait ();
			m_CancelLookup = false;
		}
		m_FutureSuggestions = std::async (std::launch::async, Lookup_MT, m_MyDictionary->symspell, std::string (m_TargetWord), Verbosity::All, symspell_Max_Edit_Distance, true, &m_SuggestionsRTS, &m_CancelLookup);
	}
}
void Text_Entity::OnImGuiRender ()
{
	if (m_ResetKeyboardFocus)
		ImGui::SetKeyboardFocusHere (), m_ResetKeyboardFocus = false;
	Text_Entity::s_IsAnyTextInputFocused = false;
	int Key_Up = ImGui::GetIO ().KeyMap[ImGuiKey_UpArrow];
	int Key_Dn = ImGui::GetIO ().KeyMap[ImGuiKey_DownArrow];
	if (Manager_Layer::Get()->SuggestionNavWithCtrl () ? ImGui::GetIO ().KeyCtrl : !ImGui::GetIO ().KeyCtrl)
		ImGui::GetIO ().KeyMap[ImGuiKey_UpArrow] = (m_SuggestionsRTS->size () > 1 && m_SelectSuggestionIDX != 0 ? -1 : Key_Up), ImGui::GetIO ().KeyMap[ImGuiKey_DownArrow] = (m_SuggestionsRTS->size () > 1 ? -1 : Key_Dn);
	ImGui::InputTextMultiline ("TextBox", &m_Buffer[0], m_Buffer.capacity () + 1, ImVec2 (-1, -1), ImGuiInputTextFlags_CallbackAlways | ImGuiInputTextFlags_CallbackResize | ImGuiInputTextFlags_CallbackCompletion, Text_Entity::text_input_callback, this);
	ImGui::GetIO ().KeyMap[ImGuiKey_UpArrow] = Key_Up;
	ImGui::GetIO ().KeyMap[ImGuiKey_DownArrow] = Key_Dn;

	// Display
	if (m_FutureSuggestions.valid ())
		if (m_FutureSuggestions.wait_for (std::chrono::seconds (0)) == future_status::ready || m_SuggestionsRTS == nullptr) {
			m_FinalSuggestions = std::move (m_FutureSuggestions.get ());
			m_SuggestionsRTS = &m_FinalSuggestions;
		}

	if (m_SuggestionsRTS->size () > 1 && Manager_Layer::IsFocused (this)) {
		ImVec2 suggestionsDrawPosn = ImGui::GetCurrentContext ()->PlatformImeLastPos;
		suggestionsDrawPosn.y += 15;
		ImGui::SetNextWindowPos (suggestionsDrawPosn);
		ImGui::Begin ("Suggestions", NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoNavInputs | ImGuiWindowFlags_AlwaysAutoResize);
		for (uint32_t i = 0; i < MIN(m_SuggestionsRTS->size (), 10); i++) {
			SuggestItem word = m_SuggestionsRTS->at (i);
			bool dummy = i == (m_SelectSuggestionIDX - 1);
			if (ImGui::MenuItem (word.term.c_str (), NULL, &dummy))
				m_SelectSuggestionIDX = i + 1, m_ResetKeyboardFocus = true, m_SelectSuggestion = true;
		}
		ImGui::End ();
	}
}

void Text_Entity::StateReset ()
{
	m_SelectSuggestionIDX = 0;
	m_RestartLookup = false;
	m_ResetKeyboardFocus = false;
}
void Text_Entity::OnEvent (char event)
{
	switch (event) {
		case 'U':
			m_SelectSuggestionIDX = m_SelectSuggestionIDX != 0 ? m_SelectSuggestionIDX - 1 : 0; 
			break;
		case 'D':
			if (m_SuggestionsRTS != nullptr)
				m_SelectSuggestionIDX = MIN (m_SuggestionsRTS->size () - 1, m_SelectSuggestionIDX + 1); 
			break;
		case 'S':
			SaveFile ();
			break;
	}
}
std::string_view Text_Entity::FileDir ()
{
	return std::string_view (m_FilePathOnDisk.c_str (), uint32_t (m_FileName - &m_FilePathOnDisk[0]));
}
void Text_Entity::SaveFile ()
{
	if (m_FilePathOnDisk.empty ()) {
		m_FilePathOnDisk = GLCore::Utils::FileDialogs::SaveFile ("Txt files (*.txt)\0*.txt\0");
		if (m_FilePathOnDisk.empty ())
			return;

		uint32_t i = m_FilePathOnDisk.size ();
		while (m_FilePathOnDisk[i - 1] != '\\' && i > 0)
			i++;
		m_FileName = &m_FilePathOnDisk[i];
	}
	std::ofstream fout (m_FilePathOnDisk, std::ios::out | std::ios::binary);
	fout << m_Buffer.c_str ();
	fout.close ();
	Manager_Layer::Get()->SetSignalHint ("Saved File");
}