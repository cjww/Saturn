#include "SceneView.h"

#include "Graphics\RenderTechniques\ForwardPlus.h"

SceneView::SceneView(sa::Engine* pEngine, sa::RenderWindow* pWindow)
	: EditorModule(pEngine)
{
	m_pWindow = pWindow;
	m_isFocused = false;
	m_isWorldCoordinates = false;
	m_selectedEntity = {};

	sa::Rect viewport = { { 0, 0 }, pWindow->getCurrentExtent() };
	m_camera.setViewport(viewport);

	m_camera.setPosition(sa::Vector3(0, 0, 5));
	m_camera.lookAt(sa::Vector3(0, 0, 0));

	m_mouseSensitivity = 30.0f;

	m_velocity = glm::vec3(0.0f);
	m_maxVelocityMagnitude = 30.0f;
	m_acceleration = 0.3f;

	m_statsUpdateTime = 0.1f;
	m_statsTimer = m_statsUpdateTime;

	pEngine->on<sa::engine_event::SceneSet>([&](const sa::engine_event::SceneSet& sceneSetEvent, sa::Engine& engine) {
		m_selectedEntity = {};
		sceneSetEvent.newScene->addActiveCamera(&m_camera);

		sceneSetEvent.newScene->on<sa::editor_event::EntitySelected>([&](const sa::editor_event::EntitySelected& e, sa::Scene&) {
			m_selectedEntity = e.entity;
		});

		sceneSetEvent.newScene->on<sa::editor_event::EntityDeselected>([&](const sa::editor_event::EntityDeselected&, sa::Scene&) {
			m_selectedEntity = {};
		});
	});
	m_zoom = 0.f;
	m_pWindow->addScrollCallback([&](double x, double y) {
		m_zoom = y;
	});

}

SceneView::~SceneView() {

}

void SceneView::update(float dt) {
	SA_PROFILE_FUNCTION();

	m_deltaTime = dt;
	
	m_statsTimer -= dt;
	if (m_statsTimer < 0.0f) {
		m_statsTimer = m_statsUpdateTime;
		m_statistics.frameTime = dt;
		m_statistics.gpuMemoryStats = sa::Renderer::get().getGPUMemoryUsage();
		
		std::copy(m_frameTimeGraph.begin() + 1, m_frameTimeGraph.end(), m_frameTimeGraph.begin());
		m_frameTimeGraph[m_frameTimeGraph.size() - 1] = dt * 1000;
		
	}

	if (!m_isFocused) {
		m_lastMousePos = m_pWindow->getCursorPosition();
		return;
	}

	sa::Vector2 mousePos = m_pWindow->getCursorPosition();
	if (m_pWindow->getMouseButton(sa::MouseButton::RIGHT)) {
		// First-person controls
		sa::Vector2 delta = m_lastMousePos - mousePos;
		m_camera.rotate(glm::radians(delta.x) * dt * m_mouseSensitivity, { 0, 1, 0 });
		m_camera.rotate(glm::radians(delta.y) * dt * m_mouseSensitivity, m_camera.getRight());
		
		int forward = m_pWindow->getKey(sa::Key::W) - m_pWindow->getKey(sa::Key::S);
		int right = m_pWindow->getKey(sa::Key::D) - m_pWindow->getKey(sa::Key::A);
		int up = m_pWindow->getKey(sa::Key::Q) - m_pWindow->getKey(sa::Key::E);

		if ((forward | right | up) == 0) {
			m_velocity = glm::vec3(0);
		}
		else {
			m_velocity += (float)forward * dt * m_camera.getForward() * m_acceleration;
			m_velocity += (float)right * dt * m_camera.getRight() * m_acceleration;
			m_velocity += (float)up * dt * m_camera.getUp() * m_acceleration;

			if (glm::length(m_velocity) > m_maxVelocityMagnitude) {
				m_velocity = glm::normalize(m_velocity);
				m_velocity *= m_maxVelocityMagnitude;
			}
		
			glm::vec3 camPos = m_camera.getPosition();
			camPos += m_velocity;
			
			m_camera.setPosition(camPos);
		}
		/*
		glm::vec3 camPos = m_camera.getPosition();
		camPos += (float)forward * dt * m_moveSpeed * m_camera.getForward();
		camPos += (float)right * dt * m_moveSpeed * m_camera.getRight();
		camPos += (float)up * dt * m_moveSpeed * m_camera.getUp();
		*/
	}
	else if (m_pWindow->getMouseButton(sa::MouseButton::MIDDLE) && m_isFocused) {
		glm::vec2 delta = m_lastMousePos - mousePos;
		glm::vec3 camPos = m_camera.getPosition();
		camPos += m_camera.getRight() * delta.x * dt * m_mouseSensitivity;
		camPos += m_camera.getUp() * delta.y * dt * m_mouseSensitivity;
		m_camera.setPosition(camPos);
	}
	else {
		m_velocity = glm::vec3(0);
	}

	if (m_zoom) {
		if(m_isFocused)
			m_camera.setPosition(m_camera.getPosition() + m_camera.getForward() * (m_zoom * 5));
		m_zoom = 0;
	}

	m_lastMousePos = mousePos;
		

}

