#include "DirectoryView.h"

#include "AssetManager.h"
#include "Assets/ModelAsset.h"

#include "EngineEditor.h"

DirectoryView::DirectoryView(sa::Engine* pEngine, sa::EngineEditor* pEditor) 
	: EditorModule(pEngine, pEditor, "Directory View", true)
{
	m_isOpen = false;
	
	m_pEngine->on<sa::editor_event::DragDropped>([&](const sa::editor_event::DragDropped& e, const sa::Engine& engine) {
		for (uint32_t i = 0; i < e.count; i++) {
			std::filesystem::path path = e.paths[i];
			if (sa::ModelAsset::isExtensionSupported(path.extension().generic_string())) {
				m_loadingAssets.push_back(sa::AssetManager::get().importAsset<sa::ModelAsset>(path));
			}
		}
	});

	sa::Image img(m_pEditor->editorRelativePath("resources/folder-white.png").generic_string());
	ImGui::g_directoryIcon = sa::Texture2D(img, true);

	sa::Image img1(m_pEditor->editorRelativePath("resources/file-white.png").generic_string());
	ImGui::g_otherFileIcon = sa::Texture2D(img1, true);


}

void DirectoryView::onImGui() {
	
	if (!m_isOpen)
		return;
	
	if (ImGui::Begin(m_name, &m_isOpen)) {
		/*
		{
			static std::string path;
			ImGui::InputText("Path", &path);
		
			if (ImGui::Button("Import")) {
				sa::AssetManager::get().importAsset<sa::ModelAsset>(path);
			}
		}

		ImGui::Separator();
		*/

		float completion = 0.f;
		for (auto it = m_loadingAssets.begin(); it != m_loadingAssets.end(); it++) {
			sa::IAsset* asset = *it;
			if (!asset->getProgress().isAllDone()) {
				completion += asset->getProgress().getAllCompletion();
			}
			else if (asset->isLoaded()) {
				asset->write();
				m_loadingAssets.erase(it);
				break;
			}
		}
		if (!m_loadingAssets.empty()) {
			completion /= m_loadingAssets.size();
			ImGui::ProgressBar(completion);
		}

		static auto openDirectory = std::filesystem::current_path();
		static int iconSize = 30;

		ImGui::DirectoryIcons("Explorer", openDirectory, iconSize);

		/*
		auto& assets = sa::AssetManager::get().getAssets();
		static sa::IAsset* selected = nullptr;

		if (ImGui::BeginListBox("Assets")) {
			for (auto& [id, asset] : assets) {
				std::string label = asset->getName() + "\t"
					+ sa::AssetManager::get().getAssetTypeName(asset->getType());
				if (asset->isLoaded()) {
					label += "\tLoaded";
				}
				else if (!asset->getProgress().isAllDone()) {
					ImGui::ProgressBar(asset->getProgress().getAllCompletion());
					label += "\tUnloaded";
				}
				else {
					label += "\tUnloaded";
				}

				label += "\t" + std::to_string(asset->getHeader().version);

				label += "\t" + asset->getAssetPath().generic_string();
				label += "\t" + std::to_string(asset->getReferenceCount());

				if (ImGui::Selectable(label.c_str(), selected == asset.get())) {
					selected = asset.get();
				}
			}

			ImGui::EndListBox();
		}

		if (ImGui::Button("Load Asset") && selected) {
			selected->load();
		}
		
		if (ImGui::Button("Release Asset") && selected) {
			selected->release();
		}

		if (ImGui::Button("Write Asset") && selected) {
			selected->write();
		}

		if (selected && selected->isLoaded()) {
			if (selected->getType() == sa::ModelAsset::type()) {
				if (ImGui::Button("Spawn")) {
					sa::Entity entity = m_pEngine->getCurrentScene()->createEntity();
					entity.addComponent<comp::Transform>();
					entity.addComponent<comp::Model>()->modelID = selected->getID();
				}
			}
			else if (selected->getType() == sa::Scene::type()) {
				if (ImGui::Button("Set Scene")) {
					m_pEngine->setScene(static_cast<sa::Scene*>(selected));
				}
			}
		}
		*/

	}
	ImGui::End();

}

void DirectoryView::update(float dt) {

}
