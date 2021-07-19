#pragma once

#include <GLCore.h>
#include <GLCoreUtils.h>

class MVC_Layer_old : public GLCore::Layer
{
public:
	MVC_Layer_old();
	virtual ~MVC_Layer_old();

	virtual void OnAttach() override;
	virtual void OnDetach() override;
	virtual void OnEvent(GLCore::Event& event) override;

	std::string ExtractLastWordFromTextBuffer ();

	virtual void OnUpdate (GLCore::Timestep ts) override;
	virtual void OnImGuiRender() override;

private:
	void ImGuiRenderDockables ();
	void LoadADictionary (std::string filePath);
	void LoadADictionary ();
	void LookupWordInDictionary (std::string &word);
	void MenuBarItems ();

public:
	static bool s_TextBoxSelected;
	static bool s_ResetFocusOnTextbox;
	static ImGuiID s_DockspaceID;
	static bool s_ShowAllLoadedDictionaries;
};