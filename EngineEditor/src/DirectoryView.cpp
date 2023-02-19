#include "DirectoryView.h"

#include "AssetManager.h"
#include "Assets/ModelAsset.h"

#include "EngineEditor.h"

DirectoryView::DirectoryView(sa::Engine* pEngine, sa::EngineEditor* pEditor) 
	: EditorModule(pEngine, pEditor, "Directory View", true)
{
	m_isOpen = false;
	m_isAssetListOpen = false;

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
	
	if (ImGui::Begin(m_name, &m_isOpen, ImGuiWindowFlags_MenuBar)) {
		
		if (ImGui::BeginMenuBar()) {

			if (ImGui::MenuItem("Assets Window")) {
				m_isAssetListOpen = !m_isAssetListOpen;
			}

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
			ImGui::EndMenuBar();
		}

		static auto openDirectory = std::filesystem::current_path();
		static int iconSize = 30;

		ImGui::DirectoryIcons("Explorer", openDirectory, iconSize);

	}
	ImGui::End();

	if (!m_isAssetListOpen)
		return;
	if (ImGui::Begin("Assets", &m_isAssetListOpen)) {
		{
			static std::string path;
			ImGui::InputText("Path", &path);

			if (ImGui::Button("Import")) {
				sa::AssetManager::get().importAsset<sa::ModelAsset>(path);
			}
		}

		ImGui::Separator();

		auto& assets = sa::AssetManager::get().getAssets();
		static sa::IAsset* selected = nullptr;

		if (ImGui::Button("Load Asset") && selected) {
			selected->load();
		}
		ImGui::SameLine();
		if (ImGui::Button("Release Asset") && selected) {
			selected->release();
		}
		ImGui::SameLine();
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

		if (ImGui::BeginChildFrame(ImGui::GetCurrentWindow()->GetID("asset_table"), ImGui::GetContentRegionAvail())) {
			if (ImGui::BeginTable("Asset Table", 5, ImGuiTableFlags_SizingFixedFit)) {
				ImGui::TableSetupColumn("Name");
				ImGui::TableSetupColumn("Type");
				ImGui::TableSetupColumn("Is Loaded");
				ImGui::TableSetupColumn("Asset Path");
				ImGui::TableSetupColumn("References");

				ImGui::TableHeadersRow();

				for (auto& [id, asset] : assets) {
					ImGui::TableNextRow();

					ImGui::TableNextColumn();
					if (ImGui::Selectable(asset->getName().c_str(), selected == asset.get())) {
						selected = asset.get();
					}

					ImGui::TableNextColumn();
					ImGui::TextUnformatted(sa::AssetManager::get().getAssetTypeName(asset->getType()).c_str());

					ImGui::TableNextColumn();
					if (asset->isLoaded()) {
						ImGui::Text("Loaded");
					}
					else {
						ImGui::Text("Unloaded");
					}

					ImGui::TableNextColumn();
					if (!asset->isLoaded() && !asset->getProgress().isAllDone()) {
						ImGui::ProgressBar(asset->getProgress().getAllCompletion());
					}
					else {
						ImGui::TextUnformatted(asset->getAssetPath().generic_string().c_str());
					}

					ImGui::TableNextColumn();
					ImGui::Text("%d", asset->getReferenceCount());

				}
				ImGui::EndTable();
			}
		}
		ImGui::EndChildFrame();
	}
	ImGui::End();

}

void DirectoryView::update(float dt) {

}
