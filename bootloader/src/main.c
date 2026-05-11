#include "../include/efi.h"
#include "../include/elf.h"
#include <stdarg.h>

EFI_SIMPLE_TEXT_INPUT_PROTOCOL *cin = NULL;
EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *cout = NULL;
EFI_BOOT_SERVICES *bs;
EFI_HANDLE image_handle = NULL;

EFI_PHYSICAL_ADDRESS stack_top;
EFI_PHYSICAL_ADDRESS stack_bottom;


void init_globals(EFI_HANDLE handle, EFI_SYSTEM_TABLE *systable) {
	cout = systable->ConOut;
	cin = systable->ConIn;
	bs = systable->BootServices;
	image_handle = handle;

	cout->Reset(cout, false);

	cin->Reset(cin, false);

}

//Shamelessly stolen print_int, print_hex and printf, Thanks Queso Fuego, may the cout spam finally die.
bool print_int(INT32 number) {
    const CHAR16 *digits = u"0123456789";
    CHAR16 buffer[11];  
    UINTN i = 0;
    const bool negative = (number < 0);

    if (negative) number = -number;

    do {
       buffer[i++] = digits[number % 10];
       number /= 10;
    } while (number > 0);

    if (negative) buffer[i++] = u'-';

    buffer[i--] = u'\0';

    for (UINTN j = 0; j < i; j++, i--) {
        UINTN temp = buffer[i];
        buffer[i] = buffer[j];
        buffer[j] = temp;
    }

    cout->OutputString(cout, buffer);

    return true;
}


bool print_hex(UINTN number) {
    const CHAR16 *digits = u"0123456789ABCDEF";
    CHAR16 buffer[20];  
    UINTN i = 0;

    do {
       buffer[i++] = digits[number % 16];
       number /= 16;
    } while (number > 0);

   
    buffer[i++] = u'x';
    buffer[i++] = u'0';


    buffer[i--] = u'\0';

    for (UINTN j = 0; j < i; j++, i--) {
        UINTN temp = buffer[i];
        buffer[i] = buffer[j];
        buffer[j] = temp;
    }

    cout->OutputString(cout, buffer);

    return true;
}


bool printf(CHAR16 *fmt, ...) {
    bool result = false;
    CHAR16 charstr[2];    
    va_list args;

    va_start(args, fmt);

    charstr[0] = u'\0', charstr[1] = u'\0';

    for (UINTN i = 0; fmt[i] != u'\0'; i++) {
        if (fmt[i] == u'%') {
            i++;

            switch (fmt[i]) {
                case u's': {
                    CHAR16 *string = va_arg(args, CHAR16*);
                    cout->OutputString(cout, string);
                }
                break;

                case u'd': {
                    INT32 number = va_arg(args, INT32);
                    print_int(number);
                }
                break;

                case u'x': {
                    UINTN number = va_arg(args, UINTN);
                    print_hex(number);
                }
                break;

                default:
                    cout->OutputString(cout, u"Invalid format specifier: %");
                    charstr[0] = fmt[i];
                    cout->OutputString(cout, charstr);
                    cout->OutputString(cout, u"\r\n");
                    result = false;
                    goto end;
                    break;
            }
        } else {
            charstr[0] = fmt[i];
            cout->OutputString(cout, charstr);
        }
    }

end:
    va_end(args);

    result = true;
    return result;
}

void uint_to_string(UINTN value, CHAR16* output_buffer, UINTN cap) {
	UINTN index = 0;
	if (value == 0) {
		output_buffer[0] = u'0';
		output_buffer[1] = u'\0';
		return;
	}
	UINTN temp;
	while (value != 0) {
		temp = value % 10;
		output_buffer[index] = u'0' + temp;
		value /= 10;
		index++;
		if (index > cap - 1) break;
	}

	UINTN l = 0;
	UINTN r = index - 1;
	CHAR16 t;

	while(l < r) {
		t = output_buffer[l];
		output_buffer[l] = output_buffer[r];
		output_buffer[r] = t;

		l++;
		r--;
	}

	output_buffer[index] = u'\0';
}

