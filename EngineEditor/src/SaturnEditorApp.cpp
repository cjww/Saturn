#include <EntryPoint.h>
#include "EngineEditor.h"

namespace sa {
	class EditorApp : public Application {
	public:
		EditorApp(bool enableImgui = true)
			: Application(enableImgui) 
		{
			pushLayer(new EngineEditor);
		}

	};

	Application* createApplication() {
		Engine::setShaderDirectory("../Engine/shaders");
		return new EditorApp;
	}

}