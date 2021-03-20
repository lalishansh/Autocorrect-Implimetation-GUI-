#pragma once

#include "Event.h"

namespace Backend {
	class KeyEvent :public Event
	{
	public:
		inline int GetKeyCode() const { return m_KeyCode; };

		EVENT_CLASS_CATEGORY(EventCategory::KEYBOARD | EventCategory::INPUT);
	protected:
		KeyEvent(int keyCode)
			: m_KeyCode(keyCode)
		{}

		int m_KeyCode;
	};

	class KeyPressedEvent : public KeyEvent
	{
	public:
		KeyPressedEvent(int keyCode, int repeatCount)
			: KeyEvent(keyCode), m_RepeatCount(repeatCount)
		{}

		inline int GetRepeatCount() const { return m_RepeatCount; }

		std::string ToString() const override {
			std::stringstream ss;
			ss << "KeyPressedEvent  : " << "\"" << (char)MIN(m_KeyCode , (unsigned char)(-1)) << "\"" << ": " << m_KeyCode << (m_KeyCode>99 ? " (" : "  (") << m_RepeatCount << " repeats)";
			return ss.str();
		}

		EVENT_CLASS_TYPE(KeyPressed)
	private: 
		int m_RepeatCount;
	};

	class KeyTypedEvent : public KeyEvent
	{
	public:
		KeyTypedEvent(int keyCode)
			: KeyEvent(keyCode)
		{}

		std::string ToString() const override {
			std::stringstream ss;
			ss << "KeyTypedEvent    : " << "\"" << (char)MIN(m_KeyCode, (unsigned char)(-1)) << "\"" << ": " << m_KeyCode ;
			return ss.str();
		}

		EVENT_CLASS_TYPE(KeyTyped)
	};

	class KeyRealeasedEvent : public KeyEvent
	{
	public:
		KeyRealeasedEvent(int keyCode)
			: KeyEvent(keyCode)
		{}

		std::string ToString() const override {
			std::stringstream ss;
			ss << "KeyRealeasedEvent: " << "\"" << (char)MIN(m_KeyCode, (unsigned char)(-1)) << "\"" << ": "<<m_KeyCode ;
			return ss.str();
		}

		EVENT_CLASS_TYPE(KeyRealeased)
	};
}