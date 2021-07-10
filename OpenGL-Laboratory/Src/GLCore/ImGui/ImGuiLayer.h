#pragma once

#include "GLCore/Core/Layer.h"

#include "GLCore/Events/ApplicationEvent.h"
#include "GLCore/Events/KeyEvent.h"
#include "GLCore/Events/MouseEvent.h"

namespace GLCore
{

	class ImGuiLayer: public Layer
	{
	public:
		ImGuiLayer ();
		~ImGuiLayer () = default;

		virtual void OnAttach () override;
		virtual void OnDetach () override;

		void Begin ();
		void End ();

		static const char *UniqueName (const std::string &name);
		static const char *UniqueName (const char *name);
		static void ResetUniqueNameCount ();
		// TODO: Implement ImGui Theme Changer
	private:
		float m_Time = 0.0f;

		std::unordered_map<const char *, std::pair<uint16_t, std::vector<std::string>>> s_UniqueNameMap;
	};

}