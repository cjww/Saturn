#include "EngineEditor.h"

int main(int argc, char** args) {

	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);


	std::string projectPath;
	if (argc > 1) {
		projectPath = args[1];
	}
	else {
		//Create project

	}

	EngineEditor editor;

	try {
		editor.openProject(projectPath);
	}
	catch (const std::exception& e) {
		std::cout << e.what() << std::endl;
	}
	return 0;
}