void SceneView::onImGui() {
	SA_PROFILE_FUNCTION();

	if (ImGui::Begin("Scene view", 0, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove)) {

		static ImGuizmo::OPERATION operation = ImGuizmo::TRANSLATE;
		static float snapDistance = 0.5f;
		static float snapAngle = 45.0f;
		static int iconSize = 16;
		static bool showIcons = true;

		if (ImGui::BeginMenuBar()) {
			static bool showStats = false;

			if (showStats) {
				ImGui::SetNextWindowBgAlpha(0.3f);
				if (ImGui::Begin("Statistics", 0, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoFocusOnAppearing)) {

					ImGui::Text("Entity Count: %llu", m_pEngine->getCurrentScene()->getEntityCount());
					
					ImGui::Text("FPS: %f", 1 / m_statistics.frameTime);
					ImGui::Text("Frame time: %f ms", m_statistics.frameTime * 1000);
					ImGui::PlotLines("Frame time", m_frameTimeGraph.data(), m_frameTimeGraph.size(), 0, 0, 0.0f, 100.f, ImVec2{0, 50});
				
					ImGui::Text("GPU Memory usage");
					size_t totalUsage = 0;
					size_t totalBudget = 0;

					ImGui::Indent();
					for (auto& heap : m_statistics.gpuMemoryStats.heaps) {
						ImGui::Text("%llu MB / %llu MB, flags: %u", heap.usage / 1000000, heap.budget / 1000000, heap.flags);
						totalUsage += heap.usage;
						totalBudget += heap.budget;
					}
					ImGui::Text("Total usage: %llu MB / %llu MB", totalUsage / 1000000, totalBudget / 1000000);
					ImGui::Unindent();
				}
				ImGui::End();
			}
			
			
			
			if (ImGui::BeginMenu("View Settings")) {
				ImGui::Checkbox("Statistics", &showStats);

				ImGui::DragFloat("Snap Distance", &snapDistance, 0.5f, 0.0f, 100.f, "%.1f m");
				ImGui::DragFloat("Snap Angle", &snapAngle, 1.f, 1.f, 180.f, "%.0f degrees");

				static bool showLightHeatmap = false;
				sa::ForwardPlus* forwardPlusTechnique = dynamic_cast<sa::ForwardPlus*>(m_pEngine->getRenderPipeline().getRenderTechnique());
				if (forwardPlusTechnique) {
					if (ImGui::Checkbox("Show Light Heatmap", &showLightHeatmap)) {
						forwardPlusTechnique->setShowHeatmap(showLightHeatmap);
					}
				}

				ImGui::Checkbox("Show Icons", &showIcons);
				if (!showIcons) {
					ImGui::BeginDisabled();
				}
				ImGui::SliderInt("Icon Size", &iconSize, 1, 100);
				if (!showIcons) {
					ImGui::EndDisabled();
				}

				ImGui::EndMenu();
			}

			ImGui::Checkbox("World Coordinates", &m_isWorldCoordinates);

			ImGui::RadioButton("T", (int*)&operation, ImGuizmo::OPERATION::TRANSLATE);
			ImGui::RadioButton("R", (int*)&operation, ImGuizmo::OPERATION::ROTATE);
			ImGui::RadioButton("S", (int*)&operation, ImGuizmo::OPERATION::SCALE);
		}
		ImGui::EndMenuBar();

		m_isFocused = ImGui::IsWindowFocused() || ImGui::IsWindowHovered();

		// render outputTexture with constant aspect ratio
		ImVec2 imAvailSize = ImGui::GetContentRegionAvail();
		glm::vec2 availSize(imAvailSize.x, imAvailSize.y);

		if (availSize != m_displayedSize) {
			m_camera.setAspectRatio(availSize.x / availSize.y);
		}

		sa::Texture texture = m_pEngine->getRenderPipeline().getRenderTechnique()->getOutputTexture();
		ImGui::Image(texture, imAvailSize);
		ImVec2 imageMin = ImGui::GetItemRectMin();
		ImVec2 imageSize = ImGui::GetItemRectSize();

		m_displayedSize = availSize;


		if (!ImGui::IsMouseDown(ImGuiMouseButton_Right) && m_isFocused) {
			if (ImGui::IsKeyPressed(ImGuiKey_W))
				operation = ImGuizmo::OPERATION::TRANSLATE;
			else if (ImGui::IsKeyPressed(ImGuiKey_E))
				operation = ImGuizmo::OPERATION::ROTATE;
			else if (ImGui::IsKeyPressed(ImGuiKey_R))
				operation = ImGuizmo::OPERATION::SCALE;
			else if (ImGui::IsKeyPressed(ImGuiKey_Q))
				operation = (ImGuizmo::OPERATION)0;
		}

		// Gizmos

		float snap = snapDistance;
		bool doSnap = ImGui::IsKeyDown(ImGuiKey_LeftCtrl);
		if (doSnap && operation == ImGuizmo::OPERATION::ROTATE) {
			snap = snapAngle;
		}

		ImGuizmo::SetOrthographic(false);
		ImGuizmo::AllowAxisFlip(false);
		ImGuizmo::SetDrawlist();
		ImGuizmo::SetRect(imageMin.x, imageMin.y, imageSize.x, imageSize.y);

		glm::vec2 screenPos = { imageMin.x, imageMin.y };
		glm::vec2 screenSize = { imageSize.x, imageSize.y };


		if (m_selectedEntity) {
			sa::Matrix4x4 projMat = m_camera.getProjectionMatrix();
			projMat[1][1] *= -1;
			glm::mat4 viewMat = m_camera.getViewMatrix();


			if (operation) {
				comp::Transform* transform = m_selectedEntity.getComponent<comp::Transform>();

				if (transform) {

					sa::Matrix4x4 transformMat = transform->getMatrix();
					float snapAxis[] = { snap, snap, snap };
					if (ImGuizmo::Manipulate(&viewMat[0][0], &projMat[0][0],
						operation, (ImGuizmo::MODE)m_isWorldCoordinates, &transformMat[0][0],
						nullptr, (doSnap) ? snapAxis : nullptr))
					{
						glm::vec3 rotation;
						glm::vec3 oldPosition = transform->position;
						ImGuizmo::DecomposeMatrixToComponents(&transformMat[0][0], (float*)&transform->position, (float*)&rotation, (float*)&transform->scale);
						transform->rotation = glm::quat(glm::radians(rotation));
						if (transform->hasParent) {
							transform->relativePosition += transform->position - oldPosition;
						}
					}
				}
			}

			comp::Light* light = m_selectedEntity.getComponent<comp::Light>();
			if (light) {
				
				const ImColor lightSphereColor = ImColor(255, 255, 0);
				static bool dragFirstCircle = false;
				static bool dragSecondCircle = false;

				ImGui::GizmoCircleResizable(light->values.position, light->values.attenuationRadius, glm::quat(glm::vec3(0, 0, 0)), &m_camera, screenPos, screenSize, lightSphereColor, dragFirstCircle, 64);
				ImGui::GizmoCircle(light->values.position, light->values.attenuationRadius, glm::quat(glm::vec3(glm::radians(90.f), 0, 0)), &m_camera, screenPos, screenSize, lightSphereColor, 64);
				ImGui::GizmoCircleResizable(light->values.position, light->values.attenuationRadius, glm::quat(glm::vec3(0, glm::radians(90.f), 0)), &m_camera, screenPos, screenSize, lightSphereColor, dragSecondCircle, 64);

			}

			comp::BoxCollider* bc = m_selectedEntity.getComponent<comp::BoxCollider>();
			if (bc) {
				comp::Transform* transform = m_selectedEntity.getComponent<comp::Transform>();
				if (transform) {	
					if (ImGui::GizmoBoxResizable(transform->position + bc->offset, bc->scale, transform->rotation, &m_camera, screenPos, screenSize, ImColor(0, 255, 0))) {
						bc->onUpdate(&m_selectedEntity);
					}
				}
			}
			comp::SphereCollider* sc = m_selectedEntity.getComponent<comp::SphereCollider>();
			if (sc) {
				comp::Transform* transform = m_selectedEntity.getComponent<comp::Transform>();
				if (transform) {
					const ImColor colliderColor = ImColor(0, 255, 0);
					static bool dragFirstCircle1 = false;
					static bool dragSecondCircle2 = false;

					bool released = ImGui::GizmoCircleResizable(transform->position, sc->radius, transform->rotation, &m_camera, screenPos, screenSize, colliderColor, dragFirstCircle1);
					ImGui::GizmoCircle(transform->position, sc->radius, transform->rotation * glm::quat(glm::vec3(glm::radians(90.f), 0, 0)), &m_camera, screenPos, screenSize, colliderColor);
					released = released || ImGui::GizmoCircleResizable(transform->position, sc->radius, transform->rotation * glm::quat(glm::vec3(0, glm::radians(90.f), 0)), &m_camera, screenPos, screenSize, colliderColor, dragSecondCircle2);
					if (released) {
						sc->onUpdate(&m_selectedEntity);
					}
				}
			}



		}
		
		if (showIcons) {
			sa::Texture2D* tex = sa::AssetManager::get().loadTexture("resources/lightbulb-icon.png", true);
			m_pEngine->getCurrentScene()->forEach<comp::Light>([&](const comp::Light& light) {
				ImGui::GizmoIcon(tex, light.values.position, &m_camera, screenPos, screenSize, iconSize);
			});
		}

		

		/*
		ImVec2 viewManipSize = ImVec2(100, 100);
		ImVec2 viewManipPos = ImVec2(imageMin.y, imageMin.y + imageSize.y - ImGui::GetWindowWidth() - viewManipSize.x);
		static glm::mat4 matrix = m_camera.getViewMatrix();
		static int interpolationFrames = 0;
		static bool isDragging = false;
		ImGuizmo::ViewManipulate((float*)&matrix, 5,
			imageMin, viewManipSize, ImColor(0, 0, 50, 150));
		if (interpolationFrames)
			interpolationFrames--;

		if (ImGui::IsMouseHoveringRect(imageMin, { imageMin.x + viewManipSize.x, imageMin.y + viewManipSize.y })
			&& ImGui::IsMouseClicked(ImGuiMouseButton_Left) || ImGui::IsMouseDragging(ImGuiMouseButton_Left) || interpolationFrames || isDragging)
		{	
			if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
				interpolationFrames = 40;
			}
			isDragging = ImGui::IsMouseDragging(ImGuiMouseButton_Left);

			glm::vec3 translation;
			glm::vec3 rotation;
			glm::vec3 scale;
			ImGuizmo::DecomposeMatrixToComponents((float*)&matrix, (float*)&translation, (float*)&rotation, (float*)&scale);
			glm::vec3 forward = glm::vec3(0, 0, 1) * glm::quat(glm::radians(rotation));
			m_camera.setPosition(translation);
			m_camera.lookAt(translation + forward);

		}
		ImGui::SetCursorPosY(150);
		ImGui::Text("%f %f %f %f\n%f %f %f %f\n%f %f %f %f\n%f %f %f %f",
			matrix[0][0], matrix[1][0], matrix[2][0], matrix[3][0],
			matrix[0][1], matrix[1][1], matrix[2][1], matrix[3][1],
			matrix[0][2], matrix[1][2], matrix[2][2], matrix[3][2],
			matrix[0][3], matrix[1][3], matrix[2][3], matrix[3][3]);

		*/
		
	}
	ImGui::End();

}

