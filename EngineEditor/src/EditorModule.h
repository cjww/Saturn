#pragma once
#include <Engine.h>
#include "Events.h"

#include <crtdbg.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#include <stdio.h>          // printf, fprintf
#include <stdlib.h>         // abort
#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#include <Engine.h>
#include <Graphics/RenderWindow.h>
#include <Tools\Clock.h>

#include <SPIRV\spirv_cross.hpp>

#include <Graphics\Vulkan\Renderer.hpp>


class EditorModule {
protected:
	sa::Engine* m_pEngine;
public:
	EditorModule(sa::Engine* pEngine);
	virtual ~EditorModule();
	virtual void onImGui() = 0;
	virtual void update(float dt) = 0;
};