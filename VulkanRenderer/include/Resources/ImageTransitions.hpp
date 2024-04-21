#pragma once

namespace sa {
	enum class Transition {
		NONE,
		RENDER_PROGRAM_INPUT,
		RENDER_PROGRAM_OUTPUT,
		RENDER_PROGRAM_DEPTH_OUTPUT,
		COMPUTE_SHADER_READ,
		COMPUTE_SHADER_WRITE,
		COMPUTE_SHADER_READ_WRITE,
		FRAGMENT_SHADER_READ,
		FRAGMENT_SHADER_WRITE,
		FRAGMENT_SHADER_READ_WRITE,
	};

	inline constexpr const char* to_string(Transition transition) {
		switch(transition) {
		case Transition::NONE:
			return "NONE";
		case Transition::RENDER_PROGRAM_INPUT:
			return "RENDER_PROGRAM_INPUT";
		case Transition::RENDER_PROGRAM_OUTPUT:
			return "RENDER_PROGRAM_OUTPUT";
		case Transition::RENDER_PROGRAM_DEPTH_OUTPUT:
			return "RENDER_PROGRAM_DEPTH_OUTPUT";
		case Transition::COMPUTE_SHADER_READ:
			return "COMPUTE_SHADER_READ";
		case Transition::COMPUTE_SHADER_WRITE:
			return "COMPUTE_SHADER_WRITE";
		case Transition::COMPUTE_SHADER_READ_WRITE:
			return "COMPUTE_SHADER_READ_WRITE";
		case Transition::FRAGMENT_SHADER_READ:
			return "FRAGMENT_SHADER_READ";
		case Transition::FRAGMENT_SHADER_WRITE:
			return "FRAGMENT_SHADER_WRITE";
		case Transition::FRAGMENT_SHADER_READ_WRITE:
			return "FRAGMENT_SHADER_READ_WRITE";
		default:
			return "";
		}
	}
}