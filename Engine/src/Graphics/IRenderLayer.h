#pragma once

#include "Renderer.hpp"

#include "SceneCamera.h"
#include "SceneCollection.h"
#include "RenderTarget.h"


namespace sa {
	class IRenderLayer {
	private:
		bool m_isActive;
	protected:
		Renderer& m_renderer;
	public:
		IRenderLayer();
		virtual ~IRenderLayer() = default;

		virtual void init() = 0;
		virtual void cleanup() = 0;
		
		virtual const Texture& render(RenderContext& context, SceneCamera* pCamera, RenderTarget* rendertarget, SceneCollection& sceneCollection) = 0;

		bool isActive() const;
		void setActive(bool active);

	};

}
