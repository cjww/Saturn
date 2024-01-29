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
		sa::BloomRenderLayer* pBloomLayer = m_pEngine->getRenderPipeline().getLayer<sa::BloomRenderLayer>();
		if (!pBloomLayer) {
			ImGui::End();
			return;
		}
		static sa::BloomPreferences bloomPrefs = pBloomLayer->getPreferences();
		bool changed = false;
		static bool autoUpdate = true;
		static bool bloomActive = true;
		if(ImGui::Checkbox("##active", &bloomActive)) {
			pBloomLayer->setActive(bloomActive);
		}

		ImGui::SameLine();
		if (ImGui::CollapsingHeader("Bloom")) {
			ImGui::PushID("Tonemapping");

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

			if (ImGui::Button("Reset")) {
				bloomPrefs = {};
				changed = true;
			}
			ImGui::PopID();
		}

		if (ImGui::CollapsingHeader("Tonemapping")) {
			ImGui::PushID("Tonemapping");
			if (ImGui::DragFloat("Exposure", &bloomPrefs.tonemapPreferences.exposure, 0.1f)) {
				bloomPrefs.tonemapPreferences.exposure = std::max(bloomPrefs.tonemapPreferences.exposure, 0.0f);
				changed = true;
			}
			if (ImGui::DragFloat("Gamma", &bloomPrefs.tonemapPreferences.gamma, 0.1f)) {
				bloomPrefs.tonemapPreferences.gamma = std::max(bloomPrefs.tonemapPreferences.gamma, 0.0f);
				changed = true;
			}

			if (ImGui::Button("Reset")) {
				bloomPrefs.tonemapPreferences = {};
				changed = true;
			}
			ImGui::PopID();
		}


		if (ImGui::Button("Apply")) {
			pBloomLayer->setBloomPreferences(bloomPrefs);
		}
		ImGui::SameLine();
		ImGui::Checkbox("Update on change", &autoUpdate);

		if (autoUpdate && changed) {
			pBloomLayer->setBloomPreferences(bloomPrefs);
		}

	}
	ImGui::End();
}

void RenderPipelinePreferences::update(float dt) {

}
