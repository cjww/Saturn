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
private:
	bool m_isClosable;
protected:
	sa::Engine* m_pEngine;
	sa::EngineEditor* m_pEditor;
	bool m_isOpen;
	const char* m_name;

public:
	EditorModule(sa::Engine* pEngine, sa::EngineEditor* pEditor, const char* name, bool isClosable);
	virtual ~EditorModule();
	virtual void onImGui() = 0;
	virtual void update(float dt) = 0;

	void open();
	void close();

	bool isOpen() const;
	void setOpen(bool open);

	const char* getName() const;
};