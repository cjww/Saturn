#pragma once

#include <stdint.h>

constexpr uint16_t SA_MAKE_VERSION(uint8_t major, uint8_t minor, uint8_t patch) {
	return (major << 8) | (minor << 4) | patch;
};

#define SA_VERSION_MAJOR 1
#define SA_VERSION_MINOR 0
#define SA_VERSION_PATCH 0
#define SA_VERSION_STR "1.0.0"

#define SA_VERSION SA_MAKE_VERSION(SA_VERSION_MAJOR, SA_VERSION_MINOR, SA_VERSION_PATCH)

#define SA_ASSET_VERSION SA_MAKE_VERSION(1, 0, 0)
#define SA_ASSET_EXTENSION ".asset"
#define SA_ASSET_PACKAGE_EXTENSION ".assetpkg"


#include "Tools/Profiler.h"
#include <Tools/Logger.hpp>

