#pragma once
#include "Camera.h"
#include "Scene.h"
#include "Texture.h"

#include "Graphics\RenderWindow.h"
#include "Graphics\Image.h"

namespace vr {
	struct Texture;
}

namespace sa {
	class IRenderTechnique {
	protected:
		bool m_useImGui;

		sa::RenderWindow* m_pWindow;
		
	public:
		virtual void init(sa::RenderWindow* pWindow, bool setupImGui = false) = 0;
		virtual void cleanup() = 0;

		bool isUsingImGui() const;
		virtual void beginFrameImGUI() = 0;

		virtual void draw(Scene* scene) = 0;

		virtual sa::Texture getOutputTexture() const = 0;

		virtual sa::Texture createShaderTexture2D(const sa::Image& img) = 0;

		sa::Vector2u getCurrentExtent() const;
		
	};
}