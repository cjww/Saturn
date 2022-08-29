#pragma once

#include "Camera.h"
#include "Scene.h"

#include "Renderer.hpp"
#include "RenderWindow.hpp"



namespace sa {
	class IRenderTechnique {
	protected:
		bool m_useImGui;

		sa::RenderWindow* m_pWindow;
		
	public:
		virtual void init(sa::RenderWindow* pWindow, bool setupImGui = false) = 0;
		virtual void cleanup() = 0;

		virtual void beginFrameImGUI() = 0;

		virtual void draw(Scene* scene) = 0;
		virtual sa::Texture getOutputTexture() const = 0;

		virtual void updateLights(Scene* pScene) = 0;

		bool isUsingImGui() const;

		sa::Extent getCurrentExtent() const;
		
	};
}