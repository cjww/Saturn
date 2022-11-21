#pragma once
#include "EditorModule.h"
#include "SceneCamera.h"

class GameView : public EditorModule {
private:
	sa::Texture2D m_colorTexture;
	sa::RenderTarget m_renderTarget;

	std::vector<sa::Texture2D> m_bloomMipTextures;
	std::vector<sa::Texture2D> m_bufferMipTextures;

	int m_mipLevel;

	bool m_renderedCamera;
	bool m_isWindowOpen;

	int m_resolutionIndex;

	std::array<sa::Extent, 7> m_Resolutions {
		sa::Extent{ 0, 0 },
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