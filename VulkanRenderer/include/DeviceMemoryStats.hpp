#pragma once


struct DeviceMemoryStats {
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
