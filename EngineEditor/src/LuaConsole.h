#pragma once
#include "EditorModule.h"
class LuaConsole : public EditorModule {
private:
	char m_inputBuffer[256];

public:
	using EditorModule::EditorModule;

	virtual void update(float dt) override;
	virtual void onImGui() override;


};

