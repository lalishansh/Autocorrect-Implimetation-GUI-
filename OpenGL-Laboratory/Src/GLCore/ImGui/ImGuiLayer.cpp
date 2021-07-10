#include "pch.h"
#include "ImGuiLayer.h"

#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_opengl3.h"

#include "../Core/Application.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>

namespace GLCore
{
	ImGuiLayer::ImGuiLayer ()
		: Layer ("ImGuiLayer")
	{}

	void ImGuiLayer::OnAttach ()
	{
		// Setup Dear ImGui context
		IMGUI_CHECKVERSION ();
		ImGui::CreateContext ();

		ImGuiIO &io = ImGui::GetIO ();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows

		// Setup Dear ImGui style
		ImGui::StyleColorsDark ();

		// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
		ImGuiStyle &style = ImGui::GetStyle ();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}

		Application &app = Application::Get ();
		GLFWwindow *window = static_cast<GLFWwindow *>(app.GetWindow ().GetNativeWindow ());

		// Setup Platform/Renderer bindings
		ImGui_ImplGlfw_InitForOpenGL (window, true);
		ImGui_ImplOpenGL3_Init ("#version 410");
	}

	void ImGuiLayer::OnDetach ()
	{
		ImGui_ImplOpenGL3_Shutdown ();
		ImGui_ImplGlfw_Shutdown ();
		ImGui::DestroyContext ();
	}

	void ImGuiLayer::Begin ()
	{
		ImGui_ImplOpenGL3_NewFrame ();
		ImGui_ImplGlfw_NewFrame ();
		ImGui::NewFrame ();

		ResetUniqueNameCount ();
	}

	void ImGuiLayer::End ()
	{
		ImGuiIO &io = ImGui::GetIO ();
		Application &app = Application::Get ();
		io.DisplaySize = ImVec2 ((float)app.GetWindow ().GetWidth (), (float)app.GetWindow ().GetHeight ());

		// Rendering
		ImGui::Render ();
		ImGui_ImplOpenGL3_RenderDrawData (ImGui::GetDrawData ());

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
			GLFWwindow *backup_current_context = glfwGetCurrentContext ();
			ImGui::UpdatePlatformWindows ();
			ImGui::RenderPlatformWindowsDefault ();
			glfwMakeContextCurrent (backup_current_context);
		}
	}

	const char *ImGuiLayer::UniqueName (const std::string &name)
	{
		return UniqueName (name.c_str ());
	}
	const char *ImGuiLayer::UniqueName (const char *name)
	{
		auto &map = Application::Get ().m_ImGuiLayer->s_UniqueNameMap;

		if (map.find (name) == map.end ()) {// Create Entry
			std::vector<std::string> _names = { std::string (name) };
			map.try_emplace (name, std::make_pair (0, std::move (_names)));
		}

		// check vector size, if small then insert entry
		std::pair<uint16_t, std::vector<std::string>> &entry = map.at (name);
		{
			for (uint16_t i = entry.second.size (); i < entry.first + 1; i++) {
				if (entry.second.size () == 1) // i.e. only 1 entry done till and that is of "name"
				{
					entry.second.clear ();
					entry.second.push_back (std::string (name) + "#" + std::to_string (1));
					entry.second.push_back (std::string (name) + "#" + std::to_string (2));
					i = 2;
					continue;
				}
				entry.second.push_back (std::string (name) + "#" + std::to_string (i + 1));
			}
		}
		entry.first++;
		return entry.second[entry.first - 1].c_str ();
	}

	void ImGuiLayer::ResetUniqueNameCount ()
	{
		auto &map = Application::Get ().m_ImGuiLayer->s_UniqueNameMap;
		for (auto &entry : map) {
			entry.second.first = 0;
		}
	}
}