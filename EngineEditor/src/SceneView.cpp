#include "SceneView.h"

#include "Graphics\RenderTechniques\ForwardPlus.h"

#include <limits>

#include "EngineEditor.h"

SceneView::SceneView(sa::Engine* pEngine, sa::EngineEditor* pEditor, sa::RenderWindow* pWindow)
	: EditorModule(pEngine, pEditor, "Scene View", false)
{
	m_pWindow = pWindow;
	m_isFocused = false;
	m_isWorldCoordinates = false;
	m_selectedEntity = {};

	m_camera.setPosition(sa::Vector3(0, 0, 5));
	m_camera.lookAt(sa::Vector3(0, 0, 0));
	m_camera.setOrthoWidth(10.f);

	m_mouseSensitivity = 30.0f;

	m_velocity = glm::vec3(0.0f);
	m_maxVelocityMagnitude = 30.0f;
	m_acceleration = 0.5f;

	m_statsUpdateTime = 0.1f;
	m_statsTimer = m_statsUpdateTime;

	m_renderTarget.initialize(pEngine, pWindow->getCurrentExtent());

	pEngine->on<sa::engine_event::SceneSet>([&](const sa::engine_event::SceneSet& sceneSetEvent, sa::Engine& engine) {
		m_selectedEntity = {};
	});

	pEngine->on<sa::editor_event::EntitySelected>([&](const sa::editor_event::EntitySelected& e, sa::Engine&) {
		m_selectedEntity = e.entity;
	});

	pEngine->on<sa::editor_event::EntityDeselected>([&](const sa::editor_event::EntityDeselected&, sa::Engine&) {
		m_selectedEntity = {};
	});
	
	m_focusPointDistance = 10.f;
	m_zoom = 0.f;
	m_pWindow->addScrollCallback([&](double x, double y) {
		if (m_isFocused) {
			m_zoom = y;
		}
	});
	
	m_camera.setViewport(sa::Rect{ { 0, 0 }, m_renderTarget.extent });

	pEngine->on<sa::engine_event::OnRender>([&](sa::engine_event::OnRender& e, sa::Engine& engine) {
		if(m_isOpen)
			e.pRenderPipeline->render(&m_camera, &m_renderTarget);
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

		m_statistics.totalGPUMemoryUsage = 0;
		m_statistics.totalGPUMemoryBudget = 0;
		for (auto& heap : m_statistics.gpuMemoryStats.heaps) {
			m_statistics.totalGPUMemoryUsage += heap.usage / 1000000;
			m_statistics.totalGPUMemoryBudget += heap.budget / 1000000;
		}

		std::copy(m_gpuMemoryData.begin() + 1, m_gpuMemoryData.end(), m_gpuMemoryData.begin());
		m_gpuMemoryData[m_gpuMemoryData.size() - 1] = m_statistics.totalGPUMemoryUsage;
	}

	if (!m_isFocused) {
		m_lastMousePos = m_pWindow->getCursorPosition();
		return;
	}

	sa::Vector2 mousePos = m_pWindow->getCursorPosition();
	if (m_pWindow->getMouseButton(sa::MouseButton::RIGHT)) {
		// First-person controls
		m_camera.setProjectionMode(sa::ePerspective);

		sa::Vector2 delta = m_lastMousePos - mousePos;
		m_camera.rotate(glm::radians(delta.x) * dt * m_mouseSensitivity * 2.f, { 0, 1, 0 });
		m_camera.rotate(glm::radians(delta.y) * dt * m_mouseSensitivity * 2.f, m_camera.getRight());
		
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
	else if (m_pWindow->getMouseButton(sa::MouseButton::MIDDLE)) {
		glm::vec2 delta = m_lastMousePos - mousePos;
		glm::vec3 camPos = m_camera.getPosition();
		glm::vec3 right = glm::normalize(glm::cross(m_camera.getUp(), m_camera.getForward()));
		glm::vec3 up = glm::normalize(glm::cross(m_camera.getForward(), right));
		camPos += right * delta.x * dt * m_mouseSensitivity;
		camPos += up * delta.y * dt * m_mouseSensitivity;
		m_camera.setPosition(camPos);
	}
	else {
		m_velocity = glm::vec3(0);
	}

	if (m_zoom) {
		m_camera.setPosition(m_camera.getPosition() + m_camera.getForward() * (m_zoom * 5));
		m_camera.setOrthoWidth(m_camera.getOrthoWidth() - m_zoom * 5);
		m_focusPointDistance -= m_zoom * 5;
		m_zoom = 0;
	}

	m_lastMousePos = mousePos;
		

}

void SceneView::onImGui() {
	SA_PROFILE_FUNCTION();

	bool isOpen = ImGui::Begin(m_name, 0, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove);
	if (isOpen) {

		static ImGuizmo::OPERATION operation = ImGuizmo::TRANSLATE;
		static float snapDistance = 0.5f;
		static float snapAngle = 45.0f;
		static int iconSize = 700;
		static bool showIcons = true;

		static bool showStats = false;
			
		if (ImGui::BeginMenuBar()) {
			

			if (ImGui::BeginMenu("View Settings")) {
				if (ImGui::Checkbox("Statistics", &showStats)) {
					ImGui::CloseCurrentPopup();
				}

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
				ImGui::SliderInt("Icon Size", &iconSize, 1, 5000);
				if (!showIcons) {
					ImGui::EndDisabled();
				}

				ImGui::EndMenu();
			}

			ImGui::RadioButton("T", (int*)&operation, ImGuizmo::OPERATION::TRANSLATE);
			ImGui::RadioButton("R", (int*)&operation, ImGuizmo::OPERATION::ROTATE);
			ImGui::RadioButton("S", (int*)&operation, ImGuizmo::OPERATION::SCALE);
			
		}
		ImGui::EndMenuBar();

		bool windowHovered = ImGui::IsWindowHovered();
		m_isFocused = windowHovered;

		// render outputTexture with variable aspect ratio
		ImVec2 imAvailSize = ImGui::GetContentRegionAvail();
		glm::vec2 availSize(imAvailSize.x, imAvailSize.y);
		
		if (availSize != m_displayedSize) {
			m_camera.setAspectRatio(availSize.x / availSize.y);
		}

		if (availSize != m_displayedSize && !ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
			m_camera.setViewport({ { 0, 0 }, { (uint32_t)availSize.x, (uint32_t)availSize.y } });
			m_renderTarget.resize({ (uint32_t)availSize.x, (uint32_t)availSize.y });
			m_displayedSize = availSize;
		}
		else if (m_renderTarget.outputTexture && m_renderTarget.mainRenderData.isInitialized && m_renderTarget.outputTexture->isValid()) {
			ImGui::Image((sa::Texture)*m_renderTarget.outputTexture, imAvailSize);
		}
		ImVec2 imageMin = ImGui::GetItemRectMin();
		ImVec2 imageSize = ImGui::GetItemRectSize();



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

		ImGuizmo::SetOrthographic(m_camera.getProjectionMode() == sa::eOrthographic);
		ImGuizmo::AllowAxisFlip(false);
		ImGuizmo::SetDrawlist();
		ImGuizmo::SetRect(imageMin.x, imageMin.y, imageSize.x, imageSize.y);


		glm::mat4 projMat = m_camera.getProjectionMatrix();
		projMat[1][1] *= -1;
		glm::mat4 viewMat = m_camera.getViewMatrix();
		
		glm::vec2 screenPos = { imageMin.x, imageMin.y };
		glm::vec2 screenSize = { imageSize.x, imageSize.y };

		if (showIcons) {
			sa::Texture2D* tex = sa::AssetManager::get().loadTexture(m_pEditor->editorRelativePath("resources/lightbulb-icon.png"), true);
			sa::Texture2D* sunTexture = sa::AssetManager::get().loadTexture(m_pEditor->editorRelativePath("resources/sun-icon.png"), true);
			m_pEngine->getCurrentScene()->forEach<comp::Light>([&](const comp::Light& light) {
				ImColor color(light.values.color);
				switch (light.values.type) {
				case sa::LightType::POINT:
					ImGui::GizmoIcon(tex, light.values.position, &m_camera, screenPos, screenSize, iconSize, color);
					break;
				case sa::LightType::DIRECTIONAL:
					ImGui::GizmoIcon(sunTexture, light.values.position, &m_camera, screenPos, screenSize, iconSize * 1.5f, color);
					break;
				case sa::LightType::SPOT:

					break;
				default:
					break;
				}
				

			});
			
			tex = sa::AssetManager::get().loadTexture(m_pEditor->editorRelativePath("resources/camera-transparent.png"), true);
			m_pEngine->getCurrentScene()->forEach<comp::Camera>([&](const comp::Camera& camera) {
				ImGui::GizmoIcon(tex, camera.camera.getPosition(), &m_camera, screenPos, screenSize, iconSize, ImColor(1.f, 1.f, 1.f, 1.f));
			});
		}

		if (m_selectedEntity) {

			bool isOperating = false;

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
					if (ImGuizmo::IsOver() && ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
						isOperating = true;
					}

					if (doSnap) {
						glm::mat4 gridMat(1);
						ImGuizmo::DrawGrid((float*)&viewMat, (float*)&projMat, (float*)&gridMat, 10.f);
					}
				}
			}




			const ImColor colliderColor = ImColor(0, 255, 0);
			const ImColor lightSphereColor = ImColor(255, 255, 0);
			// Light icons
			comp::Light* light = m_selectedEntity.getComponent<comp::Light>();
			if (light) {
				switch (light->values.type) {
				case sa::LightType::POINT:
					ImGui::GizmoSphereResizable(light->values.position, light->values.position.w, glm::quat(1, 0, 0, 0), &m_camera, screenPos, screenSize, lightSphereColor, isOperating);
					break;
				case sa::LightType::DIRECTIONAL:
					
					break;
				case sa::LightType::SPOT:
					
					break;
				default:
					break;
				}
			}

			// Box colliders
			comp::BoxCollider* bc = m_selectedEntity.getComponent<comp::BoxCollider>();
			if (bc) {
				comp::Transform* transform = m_selectedEntity.getComponent<comp::Transform>();
				if (transform) {
					glm::vec3 offset = transform->rotation * bc->offset;
					if (ImGui::GizmoBoxResizable(transform->position + offset, bc->halfLengths, transform->rotation, &m_camera, screenPos, screenSize, colliderColor, isOperating)) {
						bc->onUpdate(&m_selectedEntity);
					}
				}
			}
			// SphereColliders
			comp::SphereCollider* sc = m_selectedEntity.getComponent<comp::SphereCollider>();
			if (sc) {
				comp::Transform* transform = m_selectedEntity.getComponent<comp::Transform>();
				if (transform) {
					if (ImGui::GizmoSphereResizable(transform->position + sc->offset, sc->radius, transform->rotation, &m_camera, screenPos, screenSize, colliderColor, isOperating)) {
						sc->onUpdate(&m_selectedEntity);
					}
				}
			}

		}
		
		ImVec2 viewManipSize = ImVec2(100, 100);
		ImVec2 viewManipPos = ImVec2(imageMin.x + (imageSize.x - viewManipSize.x), imageMin.y);
		static int interpolationFrames = 0;
		static bool isDragging = false;

		int panelIndex = -1;
		ImGuizmo::ViewManipulate((float*)&viewMat, &panelIndex, m_focusPointDistance,
			viewManipPos, viewManipSize, ImColor(0, 0, 0, 0));

		/*
		ImGuizmo::ViewManipulate((float*)&viewMat, (float*)&projMat, ImGuizmo::OPERATION::TRANSLATE | ImGuizmo::OPERATION::ROTATE | ImGuizmo::OPERATION::SCALE, ImGuizmo::WORLD,
			(float*)&viewMat, m_focusPointDistance, imageMin, viewManipSize, ImColor(0, 0, 0, 0));
		*/

		bool isMouseClicked = ImGui::IsMouseClicked(ImGuiMouseButton_Left);
		bool isMouseDragging = ImGui::IsMouseDragging(ImGuiMouseButton_Left);


		if ((ImGui::IsMouseHoveringRect(viewManipPos, { viewManipPos.x + viewManipSize.x, viewManipPos.y + viewManipSize.y })
			&& (isMouseClicked || isMouseDragging)) || isDragging || interpolationFrames)
		{
			if (isMouseClicked) {
				interpolationFrames = 100;
			}
			isDragging = isMouseDragging;

			glm::mat4 inverseMatrix = glm::inverse(viewMat);
			glm::vec3 translation = glm::vec3(inverseMatrix[3]);
			glm::vec3 forward = -glm::vec3(inverseMatrix[2]);

			m_camera.setPosition(translation);
			m_camera.lookTo(forward);

			ImGui::GizmoCircleFilled2D(m_camera.getPosition() + m_camera.getForward() * m_focusPointDistance, 0.05f, &m_camera, { imageMin.x, imageMin.y }, { imageSize.x, imageSize.y }, ImColor(1.0f, 1.0f, 0.0f, 1.0f));

		}
		if (interpolationFrames)
			interpolationFrames--;

		if (panelIndex == 4) {
			m_camera.setProjectionMode(sa::eOrthographic);
		}
		else if (panelIndex != -1) {
			m_camera.setProjectionMode(sa::ePerspective);
		}

		
		ImVec2 pos = { 
			ImGui::GetCursorStartPos().x + imageSize.x - viewManipSize.x * 0.5f,
			ImGui::GetCursorStartPos().y + viewManipSize.y + ImGui::GetStyle().ItemSpacing.y
		};
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(40, 15));
		ImGui::SetCursorPos(pos);
		if (ImGui::TextButton((m_camera.getProjectionMode() == sa::ePerspective) ? "Perspective" : "Orthographic", true)) {
			m_camera.setProjectionMode(sa::ProjectionMode(((int)m_camera.getProjectionMode() + 1) % 2));
		}

		ImGui::SetCursorPosX(pos.x);
		if (ImGui::TextButton((m_isWorldCoordinates) ? "World Space" : "Object Space", true)) {
			m_isWorldCoordinates = !m_isWorldCoordinates;
		}
		ImGui::PopStyleVar();


		/*
		ImGui::SetCursorPosY(150);
		ImGui::Text("\t%f\t%f\t%f\t%f\n\t%f\t%f\t%f\t%f\n\t%f\t%f\t%f\t%f\n\t%f\t%f\t%f\t%f",
			viewMat[0][0], viewMat[1][0], viewMat[2][0], viewMat[3][0],
			viewMat[0][1], viewMat[1][1], viewMat[2][1], viewMat[3][1],
			viewMat[0][2], viewMat[1][2], viewMat[2][2], viewMat[3][2],
			viewMat[0][3], viewMat[1][3], viewMat[2][3], viewMat[3][3]);
		*/
		
		if (showStats) {
			ImGui::SetCursorPosY(ImGui::GetCursorStartPos().y);
			ImGui::Indent(ImGui::GetWindowWidth() - 400);
			ImGui::Text("Entity Count: %llu", m_pEngine->getCurrentScene()->getEntityCount());

			ImGui::Text("FPS: %f", 1 / m_statistics.frameTime);
			ImVec4 bgColor = ImGui::GetStyleColorVec4(ImGuiCol_FrameBg);
			bgColor.w = 0.5f;

			ImGui::PushStyleColor(ImGuiCol_FrameBg, bgColor);

			bgColor = ImGui::GetStyleColorVec4(ImGuiCol_Header);
			bgColor.w = 0.5f;
			ImGui::PushStyleColor(ImGuiCol_Header, bgColor);

			bgColor = ImGui::GetStyleColorVec4(ImGuiCol_HeaderHovered);
			bgColor.w = 0.5f;
			ImGui::PushStyleColor(ImGuiCol_HeaderHovered, bgColor);

			bgColor = ImGui::GetStyleColorVec4(ImGuiCol_HeaderActive);
			bgColor.w = 0.5f;
			ImGui::PushStyleColor(ImGuiCol_HeaderActive, bgColor);


			if (ImGui::CollapsingHeader("Performance")) {
				ImGui::PushStyleColor(ImGuiCol_PlotLines, ImVec4(1.f, 1.f, 1.f, 1.f));
				
				static float scale = 70.f;
				ImGui::PlotLines("##FrameTimePlot", m_frameTimeGraph.data(), m_frameTimeGraph.size(), 0, "Frame Time", 0.f, scale, ImVec2(300, 50));
				ImGui::SameLine();
				ImGui::VSliderFloat("Scale", ImVec2(15, 50), &scale, 1.f, 100.f, "%.0f");

				ImGui::PopStyleColor();
				ImGui::Text("Frame time: %f ms", m_statistics.frameTime * 1000);

			}
			if (ImGui::CollapsingHeader("Memory")) {
				ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(1.f, 1.f, 1.f, 1.f));
				ImGui::PlotHistogram("##VRAMPlot", m_gpuMemoryData.data(), m_gpuMemoryData.size(), 0, "GPU Memory usage", 0.f, m_statistics.totalGPUMemoryBudget, ImVec2(300, 50));
				ImGui::PopStyleColor();
			
				ImGui::Text("GPU Memory Heaps");
				ImGui::Indent();
				for (auto& heap : m_statistics.gpuMemoryStats.heaps) {
					ImGui::Text("%llu MB / %llu MB, flags: %u", heap.usage / 1000000, heap.budget / 1000000, heap.flags);
				}
				ImGui::Text("Total: %llu MB / %llu MB", m_statistics.totalGPUMemoryUsage, m_statistics.totalGPUMemoryBudget);
				ImGui::Unindent();
			}
			ImGui::PopStyleColor(4);
			ImGui::Unindent();
			
		}

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


sa::SceneCamera* SceneView::getCamera() {
	return &m_camera;
}

sa::Entity SceneView::getEntity() const {
	return m_selectedEntity;
}

void SceneView::setEntity(sa::Entity entity) {
	m_selectedEntity = entity;
}
