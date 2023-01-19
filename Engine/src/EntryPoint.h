#pragma once

#ifdef _WIN32
#include <stdlib.h>
#include <crtdbg.h>
#endif

#include "Core.h"

#include "Application.h"

#include "Tools\Profiler.h"

extern sa::Application* sa::createApplication();

int main(int argv, char** args) {
#ifdef _WIN32
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif	
	sa::Application* app = sa::createApplication();

	app->run();

	delete app;
}