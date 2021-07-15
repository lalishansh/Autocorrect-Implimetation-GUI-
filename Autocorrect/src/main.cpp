#include "GLCore.h"
#include "Raws/new_MVC_Layer.h"

// TODO: Update GLCore::Core (Input, KeyCode, MouseButtonCode)
// TODO: Update GLCore::Events (Event, KeyEvent, MouseEvent)
// TODO: Update GLCore::Events (Event, KeyEvent, MouseEvent)
// TODO: Update GLCore::Platform (WindowInput, windowswindow)
// TODO: Update GLCore.h

using namespace GLCore;

class MainApp : public Application
{
public:
	MainApp(int argc, char *argv[])
		: Application("Symspell", 600, 400, 0.8f)
	{
		//// Using Model-View-Controller Design_Pattern
		PushLayer (new new_MVC_Layer(argc, argv));

		//PushLayer (new )
	}
};

int main(int argc, char *argv[])
{
	std::unique_ptr<MainApp> app = std::make_unique<MainApp>(argc, argv);
	app->Run();
}