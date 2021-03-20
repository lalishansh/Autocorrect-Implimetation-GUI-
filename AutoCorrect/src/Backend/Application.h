#pragma once
#pragma once

#include "Core.h"
#include "Window.h"

#include "Events/Event.h"
#include "Events/ApplicationEvent.h"
#include "Events/KeyEvent.h"
#include "Events/MouseEvent.h"


namespace Backend {

	class Application
	{
	public:
		Application();
		virtual ~Application();
		virtual void Run();

		virtual void OnEvent(Event& e);

		inline static Application& Get() { return *s_Instance; }
		inline Window& GetWindow() { return *m_Window; }
	private:
		static Application* s_Instance;

	private:
	protected:
		std::unique_ptr<Window> m_Window;
		bool m_Running = true;
	protected:
		bool OnWindowClose(WindowCloseEvent& e);
	};

	// To be defined in Client	
	Application* CreateApplication();
}