#include "pch.h"
#include "SFML2DRenderer.h"

#include "SFML\Graphics.hpp"

void sa::SFML2DRenderer::init(RenderWindow* pWindow, bool setupImGui) {
    m_pTargetTexture = new sf::RenderTexture;
    m_pTargetTexture->create(pWindow->getCurrentExtent().x, pWindow->getCurrentExtent().y);
}

void sa::SFML2DRenderer::cleanup() {
    delete m_pTargetTexture;
}

void sa::SFML2DRenderer::beginFrameImGUI() {

}

void sa::SFML2DRenderer::draw(Scene* scene) {
    m_pTargetTexture->clear();

    m_pTargetTexture->display();
}

vr::Texture* sa::SFML2DRenderer::getOutputTexture() const {
    return nullptr;
}

vr::Texture* sa::SFML2DRenderer::createShaderTexture2D(const vr::Image& img) {
    return nullptr;
}
