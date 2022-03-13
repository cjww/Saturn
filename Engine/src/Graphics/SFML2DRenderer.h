#pragma once
#include "IRenderTechnique.h"

namespace sf {
	class RenderTexture;
}

namespace sa {
	class SFML2DRenderer : public IRenderTechnique {
	private:
		sf::RenderTexture* m_pTargetTexture;

	public:


		virtual void init(RenderWindow* pWindow, bool setupImGui = false) override;
		virtual void cleanup() override;

		virtual void beginFrameImGUI() override;

		virtual void draw(Scene* scene) override;

		virtual sa::Texture getOutputTexture() const override;

		virtual vr::Texture* createShaderTexture2D(const vr::Image& img) override;
	};
}