#include "new_MVC_Layer.h"
#include "GLCore/Core/Input.h"
#include "GLCore/Core/KeyCodes.h"
#include "GLCore/Core/MouseButtonCodes.h"
#include "SymSpell/include/SymSpell.h"
#include "imgui/imgui.h"
#include <thread>

using namespace GLCore;
using namespace GLCore::Utils;

const char *new_MVC_Layer::s_AppDataLocation = "C:\\Autocorrect_Text_Editor_Cache\\cache.txt";

void new_MVC_Layer::OnAttach()
{}

void new_MVC_Layer::OnDetach()
{}

void new_MVC_Layer::OnEvent(Event& event)
{}

void new_MVC_Layer::OnUpdate (Timestep ts)
{}

void new_MVC_Layer::OnImGuiRender()
{
	{// DockSpace

		static bool dockspaceOpen = true;
		static constexpr bool optFullscreenPersistant = true;
		bool optFullscreen = optFullscreenPersistant;

		static ImGuiDockNodeFlags dockspaceFlags = ImGuiDockNodeFlags_None;

		// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
		// because it would be confusing to have two docking targets within each others.
		ImGuiWindowFlags windowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
		if (optFullscreen) {
			ImGuiViewport *viewPort = ImGui::GetMainViewport ();
			ImGui::SetNextWindowPos (viewPort->Pos);
			ImGui::SetNextWindowSize (viewPort->Size);
			ImGui::SetNextWindowViewport (viewPort->ID);
			ImGui::PushStyleVar (ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar (ImGuiStyleVar_WindowBorderSize, 0.0f);

			windowFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		}

		// when using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background and handle the pass-thru hole, so we ask Begin() to not render background.
		if (dockspaceFlags & ImGuiDockNodeFlags_PassthruCentralNode)
			windowFlags |= ImGuiWindowFlags_NoBackground;

		// Important: note that we proceed even if Begin() return false (i.e window is collapsed).
		// This is because we want to keep our DockSpace() active. If a DockSpace is inactive,
		// all active windows docked into it will lose their parent and become undocked.
		// any change of dockspce/settings would lead towindows being stuck in limbo and never being visible.
		ImGui::PushStyleVar (ImGuiStyleVar_WindowPadding, ImVec2 (0.0f, 0.0f));
		ImGui::Begin ("Main DockSpace", &dockspaceOpen, windowFlags);
		ImGui::PopStyleVar ();

		if (optFullscreen)
			ImGui::PopStyleVar (2);

		// DockSpace
		ImGuiIO &io = ImGui::GetIO ();
		ImGuiStyle &style = ImGui::GetStyle ();
		float defaultMinWinSize = style.WindowMinSize.x;
		style.WindowMinSize.x = 280;

		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
			m_DockspaceID = ImGui::GetID ("MyDockSpace");
			ImGui::DockSpace (m_DockspaceID, ImVec2 (0.0f, 0.0f), dockspaceFlags);
		}

		style.WindowMinSize.x = defaultMinWinSize;

		// DockSpace's MenuBar
		if (ImGui::BeginMenuBar ()) {
			if (ImGui::BeginMenu ("Main")) {
				// Disabling fullscreen would allow the window to be moved to the front of other windows, 
				// which we can't undo at the moment without finer window depth/z control.
				//ImGui::MenuItem("Fullscreen", NULL, &opt_fullscreen_persistant);
				if (ImGui::MenuItem ("Exit")) 
					Application::Get ().ApplicationClose ();
				
				
				ImGui::EndMenu ();
			}
			MenuBarItems ();
			ImGui::EndMenuBar ();
		}

		// Here goes Stuff that will be put inside DockSpace
		ImGuiRenderDockables ();

		ImGui::End ();
	}
}
void new_MVC_Layer::MenuBarItems ()
{
	if (ImGui::BeginMenu ("File")) {
		if(ImGui::MenuItem ("Open"))
			OpenFileAsText(GLCore::Utils::FileDialogs::OpenFile ("all files (*.*)\0*.txt\0").c_str());
		ImGui::EndMenu ();
	}
}
void new_MVC_Layer::ImGuiRenderDockables ()
{
	for (uint32_t i = 0; i < m_TextFileObjects.size (); i++) {
		ImGui::SetNextWindowDockID (m_DockspaceID, (i == 0) ? ImGuiCond_Always : ImGuiCond_Once);
		
		ImGui::PushID (i);
		ImGui::Begin (m_TextFileObjects[i].m_FileName); // NOTE: they may have same name

		m_TextFileObjects[i].ImGuiTextRender ();

		ImGui::End ();
		ImGui::PopID ();
	}
}


void new_MVC_Layer::OpenFileAsText (const char *filePath /*= ""*/)
{
	auto Obj = Text_Object::OpenFileAsText (filePath);
	if (Obj)
	{
		m_TextFileObjects.push_back (std::move (Obj.value ()));
	}
}