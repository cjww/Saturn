#include "RenderPipelinePreferences.h"

#include "CustomImGui.h"

RenderPipelinePreferences::RenderPipelinePreferences(sa::Engine* pEngine, sa::EngineEditor* pEditor) 
	: EditorModule(pEngine, pEditor, "Render Pipeline Preferences", true) {

}

RenderPipelinePreferences::~RenderPipelinePreferences() {

}

void RenderPipelinePreferences::onImGui() {
	if (!m_isOpen)
		return;
	if (ImGui::Begin(m_name, &m_isOpen)) {

		static bool autoUpdate = true;

		bool changed = false;
		changed |= ImGui::RenderLayerPreferences<sa::ShadowRenderLayer>("Shadows", m_pEngine->getRenderPipeline());
		changed |= ImGui::RenderLayerPreferences<sa::BloomRenderLayer>("Bloom", m_pEngine->getRenderPipeline());

		if (ImGui::Button("Apply")) {
			m_pEngine->getRenderPipeline().updatePreferences();
		}
		ImGui::SameLine();
		ImGui::Checkbox("Update on change", &autoUpdate);

		if (autoUpdate && changed) {
			m_pEngine->getRenderPipeline().updatePreferences();
		}

	}
	ImGui::End();
}

void RenderPipelinePreferences::update(float dt) {

}
