#include "Backend.h"
#include <Windows.h>
#include <iostream>

class SandBox 
	: public Backend::Application 
{
public:
	SandBox() 
	{
		for (uint16_t i = 0;i < 51;i++) {
			m_WORD[i] = '\0';
		}
	}
	~SandBox()
	{}
public:
	virtual void Run() override {
		while (m_Running) {
			m_Window->OnUpdate();
		}
	}
	virtual void OnEvent(Backend::Event& e) override {
		using namespace Backend;
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>(HZ_BIND_EVENT_FN(SandBox::OnWindowClose));
		dispatcher.Dispatch<KeyPressedEvent>(HZ_BIND_EVENT_FN(SandBox::OnKeyPressed));
	}
private:
	bool OnKeyPressed(Backend::KeyPressedEvent& e) {
		if (e.GetKeyCode() == 32 || (e.GetKeyCode() > 'A'-1 && e.GetKeyCode() < 'Z'+1))
		{
			m_WORD[m_CursorPosn] = (unsigned char)e.GetKeyCode();// + 32;
			m_CursorPosn++;
		}
		if (e.GetKeyCode() == 259 && m_CursorPosn)
		{
			m_CursorPosn--;
			m_WORD[m_CursorPosn] = '\0';
		}
		Update();
		return true;
	}
private:
	void Update();
	unsigned char m_WORD[51] = { '\0' };
	uint32_t m_CursorPosn = 0;
};
void SandBox::Update() {
	system("CLS");
	std::cout << std::endl << ">> " << m_WORD <<'_'<< std::endl;
	std::cout << "====================================" << std::endl;
}

Backend::Application* Backend::CreateApplication() {
	return new SandBox();
}

int main() {
	using namespace Backend;
	Application* app = CreateApplication();
	app->Run();
	delete app;
}