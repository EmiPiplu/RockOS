#include <stdint.h>
#include <Drivers/Serial/serial.h>
#include <bootinfo.h>

void halt() {
	for (;;) {
        __asm__ volatile ("hlt");
    }
}

void kernel_main(BootInfo *boot_info) {

	serial_init();

	if (boot_info->magic != BOOTINFO_MAGIC) {
		serial_write_string("BootInfo Incorrect\n");
		halt();
	} 
	
	serial_write_string("Bootinfo Recieved\r\n");

	serial_write_string("Hello Kernel\n");
	serial_write_string("It's Ya girl, live from Serial\n");



    halt();
}