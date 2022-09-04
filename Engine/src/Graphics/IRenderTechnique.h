#pragma once

#include "Camera.h"
#include "Scene.h"

#include "Renderer.hpp"
#include "RenderWindow.hpp"



namespace sa {
	class IRenderTechnique {
	private:
		ResourceID m_imGuiRenderProgram;
		ResourceID m_imGuiFramebuffer;

	protected:
		Renderer& m_renderer;

		bool m_useImGui;

		sa::RenderWindow* m_pWindow;


		void setupImGuiPass();

	public:
		IRenderTechnique();

		virtual void init(sa::RenderWindow* pWindow, bool setupImGui = false) = 0;
		virtual void cleanup() = 0;

		virtual void beginFrameImGUI() = 0;

		virtual void draw(Scene* scene) = 0;
		virtual sa::Texture getOutputTexture() const = 0;

		virtual void updateLights(Scene* pScene) = 0;

		void drawImGui(RenderContext& context);
		bool isUsingImGui() const;

		sa::Extent getCurrentExtent() const;
		
	};
}