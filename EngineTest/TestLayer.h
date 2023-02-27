#pragma once

#include "Application.h"
#include "Tools/Clock.h"
#include <Graphics/SceneCollection.h>

namespace sa {
	

	class TestLayer : public IApplicationLayer {
	private:

		Engine* m_pEngine;
		RenderWindow* m_pWindow;

		int randomRange(int min, int max);
		
	public:

		void onAttach(Engine& engine, RenderWindow& window) override;
		void onUpdate(float dt) override;
		void onImGuiRender() override;
	};

}