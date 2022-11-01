#pragma once
#include <Core.h>
#include <Engine.h>

#include "Events.h"

#include <crtdbg.h>
#include <stdio.h>          // printf, fprintf
#include <stdlib.h>         // abort

#include <Engine.h>
#include <RenderWindow.hpp>
#include <Tools\Clock.h>

class EditorModule {
protected:
	sa::Engine* m_pEngine;
public:
	EditorModule(sa::Engine* pEngine);
	virtual ~EditorModule();
	virtual void onImGui() = 0;
	virtual void update(float dt) = 0;
};