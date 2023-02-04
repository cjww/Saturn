#include "DirectoryView.h"

#include "AssetManager.h"
#include "Assets/ModelAsset.h"

DirectoryView::DirectoryView(sa::Engine* pEngine, sa::EngineEditor* pEditor) 
	: EditorModule(pEngine, pEditor, "Directory View", true)
{
	m_isOpen = false;
	/*
	{

		sa::ModelAsset* model = sa::AssetManager::get().importAsset<sa::ModelAsset>("resources/models/viking_room/scene.gltf");
		auto& header = model->getHeader();
		header.id = 1;
		header.offset = 256;
		header.size = 64;

		model->writeToFile("MyModelAsset.asset");
	}

	
	pEngine->on<sa::engine_event::SceneSet>([](sa::engine_event::SceneSet& e, sa::Engine& engine) {
		sa::ModelAsset* model = sa::AssetManager::get().loadAsset<sa::ModelAsset>("MyModelAsset.asset");
		sa::Entity entity = e.newScene->createEntity();
		entity.addComponent<comp::Transform>();
		entity.addComponent<comp::Model>()->modelID = model->getID();
	});

	sa::ModelAsset* model = sa::AssetManager::get().importAsset<sa::ModelAsset>("viking_room/scene.gltf");

	sa::ModelAsset modelAsset;
	modelAsset.importFromFile("viking_room/scene.gltf");
	size_t id = modelAsset.getID();
	sa::AssetManager::get().addAsset(modelAsset);



	modelAsset = sa::AssetManager::get().getAsset(id);
	*/

}

void DirectoryView::onImGui() {
	if (!m_isOpen)
		return;
	if (ImGui::Begin(m_name, &m_isOpen)) {

		static std::string assetName;
		ImGui::InputText("Asset Name", &assetName);
		
		if (ImGui::Button("Import Asset")) {
			sa::ModelAsset* model = sa::AssetManager::get().importAsset<sa::ModelAsset>(SA_ASSET_DIR + assetName);
		}

		ImGui::Separator();

		auto& assets = sa::AssetManager::get().getAssets();
		
		static sa::IAsset* selected = nullptr;
		if (ImGui::BeginListBox("Assets")) {
			for (auto& [id, asset] : assets) {
				std::string label = asset->getName() + "\t"
					+ (asset->isLoaded() ? "Loaded\t" : "Unloaded\t")
					+ asset->getAssetPath().generic_string() + "\t"
					+ std::to_string(asset->getHeader().id);

				if (ImGui::Selectable(label.c_str(), selected == asset.get())) {
					selected = asset.get();
				}
			}

			ImGui::EndListBox();
		}

		if (ImGui::Button("Load Asset")) {
			selected->load();
		}

		if (ImGui::Button("Write Asset")) {
			selected->write();
		}


		if (ImGui::Button("Spawn")) {
			if (!selected->isLoaded()) {
				SA_DEBUG_LOG_ERROR("Asset ", selected->getName(), " is not loaded!");
			}
			sa::Entity entity = m_pEngine->getCurrentScene()->createEntity();
			entity.addComponent<comp::Transform>();
			if (selected->getType() == sa::AssetType::MODEL) {
				sa::ModelAsset* pModel = static_cast<sa::ModelAsset*>(selected);
				entity.addComponent<comp::Model>()->modelID = pModel->getID();
			}
		}

	}
	ImGui::End();

}

void DirectoryView::update(float dt) {

}
