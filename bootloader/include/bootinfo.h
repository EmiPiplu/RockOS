#pragma once

#define BOOTINFO_MAGIC 0xB00B135

typedef struct {
	UINT64 magic;

	UINT64 memory_map;
	UINT64 memory_map_size;
	UINT64 memory_map_descriptor_size;
	UINT64 memory_map_descriptor_version;

	UINT64 kernel_stack_bottom;
	UINT64 kernel_stack_top;
} BootInfo;