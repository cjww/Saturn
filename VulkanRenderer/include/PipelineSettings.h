#pragma once
#include <stdint.h>

namespace sa {

	typedef uint16_t CullModeFlags;

	enum Topology {
		POINT_LIST,
		LINE_LIST,
		LINE_STRIP,
		TRIANGLE_LIST,
		TRIANGLE_STRIP,
		TRIANGLE_FAN,
		LINE_LIST_WITH_ADJACENCY,
		LINE_STRIP_WITH_ADJACENCY,
		TRIANGLE_LIST_WITH_ADJACENCY,
		TRIANGLE_STRIP_WITH_ADJACENCY,
		PATCH_LIST
	};

	enum CullModeFlagBits : CullModeFlags {
		NONE,
		FRONT,
		BACK
	};

	enum PolygonMode {
		FILL,
		LINE,
		POINT
	};

	struct PipelineSettings {
		Topology topology = Topology::TRIANGLE_LIST;
		CullModeFlags cullMode = CullModeFlagBits::BACK;
		PolygonMode polygonMode = PolygonMode::FILL;
		bool depthTestEnabled = true;
	};
}