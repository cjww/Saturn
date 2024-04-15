#include <EntryPoint.h>
#include "EngineEditor.h"

namespace sa {
	class EditorApp : public Application {
	public:
		EditorApp(int argc, char** argv, bool enableImgui = true)
			: Application(enableImgui) 
		{
			EngineEditor* pEditor = new EngineEditor;
			pushLayer(pEditor);
			if (argc > 1) {
				std::filesystem::path projectPath = argv[1];
				if (std::filesystem::exists(projectPath))
					pEditor->openProject(projectPath);
			}
		}

	};

	Application* CreateApplication(int argc, char** argv) {
		Engine::SetShaderDirectory("../Engine/shaders");
		return new EditorApp(argc, argv);
	}

}