#pragma once

#include <array>

namespace sa {


	enum MemoryPropertyFlagBits {
		DEVICE_LOCAL_BIT = 0x00000001,
		HOST_VISIBLE_BIT = 0x00000002,
		HOST_COHERENT_BIT = 0x00000004,
		HOST_CACHED_BIT = 0x00000008,
		LAZILY_ALLOCATED_BIT = 0x00000010,
		PROTECTED_BIT = 0x00000020,
		DEVICE_COHERENT_BIT_AMD = 0x00000040,
		DEVICE_UNCACHED_BIT_AMD = 0x00000080,
		RDMA_CAPABLE_BIT_NV = 0x00000100,
		MEMORY_PROPERTY_MAX_ENUM_COUNT = 9
	};
	typedef uint32_t MemoryPropertyFlags;

	inline const char* MemoryPropertyName(MemoryPropertyFlagBits memoryProperty) {
		switch (memoryProperty)
		{
		case sa::DEVICE_LOCAL_BIT:
			return "Device local";
		case sa::HOST_VISIBLE_BIT:
			return "Host visible";
		case sa::HOST_COHERENT_BIT:
			return "Host coherent";
		case sa::HOST_CACHED_BIT:
			return "Host cached";
		case sa::LAZILY_ALLOCATED_BIT:
			return "Lazily allocated";
		case sa::PROTECTED_BIT:
			return "Protected";
		case sa::DEVICE_COHERENT_BIT_AMD:
			return "Device coherent AMD";
		case sa::DEVICE_UNCACHED_BIT_AMD:
			return "Device unchached AMD";
		case sa::RDMA_CAPABLE_BIT_NV:
			return "RDMA capable";
		default:
			break;
		}
		return "";
	}

	struct HeapMemoryStats {
		MemoryPropertyFlags propertyFlags;
		// Number of bytes currently used by program
		size_t usage;
		// Number of bytes available to program
		size_t budget;
		// Number of bytes allocated by all resources
		size_t allocationBytes;
		// Number of allocated resources
		size_t allocationCount;
		// Number of bytes allocated in VkDeviceMemory blocks
		size_t blockBytes;
		// Number of Vulkanmemory blocks allocated
		size_t blockCount;

	};

	struct DeviceMemoryStats {
		size_t totalUsage;
		size_t totalBudget;
		std::array<HeapMemoryStats, 32> heaps;
		uint8_t heapCount;
	};
}