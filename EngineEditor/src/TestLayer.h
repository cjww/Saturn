#pragma once

#include "Application.h"
#include "SceneCamera.h"
#include "Tools/Clock.h"


namespace sa {
	

	class TestLayer : public IApplicationLayer {
	private:
		Clock m_infoClock;

		int m_row = 0;
		int m_column = 0;

		std::unordered_map<sa::Entity, ModelAsset*> m_completions;

		Engine* m_pEngine;
		RenderWindow* m_pWindow;

		int randomRange(int min, int max);
		float randomRange(float min, float max);

		Entity createModelEntity(Engine& engine, const std::filesystem::path& modelPath, float scale = 1.0f);
		Entity createDynamicBox(const Vector3& position, float scale = 1.0f);
	public:

		void onAttach(Engine& engine, RenderWindow& window) override;
		void onUpdate(float dt) override;
		void onImGuiRender() override;
	};

}