#include "Application.h"
#include <iostream>
#include "Input.h"

namespace Backend {

	Application* Application::s_Instance = nullptr;

	Application::Application ()
	{
		s_Instance = this;

		m_Window = std::unique_ptr<Window> (Window::Create ({ "Autocorrect Test", 150, 60 }));
		m_Window->SetEventCallback(HZ_BIND_EVENT_FN(Application::OnEvent));
	}
	Application::~Application()
	{}

	void Application::Run()
	{
		while (m_Running) {
			m_Window->OnUpdate();
		};
	}

	void Application::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>(HZ_BIND_EVENT_FN(Application::OnWindowClose));
	}
	bool Application::OnWindowClose(WindowCloseEvent& e)
	{
		std::cout << std::endl << e;
		m_Running = false;
		return true;
	}
}