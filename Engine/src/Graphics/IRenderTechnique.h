#pragma once

#include "SceneCamera.h"
#include "Scene.h"
#include "Renderer.hpp"
#include "Resources\Texture.hpp"

namespace sa {

	struct RenderTarget {
		DynamicTexture colorTexture;
		ResourceID framebuffer = NULL_RESOURCE;
		ResourceID renderProgram = NULL_RESOURCE;
		ResourceID pipeline = NULL_RESOURCE;
		
		struct BloomData {
			bool isInitialized = false;

			ResourceID filterDescriptorSet = NULL_RESOURCE;
			std::vector<ResourceID> blurDescriptorSets;
			std::vector<ResourceID> upsampleDescriptorSets;
			ResourceID compositeDescriptorSet = NULL_RESOURCE;

			DynamicTexture2D bloomTexture;
			std::vector<DynamicTexture2D> bloomMipTextures;

			DynamicTexture2D bufferTexture;
			std::vector<DynamicTexture2D> bufferMipTextures;

			DynamicTexture2D outputTexture;

		} bloomData;

		DynamicTexture* outputTexture = nullptr;
	};


	struct DrawData {
		Texture colorTexture;
		Texture finalTexture;
	};

	class IRenderTechnique {
	protected:
		Renderer& m_renderer;


	public:
		DrawData drawData;


		IRenderTechnique();
		virtual ~IRenderTechnique() = default;

		virtual void init(Extent extent) = 0;
		virtual void cleanup() = 0;

		virtual void onWindowResize(Extent extent) = 0;

		virtual void updateData(RenderContext& context) = 0;
		
		virtual bool prepareRender(RenderContext& context, SceneCamera* pCamera) { return true; };
		virtual void render(RenderContext& context, SceneCamera* pCamera, ResourceID framebuffer) = 0;
		virtual void endRender(RenderContext& context) {};

		virtual ResourceID createColorFramebuffer(const DynamicTexture& colorTexture) = 0;

		virtual void updateLights(Scene* pScene) = 0;
		virtual void collectMeshes(Scene* pScene) = 0;

	};
}