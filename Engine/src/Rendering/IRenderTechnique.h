#pragma once

#include "Camera.h"
#include "Scene.h"
namespace sa {
	class IRenderTechnique {
	protected:
		bool m_useImGui;

		
	public:
		virtual void init(RenderWindow* pWindow, bool setupImGui = false) = 0;
		virtual void cleanup() = 0;

		bool isUsingImGui() const;
		virtual void beginFrameImGUI() = 0;

		virtual void draw(Scene* scene) = 0;

		virtual vr::Texture* getOutputTexture() const = 0;

		virtual vr::Texture* createShaderTexture2D(const vr::Image& img) = 0;
		
	};
}