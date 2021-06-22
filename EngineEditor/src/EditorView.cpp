#include "EditorView.h"

EditorView::EditorView(const RenderWindow* pWindow) {
	Rect viewport;
	viewport.setSize(pWindow->getCurrentExtent() / 2);
	viewport.setPosition({ pWindow->getCurrentExtent().x / 4, 0 });
	m_camera.setViewport(viewport);

	m_camera.setPosition(glm::vec3(0, 0, 1));
	m_camera.lookAt(glm::vec3(0, 0, 0));


}

EditorView::~EditorView() {

}

Camera* EditorView::getCamera() {
	return &m_camera;
}
