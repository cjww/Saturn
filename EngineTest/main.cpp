
#include "EntryPoint.h"

#include "TestLayer.h"

namespace sa {

	class TestApp : public Application {
	private:
	public:
		TestApp(bool enableImgui = false)
			: Application(enableImgui)
		{
			pushLayer(new TestLayer());
		}

	};

	Application* createApplication() {
		Engine::setShaderDirectory("../Engine/shaders/");
		return new TestApp;
	}
}