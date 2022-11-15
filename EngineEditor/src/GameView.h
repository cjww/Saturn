#pragma once
#include "EditorModule.h"
#include "SceneCamera.h"

class GameView : public EditorModule {
private:
	sa::Texture2D m_colorTexture;
	sa::RenderTarget m_renderTarget;

	bool m_renderedCamera;

	int m_resolutionIndex;

	std::array<sa::Extent, 2> m_Resolutions {
		sa::Extent{ 0, 0 },
		sa::Extent{ 1920, 1080 }
	};


public:
	GameView(sa::Engine* pEngine, sa::EngineEditor* pEditor, sa::RenderWindow* pWindow);

	virtual void update(float dt) override;
	virtual void onImGui() override;

};