
#include "EntryPoint.h"

#include "TestLayer.h"

namespace sa {

	class TestApp : public Application {
	private:
	public:
		TestApp()
			: Application()
		{
			pushLayer(new TestLayer());
		}

	};

	Application* createApplication() {
		return new TestApp;
	}
}