void SceneView::imGuiDrawLine(glm::vec3 p1, glm::vec3 p2, const ImColor& color, float thickness) {
	ImVec2 contentMin = ImGui::GetWindowContentRegionMin();
	ImVec2 windowPos = ImGui::GetWindowPos();
	contentMin.x += windowPos.x;
	contentMin.y += windowPos.y;

	glm::vec3 forward = m_camera.getForward();

	glm::vec3 cameraToPos1 = p1 - m_camera.getPosition();
	glm::vec3 cameraToPos2 = p2 - m_camera.getPosition();

	if (glm::dot(cameraToPos1, forward) > 0 && glm::dot(cameraToPos2, forward) > 0) {

		p1 = sa::math::worldToScreen(p1, &m_camera, 
			{ contentMin.x, contentMin.y },
			m_displayedSize);
		p2 = sa::math::worldToScreen(p2, &m_camera, 
			{ contentMin.x, contentMin.y }, 
			m_displayedSize);

		ImGui::GetWindowDrawList()->AddLine({ p1.x, p1.y }, { p2.x, p2.y }, color, thickness);

	}
}

bool SceneView::imGuiDrawVector(glm::vec3 origin, glm::vec3 v, const ImColor& color, float thickness) {
	ImVec2 contentMin = ImGui::GetWindowContentRegionMin();
	ImVec2 windowPos = ImGui::GetWindowPos();
	contentMin.x += windowPos.x;
	contentMin.y += windowPos.y;


	glm::vec3 cameraToPos = v - m_camera.getPosition();

	glm::vec3 forward = m_camera.getForward();

	if (glm::dot(cameraToPos, forward) > 0) {
		glm::vec3 o = sa::math::worldToScreen(origin, &m_camera,
			{ contentMin.x, contentMin.y },
			{ m_displayedSize.x, m_displayedSize.y });

		glm::vec3 end = sa::math::worldToScreen(origin + v, &m_camera,
			{ contentMin.x, contentMin.y },
			{ m_displayedSize.x, m_displayedSize.y });

		ImGui::GetWindowDrawList()->AddLine({ o.x, o.y }, { end.x, end.y }, color, thickness);

		glm::vec3 dir = v;
		dir = glm::normalize(dir);
		glm::vec3 normal = forward;
		normal = glm::normalize(normal);
		glm::vec3 tangent = glm::cross(dir, normal);
		tangent = glm::normalize(tangent);

		float size = 0.02f * thickness;
		glm::vec3 p2 = origin + v;
		p2 -= dir * size * 4.f;
		glm::vec3 p3 = p2;

		p2 += tangent * size;
		p3 -= tangent * size;
		
		p2 = sa::math::worldToScreen(p2, &m_camera,
			{ contentMin.x, contentMin.y },
			{ m_displayedSize.x, m_displayedSize.y });
		p3 = sa::math::worldToScreen(p3, &m_camera,
			{ contentMin.x, contentMin.y },
			{ m_displayedSize.x, m_displayedSize.y });

		ImGui::GetWindowDrawList()->AddTriangleFilled({ end.x, end.y }, { p2.x, p2.y }, { p3.x, p3.y }, color);

		glm::vec3 min = glm::min(glm::min(glm::min(end, p2), p3), o);
		glm::vec3 max = glm::max(glm::max(glm::max(end, p2), p3), o);

		ImVec2 arrowRectMin = { min.x, min.y };
		ImVec2 arrowRectMax = { max.x, max.y };

		if (ImGui::IsMouseHoveringRect(arrowRectMin, arrowRectMax)) {
			return true;
		}
	}

	return false;
}

bool SceneView::imGuiDrawVector(glm::vec3 v, const ImColor& color, float thickness) {
	return imGuiDrawVector({ 0, 0, 0 }, v, color, thickness);
}


sa::Camera* SceneView::getCamera() {
	return &m_camera;
}

sa::Entity SceneView::getEntity() const {
	return m_selectedEntity;
}

void SceneView::setEntity(sa::Entity entity) {
	m_selectedEntity = entity;
}
