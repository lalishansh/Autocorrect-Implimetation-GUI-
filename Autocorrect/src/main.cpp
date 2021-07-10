#include "GLCore.h"
#include "new_MVC_Layer.h"
//#include <wstring>
//#include "MVC_Layer.h"

using namespace GLCore;

class MainApp : public Application
{
public:
	MainApp(int argc, char *argv[])
		: Application("Symspell", 600, 400, 0.8f)
	{
		// Using Model-View-Controller Design_Pattern
		//PushLayer (new MVC_Layer ());
		PushLayer (new new_MVC_Layer(argc, argv));
	}
};

int main(int argc, char *argv[])
{
	std::unique_ptr<MainApp> app = std::make_unique<MainApp>(argc, argv);
	app->Run();
}