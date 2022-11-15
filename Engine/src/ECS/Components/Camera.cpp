#include "pch.h"
#include "Camera.h"

#include "Application.h"


namespace comp {

	void Camera::setRenderTarget(sa::RenderTarget* pRenderTarget) {
		this->pRenderTarget = pRenderTarget;
		isPrimary = false;
	}

	void Camera::serialize(sa::Serializer& s) {
		s.value("isPrimary", isPrimary);
	}

	void Camera::deserialize(void* pDoc) {
		using namespace simdjson::ondemand;
		object& obj = *(object*)pDoc;
		isPrimary = obj["isPrimary"];
	}

	void Camera::onConstruct(sa::Entity* entity) {
		camera.setViewport({ { 0, 0 }, sa::Application::get()->getWindow()->getCurrentExtent() });
	
		
	}

	void Camera::onDestroy(sa::Entity* entity) {

		//sa::Application::get()->getEngine().erase(*(sa::Engine::connection<sa::engine_event::WindowResized>*)windowCallbackConnection);
	}



}