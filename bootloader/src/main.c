#include "../include/efi.h"

EFI_SIMPLE_TEXT_INPUT_PROTOCOL *cin = NULL;
EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *cout = NULL;
EFI_BOOT_SERVICES *bs;
EFI_HANDLE image_handle = NULL;


void init_globals(EFI_HANDLE handle, EFI_SYSTEM_TABLE *systable) {
	cout = systable->ConOut;
	cin = systable->ConIn;
	bs = systable->BootServices;
	image_handle = handle;
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

void byte_to_char16(UINT8* input, CHAR16* output, UINTN len) {
	UINTN index = 0;
	while (index < len) {
		output[index] = (CHAR16)input[index];
		index++;
	}
	output[index] = u'\0';
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

// Entry Point
EFI_STATUS efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {

	init_globals(ImageHandle, SystemTable);

    cout->Reset(cout, false);

    cout->ClearScreen(cout);

    cout->OutputString(cout, u"Hello World!\r\n");

	cin->Reset(cin, false);

	cout->OutputString(cout, u"Press Any key to continue\r\n");

	get_key();

	cout->OutputString(cout, u"Opening Kernel.elf\r\n");

	EFI_FILE_PROTOCOL* file = open_file(u"kernel.elf");

	UINT8* buf[64];

	UINTN buffer_size = 64;
	
	file->Read(file, &buffer_size, buf);

	CHAR16 charbuf[21];

	uint_to_string(buffer_size, charbuf, 21);

	cout->OutputString(cout, u"Read ELF Header. Size: ");
	cout->OutputString(cout, charbuf);

    while (1) ;

    return EFI_SUCCESS;
}