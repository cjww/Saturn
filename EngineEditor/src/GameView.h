#pragma once
#include "EditorModule.h"
#include "Camera.h"

class GameView : public EditorModule {
private:
	sa::Texture2D m_colorTexture;
	sa::RenderTarget m_renderTarget;
	sa::Camera m_camera;
public:
	GameView(sa::Engine* pEngine, sa::EngineEditor* pEditor, sa::RenderWindow* pWindow);

	virtual void update(float dt) override;
	virtual void onImGui() override;

};