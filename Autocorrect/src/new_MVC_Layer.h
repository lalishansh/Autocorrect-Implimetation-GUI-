#pragma once

#include <GLCore.h>
#include <GLCoreUtils.h>
#include <imgui/imgui.h>
#include "Text_Object.h"

// Singleton
class new_MVC_Layer : public GLCore::Layer
{
public:
	new_MVC_Layer  () = default;
	~new_MVC_Layer () = default;

	virtual void OnAttach() override;
	virtual void OnDetach() override;
	virtual void OnEvent(GLCore::Event& event) override;
	
	virtual void OnUpdate (GLCore::Timestep ts) override;
	virtual void OnImGuiRender() override;

	void OpenFileAsText (const char* filePath = "");
	// void HomeScreen () {}
private:
	void ImGuiRenderDockables ();
	void MenuBarItems ();

private:
	ImGuiID m_DockspaceID;
	std::vector<Text_Object> m_TextFileObjects;
	// std::vector<std::string> m_FileHistory; // every-time any file is opened, History updates and then immediately saved on disk
	static const char *s_AppDataLocation;
};