void uint_to_hex_string(UINTN value, CHAR16* output_buffer, UINTN cap) {
	UINTN index = 0;
	CHAR16* digits = u"0123456789ABCDEF";
	if (value == 0) {
		output_buffer[0] = u'0';
		output_buffer[1] = u'\0';
		return;
	}
	UINTN temp;
	while (value != 0) {
		temp = value % 16;
		output_buffer[index] = digits[temp];
		value /= 16;
		index++;
		if (index > cap - 1) break;
	}

	UINTN l = 0;
	UINTN r = index - 1;
	CHAR16 t;

	while(l < r) {
		t = output_buffer[l];
		output_buffer[l] = output_buffer[r];
		output_buffer[r] = t;

		l++;
		r--;
	}

	output_buffer[index] = u'\0';
}

void byte_to_char16(UINT8* input, CHAR16* output, UINTN len) {
	UINTN index = 0;
	while (index < len) {
		output[index] = (CHAR16)input[index];
		index++;
	}
	output[index] = u'\0';
}

UINTN bytes_to_pages(UINTN size) {
	return (size + 4095) / 4096;
}

UINTN align_down(UINTN x, UINTN a) {
	return x & ~(a - 1);
}

UINTN align_up(UINTN x, UINTN a) {
	return (x + a - 1) & ~(a - 1);
}

EFI_INPUT_KEY get_key(void) {
	EFI_EVENT events[1];
	EFI_INPUT_KEY key;
	EFI_STATUS status;

	key.ScanCode = 0;
	key.UnicodeChar = u'\0';

	events[0] = cin->WaitForKey;
	UINTN index = 0;
	status = bs->WaitForEvent(1, events, &index);
	if (EFI_ERROR(status)) {
		cout->OutputString(cout, u"get_key: Error Waiting for Event\r\n");
		return key;
	}

	if (index == 0){
		status = cin->ReadKeyStroke(cin, &key);
		if (EFI_ERROR(status)) {
			cout->OutputString(cout, u"get_key: Error Reading Keystroke\r\n");
			return key;
		}
		return key;
	}

	return key;
}

EFI_FILE_PROTOCOL* open_file(CHAR16 *filename) {

	EFI_LOADED_IMAGE_PROTOCOL *loaded_image;

	EFI_GUID loaded_image_protocol_guid = {
		0x5B1B31A1,
		0x9562,
		0x11D2,
		0x8E,
		0x3F,
		{0x00, 0xA0, 0xC9, 0x69, 0x72, 0x3B}
	};


	EFI_STATUS status =	bs->HandleProtocol(image_handle, &loaded_image_protocol_guid, (void**)&loaded_image);
	if (EFI_ERROR(status)){
		cout->OutputString(cout, u"Error finding EFI_LOADED_IMAGE_PROTOCOL\r\n");
		while (1);
	}
	
	cout->OutputString(cout, u"Found Loaded Image Protocol\r\n");
	
	EFI_GUID SimpleFileSystemProtocolGuid = {
		0x964E5B22,
		0x6459,
		0x11D2,
		0x8E,
		0x39,
		{0x00,0xA0,0xC9,0x69,0x72,0x3B}
	};

	EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *filesystem;

	status = bs->HandleProtocol(loaded_image->DeviceHandle, &SimpleFileSystemProtocolGuid, (void**)&filesystem);
	if (EFI_ERROR(status)){
		cout->OutputString(cout, u"Error finding EFI_SIMPLE_FILE_SYSTEM_PROTOCOL\r\n");
		while (1);
	}
	
	cout->OutputString(cout, u"Found File System Protocol\r\n");

	EFI_FILE_PROTOCOL *root;

	status = filesystem->OpenVolume(filesystem, &root);
	if (EFI_ERROR(status)){
		cout->OutputString(cout, u"Error opening Volume\r\n");
		while (1);
	}
	
	cout->OutputString(cout, u"Opened Root Volume\r\n");

	EFI_FILE_PROTOCOL *file;

	status = root->Open(root, &file, filename, EFI_FILE_MODE_READ, 0);
		if (EFI_ERROR(status)){
		cout->OutputString(cout, u"Error opening file\r\n");
		while (1);
	}
	
	cout->OutputString(cout, u"Opened file\r\n");

	return file;

}

