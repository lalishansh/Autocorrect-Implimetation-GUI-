#include "Core.h"
#include <imgui/imgui.h>
#include <filesystem>

bool QueryBox::OnImGuiRender ()
{
	ImGui::OpenPopup (ImGui::GetID ("Query User Pop-up"));
	bool Ready = false;
	
	if (ImGui::BeginPopupModal ("Query User Pop-up", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
		
		ImGui::Text (message.first.c_str ());
		ImGui::Indent ();
		for (auto& option: query_options)
		{
			ImGui::Checkbox (option.first.c_str (), &option.second);
		}
		ImGui::Unindent ();

		float width = ImGui::GetContentRegionAvailWidth ()*0.5f - 4.0f; // Debug
		
		if(ImGui::Button ("Yes", ImVec2 (width, 0)))
		   message.second =  true, Ready = true;
		ImGui::SameLine ();
		if(ImGui::Button ("No", ImVec2 (width, 0)))
		   message.second = false, Ready = true;

		ImGui::EndPopup ();
	}
	return Ready;
}

std::optional<std::string> AbsoluteDirectoryPath (std::string orignal)
{
	if (std::filesystem::exists (orignal))
	{
		return { std::filesystem::absolute (std::move(orignal)).string () };
	}
	return {};
}
std::string CreateNewFile (std::string orignal /*= std::string()*/)
{
	uint32_t idx = MAX(orignal.size () - 1, 0);
	while (orignal[idx] != '.' && orignal[idx] != '\\' && idx > 0)
		idx--;
	if (orignal[idx] != '.')
		if (orignal[idx] == '\\') {
			orignal = orignal.substr (0, idx + 1) + "New_Untitled.txt"; idx = orignal.size () - 4;
		} else {
			orignal = std::filesystem::current_path ().string () + std::string ("\\New_file.txt"); idx = orignal.size () - 4;
		}
	if (orignal[idx] != '.') __debugbreak ();

	{
		int del = idx - orignal.size ();
		orignal = std::filesystem::absolute (orignal.c_str ()).string ();
		idx = uint32_t (del + orignal.size ());
	}
	if (std::filesystem::exists (orignal)) {
		if (orignal[idx - 1] != ')') {
			int del = idx - orignal.size ();
			orignal.insert (idx, " (1)");
			idx += 4;
			idx = uint32_t (del + orignal.size ());
		}
		{
			int end = idx - 2;
			int start = end + 1;
			while (orignal[start] != '(' && start >= 0)
				start--;
			if (orignal[start] != '(' || start == end) { // condn -> __().txt
				orignal.insert (idx, " (1)");
				idx += 4;
				end = idx - 2;//same place
				start = end - 1;
			}
			start++;
			uint32_t num = 1;
			uint32_t num_base = 1;
			{
				std::string tmp = orignal.substr (start, end + 1);
				int check = atoi (tmp.c_str ());
				if (check != 1) { // tried different config, this seems most reasonable, windows too handle it similarly(i think)
					orignal.insert (idx, " (1)");
					idx += 4;
					end = idx - 2;//same place
					start = end;
				}

			}
			while (std::filesystem::exists (orignal)) {
				num++;
				if (num/num_base > 9) { // we need more chars
					orignal.insert (start, "0");
					idx++;
					end++;
					num_base *= 10;
				}
				int temp_end = end;
				for (int numb = num; numb > 0; numb /= 10) {
					char char_val = char ((numb % 10) + 48);
					orignal[temp_end] = char_val;
					MY_ASSERT(temp_end < start);
					temp_end--;
				}
			}
		}
	} else {
		uint32_t i = idx;
		while (orignal[i] != '\\')
			i--;
		std::string dir = orignal.substr (0, i);
		if (!std::filesystem::exists (dir))
			std::filesystem::create_directories (dir);
	}
	std::ofstream fout (orignal.c_str ()); // RAII
	return orignal;
}
std::string CreateNewFileIfNotExists (std::string orignal /*= std::string()*/)
{
	if (std::filesystem::exists (orignal))
	{
		return orignal;
	} else {
		return CreateNewFile (std::move (orignal));
	}
}
std::string GetFileExtension (const char *fileName_or_path, uint32_t size)
{
	if (size == 0)
		size = strlen (fileName_or_path);
	size--;
	while (fileName_or_path[size] != '.') {
		size--;
		MY_ASSERT (size != 0);
	}
	return std::string (&fileName_or_path[size + 1]);
}
std::string GetFileName (const char *fileName_or_path, uint32_t size)
{
	if (size == 0)
		size = strlen (fileName_or_path);
	while (fileName_or_path[size] != '\\') {
		size--;
		MY_ASSERT (size != 0);
	}
	return std::string (&fileName_or_path[size + 1]);
}