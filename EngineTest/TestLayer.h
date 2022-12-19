#pragma once

#include "Application.h"
#include "Tools/Clock.h"


namespace sa {
	

	class TestLayer : public IApplicationLayer {
	private:
		SceneCamera m_camera;

		Clock m_infoClock;

		int m_row = 0;
		int m_column = 0;

		std::unordered_map<sa::Entity, sa::ProgressView<ResourceID>&> m_completions;

		Engine* m_pEngine;
		RenderWindow* m_pWindow;

		RenderTarget m_renderTarget;
		Texture2D m_colorTexture;


		int randomRange(int min, int max);
		Entity createModelEntity(Engine& engine, const std::filesystem::path& modelPath, float scale = 1.0f);

	public:

		void onAttach(Engine& engine, RenderWindow& window) override;
		void onUpdate(float dt) override;
		void onImGuiRender() override;
	};

}