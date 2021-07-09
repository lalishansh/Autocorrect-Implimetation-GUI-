#include "GLCore.h"
#include "new_MVC_Layer.h"
//#include "MVC_Layer.h"

using namespace GLCore;

class MainApp : public Application
{
public:
	MainApp()
		: Application("Symspell", 600, 400)
	{
		// Using Model-View-Controller Design_Pattern
		//PushLayer (new MVC_Layer ());
		PushLayer (new new_MVC_Layer());
	}
};

int main()
{
	std::unique_ptr<MainApp> app = std::make_unique<MainApp>();
	app->Run();
}