#pragma once

#include "PipelineLayout.hpp"
#include "Shader.hpp"
#include "RenderContext.hpp"
#include "SceneCamera.h"
#include "Resources/DynamicBuffer.hpp"

namespace sa {
	class DebugRenderer {
	private:
		
		struct Vertex {
			glm::vec3 position;
			Color color;
		};

		std::array<Shader, 2> m_shaders;

		ResourceID m_pipeline;
		ResourceID m_wireframePipeline;
		PipelineLayout m_pipelineLayout;

		bool m_isInitialized;

		DynamicBuffer m_lineVertexBuffer;

		std::vector<Vertex> m_lineList;


		DebugRenderer();
	public:
		static DebugRenderer& Get();

		void initialize(ResourceID renderProgram);
		bool isInitialized() const;

		void render(RenderContext& context, Extent extent, const SceneCamera& sceneCamera);

		void drawLine(const glm::vec3& p1, const glm::vec3& p2, Color color = Color::White);
		void drawLines(glm::vec3* pPoints, uint32_t pointCount, Color color = Color::White);
		void drawLineLoop(glm::vec3* pPoints, uint32_t pointCount, Color color = Color::White);

	};
}