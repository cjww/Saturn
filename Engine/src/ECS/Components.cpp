#include "pch.h"
#include "Components.h"

#include "AssetManager.h"

namespace comp {
	Model::Model(const std::string& name) {
		sa::AssetManager::get().loadModel(name);
	}
}
