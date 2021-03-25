#include <Backend.h>
#include <Windows.h>
#include <iostream>

#include <fcntl.h>
#include <io.h>
#include "Logic/include/SymSpell.h"
#include "main.h"


class SandBox 
	: public Backend::Application 
{
public:
	SandBox() 
	{
		for (uint16_t i = 0;i < 51;i++) {
			m_StringStream[i] = '\0';
			if(i<29)
				m_WORD[i] = '\0';
		}
		m_StringStream[0] = ' ';
		InitSymSpell();
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
		if ((e.GetKeyCode() > 'A'-1 && e.GetKeyCode() < 'Z'+1) && m_CursorPosn < 51)
		{
			m_StringStream[m_CursorPosn] = (unsigned char)e.GetKeyCode() + 32;
			m_CursorPosn++;
		}
		if (e.GetKeyCode() == 259 && m_CursorPosn > 1)
		{
			m_CursorPosn--;
			if (m_WORDPosnInStream > m_CursorPosn) {
				m_WORDPosnInStream -= 2;
				while(m_StringStream[m_WORDPosnInStream] != ' ' && m_WORDPosnInStream > 0)
					m_WORDPosnInStream--;
				m_WORDPosnInStream++;
				if (m_WORDPosnInStream > 50) {
					m_WORDPosnInStream = 0;
				}
			}
			m_StringStream[m_CursorPosn] = '\0';
		}
		if (e.GetKeyCode() == 32) {
			m_StringStream[m_CursorPosn] = 32;// or ' '
			m_WORDPosnInStream = m_CursorPosn + 1;
			m_CursorPosn++;
		}
		Update();
		return true;
	}
private:
	void Update();
	char m_StringStream[51] = { '\0' };
	char m_WORD[29] = { '\0' };
	uint8_t m_CursorPosn = 1;
	uint8_t m_WORDPosnInStream = 1;
};
void SandBox::Update() {
	system("CLS");
	std::cout << std::endl << ">> " << m_StringStream <<'_'<< std::endl;
	std::cout << "====================================" << std::endl;
	uint8_t wordSize = m_CursorPosn - m_WORDPosnInStream;
	if (wordSize > 1) {
		for (uint8_t i = m_WORDPosnInStream; i < m_CursorPosn + 1; i++) {
			m_WORD[i - m_WORDPosnInStream] = m_StringStream[i];
		}
	}
	else {
		m_WORD[0] = '\0';
	}
	std::cout << std::endl << ">> " << m_WORD << " <<" << (int)m_WORDPosnInStream << " " << (int)m_CursorPosn << std::endl;
	std::cout << "====================================" << std::endl;
	if(m_WORD[0]!='\0')
		std::cout << std::endl << "| " << Test(m_WORD) << " |" << std::endl;
}
const int initialCapacity = 82765;
const int maxEditDistance = 2;
const int prefixLength = 3;
SymSpell symSpell(initialCapacity, maxEditDistance, prefixLength);
void InitSymSpell() {
	int start = clock();
	symSpell.LoadDictionary(".\\frequency_dictionary_en_82_765.txt", 0, 1, XL(' '));
	int end = clock();
	float time = (float)((end - start) / (CLOCKS_PER_SEC / 1000));
	xcout << XL("Library loaded: ") << time << XL(" ms") << endl;
}
std::string Test(xstring in) {
	return symSpell.Lookup(in, Verbosity::Closest)[0].term;
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