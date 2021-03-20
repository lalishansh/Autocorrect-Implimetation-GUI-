#pragma once
#include "Core.h"
#include <sstream>
#include <string>

namespace Backend {

	enum class EventType
	{
		Name = 0,
		WindowClose, WindowResize, WindowFocus, WindowLostFocus, WindowMoved,
		AppTick, AppUpdate, AppRender,
		KeyPressed, KeyRealeased, KeyTyped,
		MouseButtonPressed, MouseButtonRealeased, MouseMoved, MouseScrolled
	};
	enum EventCategory
	{
		Name = 0,
		APPLICATION = BIT(0),
		INPUT		= BIT(1),
		KEYBOARD	= BIT(2),
		MOUSE		= BIT(3),
		MOUSEBUTTON = BIT(4)
	};

#define EVENT_CLASS_TYPE(type) static EventType GetStaticType() { return EventType::##type; }\
							   virtual EventType GetEventType() const override { return GetStaticType(); }\
							   virtual const char* GetName() const override { return #type; }

#define EVENT_CLASS_CATEGORY(category) virtual int GetCategoryFlags() const override { return category; }

	class Event
	{
	public:
		bool Handled = false;

		virtual EventType GetEventType() const = 0;
		virtual const char* GetName() const = 0;
		virtual int GetCategoryFlags() const = 0;
		virtual std::string ToString() const { return GetName(); }

		inline bool IsInCategory(EventCategory category)
		{
			return GetCategoryFlags() & category;
		}
	protected:
	};

	class EventDispatcher
	{
	private:
		template<typename T>
		using EventFn = std::function<bool(T&)>;
	public:
		EventDispatcher(Event& event)
			: m_Event(event)
		{}

		template<typename T>
		bool Dispatch(EventFn<T> func) 
		{
			if (m_Event.GetEventType() == T::GetStaticType())
			{
				m_Event.Handled = func(*(T*)&m_Event);
				return true;
			}
			return false;
		}

	private:
		Event& m_Event;
	};

	inline std::ostream& operator<<(std::ostream& os, const Event& e)
	{
		return os << e.ToString();
	}
}