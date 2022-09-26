#pragma once
#include "EditorModule.h"
class LuaConsole : public EditorModule {
private:

	enum class LogType {
		INFO,
		SUCCESS,
		WARNING,
		ERROR
	};

	struct LogItem {
		std::string msg;
		LogType type;
		LogItem(const std::string& msg, LogType type) {
			this->msg = msg;
			this->type = type;
		}
	};

	char m_inputBuffer[256];

	std::vector<LogItem> m_items;

public:
	using EditorModule::EditorModule;

	virtual void update(float dt) override;
	virtual void onImGui() override;


};

