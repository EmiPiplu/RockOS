#include <Drivers/Serial/serial.h>
#include <IO/io.h>

void serial_init() {
	outb(0x3FB, 0x80); // Set DLAB
	outb(0x3F8, 0x03); // Divisor Low Byte
	outb(0x3F9, 0x00); // Divisor High byte
	outb(0x3FB, 0x03); // Clear DLAB, set 8N1
	outb(0x3FA, 0xC7); // enable FIFO
	outb(0x3FC, 0x0B); // mark port as ready 
}

int serial_can_write(){
	if (!(inb(0x3FD) & 0x20)) return -1;
	else return 0;
}

void serial_write_char(char c) {
	while(serial_can_write() != 0);

    outb(0x3F8, c);
}

void serial_write_string(const char* str) {
    for(int i = 0; str[i] != '\0'; i++) {
        serial_write_char(str[i]);
    }
}