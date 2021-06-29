#pragma once
#include <Engine.h>

class EditorModule {
protected:
	sa::Engine* m_pEngine;
public:
	EditorModule(sa::Engine* pEngine);
	virtual void onImGui() = 0;
	virtual void update(float dt) = 0;
};