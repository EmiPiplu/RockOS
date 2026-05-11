ENTRY(kernel_main)

KERNEL_VMA = 0xffffffff80000000;
KERNEL_LMA = 0x100000;

SECTIONS
{
    . = KERNEL_VMA;

    .text : AT(ADDR(.text) - KERNEL_VMA + KERNEL_LMA)
    {
        *(.text .text.*)
    }

    . = ALIGN(4096);

    .rodata : AT(ADDR(.rodata) - KERNEL_VMA + KERNEL_LMA)
    {
        *(.rodata .rodata.*)
    }

    . = ALIGN(4096);

    .data : AT(ADDR(.data) - KERNEL_VMA + KERNEL_LMA)
    {
        *(.data .data.*)
    }

    . = ALIGN(4096);

    .bss : AT(ADDR(.bss) - KERNEL_VMA + KERNEL_LMA)
    {
        *(COMMON)
        *(.bss .bss.*)
    }
}