#pragma once
#include <RenderWindow.hpp>

#include <Renderer.hpp>
#include "Camera.h"
namespace sa {
	class IRenderTechnique {
	protected:
		bool m_useImGui;

		std::set<Camera*> m_activeCameras;

	public:
		virtual void init(RenderWindow* pWindow, bool setupImGui = false) = 0;
		virtual void cleanup() = 0;

		bool isUsingImGui() const;
		virtual void beginFrameImGUI() = 0;

		virtual void draw() = 0;

		virtual void addCamera(Camera* camera);
		virtual void removeCamera(Camera* camera);

		virtual vr::Texture* getOutputTexture() const = 0;

		virtual vr::Texture* createShaderTexture2D(const vr::Image& img) = 0;
		
	};
}