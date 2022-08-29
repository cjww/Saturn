#pragma once
#include "Application.h"

extern sa::Application* sa::createApplication();

int main(int argv, char** args) {

	sa::Application* app = sa::createApplication();

	app->run();

	delete app;

}