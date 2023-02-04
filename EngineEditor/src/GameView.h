#pragma once
#include "EditorModule.h"
#include "SceneCamera.h"

class GameView : public EditorModule {
private:
	sa::RenderTarget m_renderTarget;

	int m_mipLevel;

	bool m_renderedCamera;
	bool m_isWindowOpen;

	int m_resolutionIndex;

	std::array<sa::Extent, 6> m_Resolutions {
		sa::Extent{ 1920, 1080 },
		sa::Extent{ 1366, 768 },
		sa::Extent{ 1600, 900 },
		sa::Extent{ 1920, 1200 },
		sa::Extent{ 2560, 1440 },
		sa::Extent{ 3440, 1440 }
	};


public:
	GameView(sa::Engine* pEngine, sa::EngineEditor* pEditor, sa::RenderWindow* pWindow);

	virtual void update(float dt) override;
	virtual void onImGui() override;

};