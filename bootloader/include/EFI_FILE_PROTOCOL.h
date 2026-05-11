#define EFI_FILE_MODE_READ       0x0000000000000001

typedef struct EFI_FILE_PROTOCOL EFI_FILE_PROTOCOL;

typedef
EFI_STATUS
(EFIAPI *EFI_FILE_OPEN) (
  IN EFI_FILE_PROTOCOL                  *This,
  OUT EFI_FILE_PROTOCOL                 **NewHandle,
  IN CHAR16                             *FileName,
  IN UINT64                             OpenMode,
  IN UINT64                             Attributes
  );

typedef
EFI_STATUS
(EFIAPI *EFI_FILE_READ) (
  IN EFI_FILE_PROTOCOL           *This,
  IN OUT UINTN                   *BufferSize,
  OUT VOID                       *Buffer
  );

typedef struct EFI_FILE_PROTOCOL {
  UINT64                          Revision;
  EFI_FILE_OPEN                   Open;
  void*                  Close;
  void*                 Delete;
  EFI_FILE_READ                   Read;
  void*                  Write;
  void*           GetPosition;
  void*           SetPosition;
  void*               GetInfo;
  void*               SetInfo;
  void*                  Flush;
  void*                OpenEx; // Added for revision 2
  void*                ReadEx; // Added for revision 2
  void*               WriteEx; // Added for revision 2
  void*               FlushEx; // Added for revision 2
} EFI_FILE_PROTOCOL;