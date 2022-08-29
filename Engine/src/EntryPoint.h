#pragma once
#define SA_PROFILER_ENABLE
#include "Application.h"

#include "Tools\Profiler.h"

extern sa::Application* sa::createApplication();

int main(int argv, char** args) {
	SA_PROFILER_BEGIN_SESSION();
	sa::Application* app = sa::createApplication();

	app->run();

	SA_PROFILER_END_SESSION();
	delete app;

}