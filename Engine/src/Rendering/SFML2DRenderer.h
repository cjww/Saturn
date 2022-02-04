#pragma once
#include "IRenderTechnique.h"

namespace sa {
	class SFML2DRenderer : public IRenderTechnique {
		virtual void init(RenderWindow* pWindow, bool setupImGui = false) = 0;
		virtual void cleanup() = 0;

		virtual void beginFrameImGUI() = 0;

		virtual void draw(Scene* scene) = 0;

		virtual vr::Texture* getOutputTexture() const = 0;

		virtual vr::Texture* createShaderTexture2D(const vr::Image& img) = 0;
	};
}