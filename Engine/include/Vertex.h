#pragma once

#include "Tools/Vector.h"

namespace sa {

	struct VertexUV {
		Vector4 position;
		Vector2 texCoord;
		bool operator==(const VertexUV& other) const {
			return this->position == other.position && this->texCoord == other.texCoord;
		}
	};

	struct VertexNormalUV {
		Vector4 position;
		Vector4 normal;
		Vector2 texCoord;
		bool operator==(const VertexNormalUV& other) const {
			return this->position == other.position && this->normal == other.normal && this->texCoord == other.texCoord;
		}
	};

	struct VertexColor {
		Vector4 position;
		Vector4 color;
	};
}

namespace std {
	template<>
	struct hash<sa::VertexUV> {
		size_t operator()(const sa::VertexUV& v) const {
			return std::hash<sa::Vector4>()(v.position) ^ std::hash<sa::Vector2>()(v.texCoord);
		}
	};

	template<>
	struct hash<sa::VertexNormalUV> {
		size_t operator()(const sa::VertexNormalUV& v) const {
			return std::hash<sa::Vector4>()(v.position) ^ std::hash<sa::Vector4>()(v.normal) ^ std::hash<sa::Vector2>()(v.texCoord);
		}
	};
}
