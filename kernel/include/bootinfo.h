#include <stdint.h>

#define BOOTINFO_MAGIC 0xB00B135

typedef struct {
    uint64_t magic;

    uint64_t memory_map;
    uint64_t memory_map_size;
    uint64_t memory_map_descriptor_size;
    uint64_t memory_map_descriptor_version;

    uint64_t kernel_stack_bottom;
    uint64_t kernel_stack_top;
} BootInfo;