BOOLEAN validate_elf(Elf64_Ehdr hdr) {

	CHAR16 print_buf[128];	

	if (hdr.e_ehsize != 64){
		cout->OutputString(cout, u"ELF Header size is incorrect, Expected 64, Got ");
		uint_to_string(hdr.e_ehsize, print_buf, 128);
		cout->OutputString(cout, print_buf);
		cout->OutputString(cout, u"\r\n");
		return false;
	}

	if (hdr.e_phentsize != 56){
		cout->OutputString(cout, u"Header size is incorrect, Expected 56, Got ");
		uint_to_string(hdr.e_phentsize, print_buf, 128);
		cout->OutputString(cout, print_buf);
		cout->OutputString(cout, u"\r\n");
		return false;
	}


	if (hdr.e_ident[0] == 0x7F && hdr.e_ident[1] == 'E' && hdr.e_ident[2] == 'L' && hdr.e_ident[3] == 'F') {
		uint_to_hex_string(hdr.e_ident[0], print_buf, 2);
		cout->OutputString(cout, u"ELF Header Verified 0x");
		cout->OutputString(cout, print_buf);
		cout->OutputString(cout, u" ");
		byte_to_char16(&hdr.e_ident[1], print_buf, 3);
		cout->OutputString(cout, print_buf);
		cout->OutputString(cout, u"\r\n");
	} else {
		cout->OutputString(cout, u"ERROR: Invalid ELF Header\r\n");
		return false;
	}

	if (hdr.e_ident[4] == 2) {
		cout->OutputString(cout, u"ELF is 64 Bit\r\n");
	} else {
		cout->OutputString(cout, u"ERROR: ELF is Not 64 Bit\r\n");
		return false;
	}

	if (hdr.e_ident[5] == 1) {
		cout->OutputString(cout, u"ELF is Little Endian\r\n");
	} else {
		cout->OutputString(cout, u"ERROR: ELF is not Little Endian\r\n");
		return false;
	}

	if (hdr.e_ident[6] == 1) {
		cout->OutputString(cout, u"ELF Version Correct\r\n");
	} else {
		cout->OutputString(cout, u"Incorrect ELF Version\r\n");
		return false;
	}

	if (hdr.e_type == 2) {
		cout->OutputString(cout, u"ELF is Executable\r\n");
	} else {
		cout->OutputString(cout, u"ERROR: ELF is Not Executable\r\n");
		return false;
	}

	if (hdr.e_machine == 0x3E) {
		cout->OutputString(cout, u"Arch is x86_64\r\n");
	} else {
		cout->OutputString(cout, u"ERROR Arch is not x86_64\r\n");
		return false;
	}

	if (hdr.e_phoff != 0) {
		cout->OutputString(cout, u"Program Header offset verified\r\n");
	} else {
		cout->OutputString(cout, u"Program Header offset incorrect\r\n");
		return false;
	}

	if (hdr.e_phnum > 0) {
		cout->OutputString(cout, u"Program Headers present: ");
		uint_to_string(hdr.e_phnum, print_buf, 128);
		cout->OutputString(cout, print_buf);
		cout->OutputString(cout, u"\r\n");
	} else {
		cout->OutputString(cout, u"Program Headers not found");
		return false;
	}
	
	if (hdr.e_phnum > 16) {
		cout->OutputString(cout, u"Too Many Program Headers\r\n");
	}

	cout->OutputString(cout, u"Entry Point: 0x");
	uint_to_hex_string(hdr.e_entry, print_buf, 128);
	cout->OutputString(cout, print_buf);
	cout->OutputString(cout, u"\r\n");

 
	return true;
}

