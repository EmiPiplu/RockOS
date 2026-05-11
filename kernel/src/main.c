#include <stdint.h>
#include <Drivers/Serial/serial.h>


void kernel_main(void) {

	serial_init();

	serial_write_string("Hello Kernel\n");
	serial_write_string("It's Ya girl, live from Serial\n");



    for (;;) {
        __asm__ volatile ("hlt");
    }
}