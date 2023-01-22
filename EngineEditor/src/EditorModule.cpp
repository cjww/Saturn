#include "EditorModule.h"

EditorModule::EditorModule(sa::Engine* pEngine, sa::EngineEditor* pEditor, const char* name, bool isClosable)
	: m_pEngine(pEngine)
	, m_pEditor(pEditor)
	, m_name(name)
	, m_isClosable(isClosable)
	, m_isOpen(!isClosable)
{
}

EditorModule::~EditorModule() {

}


void EditorModule::open() {
	m_isOpen = true;
}

void EditorModule::close() {
	if (m_isClosable)
		m_isOpen = false;
}

const char* EditorModule::getName() const {
	return m_name;
}