void Load_Kernel(EFI_FILE_PROTOCOL* file, Elf64_Ehdr hdr) {

	file->SetPosition(file, hdr.e_phoff);

	Elf64_Phdr phdrs[16];
	UINTN phdr_size = hdr.e_phnum * hdr.e_phentsize;

	file->Read(file, &phdr_size, &phdrs);
		if (phdr_size != hdr.e_phnum * hdr.e_phentsize){
			cout->OutputString(cout, u"Could not Read all phdrs\r\n");
		}

	CHAR16 print_buf[128];

	// Ok i kinda need to fix all of these but oh well it's done and it works.
	for (UINTN i = 0; i < hdr.e_phnum; i++) {
		cout->OutputString(cout, u"Index: ");
		uint_to_string(i, print_buf, 128);
		cout->OutputString(cout, print_buf);
		cout->OutputString(cout, u"\r\n");

		cout->OutputString(cout, u"Type: ");
		uint_to_string(phdrs[i].p_type, print_buf, 128);
		cout->OutputString(cout, print_buf);
		cout->OutputString(cout, u"\r\n");

		cout->OutputString(cout, u"Offset: ");
		uint_to_string(phdrs[i].p_offset, print_buf, 128);
		cout->OutputString(cout, print_buf);
		cout->OutputString(cout, u"\r\n");

		cout->OutputString(cout, u"Virtual Address: ");
		uint_to_hex_string(phdrs[i].p_vaddr, print_buf, 128);
		cout->OutputString(cout, print_buf);
		cout->OutputString(cout, u"\r\n");

		cout->OutputString(cout, u"Physical Address: ");
		uint_to_hex_string(phdrs[i].p_paddr, print_buf, 128);
		cout->OutputString(cout, print_buf);
		cout->OutputString(cout, u"\r\n");

		cout->OutputString(cout, u"File Size: ");
		uint_to_string(phdrs[i].p_filesz, print_buf, 128);
		cout->OutputString(cout, print_buf);
		cout->OutputString(cout, u"\r\n");

		cout->OutputString(cout, u"Memory Size: ");
		uint_to_string(phdrs[i].p_memsz, print_buf, 128);
		cout->OutputString(cout, print_buf);
		cout->OutputString(cout, u"\r\n");

		cout->OutputString(cout, u"Flags: ");
		uint_to_string(phdrs[i].p_flags, print_buf, 128);
		cout->OutputString(cout, print_buf);
		cout->OutputString(cout, u"\r\n");

		cout->OutputString(cout, u"Alignment: ");
		uint_to_string(phdrs[i].p_align, print_buf, 128);
		cout->OutputString(cout, print_buf);
		cout->OutputString(cout, u"\r\n");

	}


	for (UINTN i = 0; i < hdr.e_phnum; i++) {

		if(phdrs[i].p_type != 1) {
			continue;
		}

		EFI_PHYSICAL_ADDRESS address = phdrs[i].p_paddr;
		UINTN msize = phdrs[i].p_memsz;
		UINTN fsize = phdrs[i].p_filesz;
		UINTN expected_fsize = fsize;
		EFI_PHYSICAL_ADDRESS page_base = align_down(address, 4096);
		UINTN page_offset = address - page_base;
		UINTN pages = bytes_to_pages(page_offset + msize);

		if (phdrs[i].p_memsz < phdrs[i].p_filesz) {
			cout->OutputString(cout, u"Invalid ELF segment: memsz < filesz\r\n");
			while (1);
		}
		printf(u"Loading PT_LOAD Segment %d\r\nPhys: %x\r\n memsz: %d\r\n Pages: %d\r\n", i, address, msize, pages);
		EFI_STATUS status = bs->AllocatePages(AllocateAddress, EfiLoaderData, pages, &page_base);
		if (EFI_ERROR(status)){
			cout->OutputString(cout, u"Could not allocate pages \r\n");
			uint_to_hex_string(status, print_buf, 128);
			cout->OutputString(cout, print_buf);
			while(1);
		}
		printf(u"Allocated page\r\n");
		status = file->SetPosition(file, phdrs[i].p_offset);
		if (EFI_ERROR(status)){
			cout->OutputString(cout, u"Could not Set Position\r\n");
			while(1);
		}
		status = file->Read(file, &fsize, (VOID*)address);
		if (EFI_ERROR(status)){
			cout->OutputString(cout, u"Could not read file\r\n");
			while(1);
		}
		if (fsize != expected_fsize){
			printf(u"Read Error: expected %d bytes, got %d\r\n", expected_fsize, fsize);
			while(1);
		}

		if (phdrs[i].p_memsz > phdrs[i].p_filesz) {
			UINT8 *bss_start = (UINT8 *)(address + phdrs[i].p_filesz);
			UINTN bss_size = phdrs[i].p_memsz - phdrs[i].p_filesz;

			for (UINTN j = 0; j < bss_size; j++) {
				bss_start[j] = 0;
			}
		}

	}

	printf(u"Kernel Loaded\r\n");

}

