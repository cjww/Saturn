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

namespace sa {
	class EngineEditor;
}

class EditorModule {
protected:
	sa::Engine* m_pEngine;
	sa::EngineEditor* m_pEditor;

public:
	EditorModule(sa::Engine* pEngine, sa::EngineEditor* pEditor);
	virtual ~EditorModule();
	virtual void onImGui() = 0;
	virtual void update(float dt) = 0;
};