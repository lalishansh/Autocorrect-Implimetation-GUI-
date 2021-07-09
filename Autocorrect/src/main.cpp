#include "GLCore.h"
#include "MVC_Layer.h"

using namespace GLCore;

class MainApp : public Application
{
public:
	MainApp()
		: Application("OpenGL Examples", 600, 400)
	{
		// Using Model-View-Controller Design_Pattern
		PushLayer(new MVC_Layer());
	}
};

int main()
{
	std::unique_ptr<MainApp> app = std::make_unique<MainApp>();
	app->Run();
}