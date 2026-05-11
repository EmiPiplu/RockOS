typedef
EFI_STATUS
(EFIAPI *EFI_WAIT_FOR_EVENT) (
   IN UINTN             NumberOfEvents,
   IN EFI_EVENT         *Event,
   OUT UINTN            *Index
  );

typedef
EFI_STATUS
(EFIAPI *EFI_HANDLE_PROTOCOL) (
   IN EFI_HANDLE                    Handle,
   IN EFI_GUID                      *Protocol,
   OUT VOID                         **Interface
   );


typedef struct {
	EFI_TABLE_HEADER Hdr;

	void* RaiseTPL;
	void* RestoreTPL;

	void* AllocatePages;
	void* FreePages;
	void* GetMemoryMap;
	void* AllocatePool;
	void* FreePool;

	void* CreateEvent;
	void* SetTimer;
	EFI_WAIT_FOR_EVENT WaitForEvent;
	void* SignalEvent;
	void* CloseEvent;
	void* CheckEvent;

	void* InstallProtocolInterface;
	void* ReinstallProtocolInterface;
	void* UninstallProtocolInterface;
	EFI_HANDLE_PROTOCOL HandleProtocol;

} EFI_BOOT_SERVICES;