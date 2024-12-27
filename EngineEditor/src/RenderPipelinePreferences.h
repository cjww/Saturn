#pragma once

#include "EditorModule.h"

class RenderPipelinePreferences : public EditorModule {
private:
	
public:
	RenderPipelinePreferences(sa::Engine* pEngine, sa::EngineEditor* pEditor);
	virtual ~RenderPipelinePreferences();
	virtual void onImGui() override;
	virtual void update(float dt) override;
};