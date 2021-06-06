#include "Engine.h"

void Engine::loadXML(const std::filesystem::path& path, rapidxml::xml_document<>& xml, std::string& xmlStr) {
	std::ifstream file(path);
	if (!file.is_open()) {
		throw std::runtime_error("Failed to open file " + path.string());
	}
	xmlStr.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
	xmlStr.push_back('\0');
	file.close();
	xml.parse<0>(xmlStr.data());
}

void Engine::loadFromFile(const std::filesystem::path& configPath) {
	rapidxml::xml_document<> doc;
	std::string docStr;
	loadXML(configPath, doc, docStr);
	rapidxml::xml_node<>* rendererNode = doc.first_node("Renderer");
	if (rendererNode) {
		rapidxml::xml_attribute<>* attr = rendererNode->first_attribute("api");
		
	}
}

void Engine::setup(RenderWindow* pWindow, const std::filesystem::path& configPath) {
	
	m_pRenderTechnique = new ForwardRenderer;
	m_pRenderTechnique->init(pWindow);



}

void Engine::update() {
	m_frameTime.start = std::chrono::high_resolution_clock::now();

}

void Engine::draw() {
	m_pRenderTechnique->draw();
	m_frameTime.cpu = std::chrono::high_resolution_clock::now() - m_frameTime.start;
}

void Engine::cleanup() {
	
	ResourceManager::cleanup();
	m_pRenderTechnique->cleanup();
	delete m_pRenderTechnique;
}

std::chrono::duration<double, std::milli> Engine::getCPUFrameTime() const {
	return m_frameTime.cpu;
}