void Read_Memory_Map() {

	UINTN MemoryMapSize = 0;
	EFI_MEMORY_DESCRIPTOR *MemoryMap = NULL;
	UINTN MapKey;
	UINTN DescriptorSize;
	UINT32 DescriptorVersion;

	EFI_STATUS status = bs->GetMemoryMap(&MemoryMapSize, NULL, &MapKey, &DescriptorSize, &DescriptorVersion);
	if (status != EFI_BUFFER_TOO_SMALL) {
		cout->OutputString(cout, u"Something went wrong getting Memory Map size\r\n");
		while(1);
	}
	
	MemoryMapSize += 4096;

	status = bs->AllocatePool(EfiLoaderData, MemoryMapSize, (VOID**)&MemoryMap);
	if (EFI_ERROR(status)) {
		cout->OutputString(cout, u"Could not allocate pool\r\n");
		while(1);
	}
	status = bs->GetMemoryMap(&MemoryMapSize, MemoryMap, &MapKey, &DescriptorSize, &DescriptorVersion);
	if (EFI_ERROR(status)) {
		cout->OutputString(cout, u"Could not Read memory map\r\n");
		while(1);
	}

	UINTN DescriptorCount = MemoryMapSize / DescriptorSize; 

	UINT8* Base = (UINT8*)MemoryMap;

	for (UINTN i = 0; i < DescriptorCount; i++ ) {
		EFI_MEMORY_DESCRIPTOR* desc = (EFI_MEMORY_DESCRIPTOR*)(Base + (i * DescriptorSize));

		printf(u"Type: %d, Phys: %x, Pages: %d\r\n", desc->Type, desc->PhysicalStart, desc->NumberOfPages);
	}

}

// Entry Point
EFI_STATUS efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {

	init_globals(ImageHandle, SystemTable);

	cout->OutputString(cout, u"Opening Kernel.elf\r\n");

	EFI_FILE_PROTOCOL* file = open_file(u"kernel.elf");

	Elf64_Ehdr hdr;

	UINTN hdr_size = sizeof(Elf64_Ehdr);


	EFI_STATUS status = file->Read(file, &hdr_size, &hdr);
	if (hdr_size != sizeof(Elf64_Ehdr) || EFI_ERROR(status)){
		cout->OutputString(cout, u"Error Reading ELF file\r\n");
		while (1);
		
	} 
	if (!validate_elf(hdr)) {
		cout->OutputString(cout, u"Could Not validate Header\r\n");
		while (1);
	}



	Load_Kernel(file, hdr);
	

	status = bs->AllocatePages(AllocateAnyPages, EfiLoaderData, 4, &stack_bottom);
		if (EFI_ERROR(status)){
			cout->OutputString(cout, u"Could not allocate pages \r\n");
			while(1);
		}
	stack_top = stack_bottom + (4096 * 4);

	Read_Memory_Map();

    while (1);

    return EFI_SUCCESS;
}