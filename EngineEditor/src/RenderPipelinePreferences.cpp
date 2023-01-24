#include "RenderPipelinePreferences.h"

RenderPipelinePreferences::RenderPipelinePreferences(sa::Engine* pEngine, sa::EngineEditor* pEditor) 
	: EditorModule(pEngine, pEditor, "Render Pipeline Preferences", true) {

}

RenderPipelinePreferences::~RenderPipelinePreferences() {

}

void RenderPipelinePreferences::onImGui() {
	if (!m_isOpen)
		return;

	if (ImGui::Begin(m_name, &m_isOpen)) {
		static sa::BloomPreferences bloomPrefs = m_pEngine->getRenderPipeline().getLayer<sa::BloomRenderLayer>()->getBloomPreferences();
		bool changed = false;
		static bool autoUpdate = true;

		if (ImGui::CollapsingHeader("Bloom")) {
			if (ImGui::DragFloat("Threshold", &bloomPrefs.threshold, 0.1f)) {
				bloomPrefs.threshold = std::max(bloomPrefs.threshold, 0.0f);
				changed = true;
			}

			if (ImGui::DragFloat("Intensity", &bloomPrefs.intensity, 0.1f)) {
				bloomPrefs.intensity = std::max(bloomPrefs.intensity, 0.0f);

				changed = true;
			}

			if (ImGui::DragFloat("Spread", &bloomPrefs.spread, 0.1f)) {
				bloomPrefs.spread = std::max(bloomPrefs.spread, 0.0f);

				changed = true;
			}

			if (ImGui::InputInt("Gaussian Kernel Radius", &bloomPrefs.gaussData.kernelRadius, 1, 10)) {
				bloomPrefs.gaussData.kernelRadius = std::min(bloomPrefs.gaussData.kernelRadius, 6);
				bloomPrefs.gaussData.kernelRadius = std::max(bloomPrefs.gaussData.kernelRadius, 1);

				changed = true;
			}
		}

		if (ImGui::CollapsingHeader("Tonemapping")) {
			if (ImGui::DragFloat("Exposure", &bloomPrefs.tonemapPreferences.exposure, 0.1f)) {
				bloomPrefs.tonemapPreferences.exposure = std::max(bloomPrefs.tonemapPreferences.exposure, 0.0f);
				changed = true;
			}
			if (ImGui::DragFloat("Gamma", &bloomPrefs.tonemapPreferences.gamma, 0.1f)) {
				bloomPrefs.tonemapPreferences.gamma = std::max(bloomPrefs.tonemapPreferences.gamma, 0.0f);
				changed = true;
			}

		}


		if (ImGui::Button("Apply")) {
			m_pEngine->getRenderPipeline().getLayer<sa::BloomRenderLayer>()->setBloomPreferences(bloomPrefs);
		}
		ImGui::SameLine();
		ImGui::Checkbox("Update on change", &autoUpdate);

		if (autoUpdate && changed) {
			m_pEngine->getRenderPipeline().getLayer<sa::BloomRenderLayer>()->setBloomPreferences(bloomPrefs);
		}

	}
	ImGui::End();
	
}

void RenderPipelinePreferences::update(float dt) {

}
