#pragma once

#include "Backend/Input.h"

namespace Backend {

	class WindowsInput : public Input
	{
	protected:
		inline bool IsKeyPressedImpl(int keycode) override;
		virtual bool IsMouseButtonPressedImpl(int button) override;
		virtual float GetMouseXImpl() override;
		virtual float GetMouseYImpl() override;
		std::pair<float, float> GetMousePosImpl() override;
	};

}