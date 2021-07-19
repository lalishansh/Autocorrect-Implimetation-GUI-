#include <GLCore.h>
#include "Text_Entity.h"
#include "Manager_Layer.h"

// TODO: Update GLCore::Core (Input, KeyCode, MouseButtonCode)
// TODO: Update GLCore::Events (Event, KeyEvent, MouseEvent)
// TODO: Update GLCore::Events (Event, KeyEvent, MouseEvent)
// TODO: Update GLCore::Platform (WindowInput, windowswindow)
// TODO: Update GLCore.h

using namespace GLCore;

class MainApp : public Application
{
public:
	MainApp(int argc, char *argv[], char *envv[])
		: Application("Symspell", 600, 400, 0.8f)
	{
		//// Using Model-View-Controller Design_Pattern
		PushLayer (Manager_Layer::Create(argc, argv, envv));

		//PushLayer (new )
	}
};

int main(int argc, char *argv[], char *envv[])
{
	std::unique_ptr<MainApp> app = std::make_unique<MainApp>(argc, argv, envv);
	app->Run();
}