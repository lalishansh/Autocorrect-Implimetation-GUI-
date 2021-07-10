#pragma once

#include "pch.h"

#include "GLCore/Core/Core.h"
#include "GLCore/Events/Event.h"

namespace GLCore {

	struct WindowProps
	{
		std::string Title;
		uint32_t Width;
		uint32_t Height;
		float Opacity;

		WindowProps(const std::string& title = "OpenGL Sandbox",
			        uint32_t width = 1280,
			        uint32_t height = 720, 
					float opacity = 1.0f)
			: Title(title), Width(width), Height(height), Opacity(opacity)
		{
		}
	};

	// Interface representing a desktop system based Window
	class Window
	{
	public:
		using EventCallbackFn = std::function<void(Event&)>;

		virtual ~Window() = default;

		virtual void OnUpdate() = 0;

		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;

		// Window attributes
		virtual void SetEventCallback(const EventCallbackFn& callback) = 0;
		virtual void SetVSync(bool enabled) = 0;
		virtual bool IsVSync() const = 0;

		virtual void SetOpacity(float opacity) = 0;

		virtual void* GetNativeWindow() const = 0;

		static Window* Create(const WindowProps& props = WindowProps());
	};

}