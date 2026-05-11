#pragma once

#define PTE_PRESENT  (1 << 0)
#define PTE_WRITABLE (1 << 1)
#define PTE_USER     (1 << 2)
#define PTE_NOCACHE  (1 << 4)
#define PTE_SIZE 	 (1 << 7)

#define KERNEL_VMA_BASE 0xffffffff80000000ULL
#define PHYS_TO_VIRT(phys) ((VOID*)((UINT64)(phys) + PHYS_BASE))
#define VIRT_TO_PHYS(virt) ((UINT64)(virt) - PHYS_BASE)

typedef struct {
	UINT64 value;
}PageTableEntry;

typedef struct {
	PageTableEntry entry[512];
} PageTable;