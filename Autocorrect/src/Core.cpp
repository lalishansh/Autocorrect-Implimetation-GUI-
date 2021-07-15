#include "Core.h"
#include <imgui/imgui.h>

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

		float width = ImGui::GetContentRegionMax ().x/2 - 1.0f; // Debug
		
		Ready |= ImGui::Button ("Yes", ImVec2 (width, 0));
		Ready |= ImGui::Button ("No", ImVec2 (width, 0));

		ImGui::EndPopup ();
	}
	return Ready;
}