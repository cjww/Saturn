#pragma once

#include <Renderer.hpp>
#include <RenderWindow.hpp>
#include "Assets\Asset.h"
#include "ECS/Events.h"

namespace sa {

	class Engine;

	class RenderTarget : public Asset {
	private:
		friend class RenderPipeline;
		Renderer& m_renderer;

		bool m_isActive;

		Extent m_extent;
		bool m_wasResized;

		entt::connection m_windowResizedConnection;

		DynamicTexture* m_pOutputTexture = nullptr;

		entt::dispatcher* m_pDispatcher;

		void onWindowResized(const engine_event::WindowResized& e);
	public:

		RenderTarget();
		RenderTarget(const AssetHeader& header);

		virtual ~RenderTarget();

		void initialize(Engine* pEngine, Extent extent);
		void initialize(Engine* pEngine, RenderWindow* pWindow);
		void destroy();

		void resize(Extent extent);
		bool wasResized() const;

		bool isReady() const;

		const Extent& getExtent() const;

		const DynamicTexture* getOutputTextureDynamic() const;
		const Texture& getOutputTexture() const;
		void setOutputTexture(const DynamicTexture& dynamicTexture);

		void setActive(bool isActive);
		bool isActive() const;

		virtual bool onLoad(std::ifstream& file, AssetLoadFlags flags) override;
		virtual bool onWrite(std::ofstream& file, AssetWriteFlags flags) override;
		virtual bool onUnload() override;

	};
}
