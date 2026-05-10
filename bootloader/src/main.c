#include "efi.h"

EFI_SIMPLE_TEXT_INPUT_PROTOCOL *cin = NULL;
EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *cout = NULL;
EFI_BOOT_SERVICES *bs;
EFI_HANDLE image = NULL;


void init_globals(EFI_HANDLE handle, EFI_SYSTEM_TABLE *systable) {
	cout = systable->ConOut;
	cin = systable->ConIn;
	bs = systable->BootServices;
	image = handle;
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

// Entry Point
EFI_STATUS efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {

	init_globals(ImageHandle, SystemTable);

    cout->Reset(cout, false);

    cout->ClearScreen(cout);

    cout->OutputString(cout, u"Hello World!\r\n");

	cin->Reset(cin, false);

	cout->OutputString(cout, u"Press Any key to continue\r\n");

	get_key();

	cout->OutputString(cout, u"Key Pressed!\r\n");


    while (1) ;

    return EFI_SUCCESS;
}