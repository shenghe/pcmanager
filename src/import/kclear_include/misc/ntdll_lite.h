#ifndef NTDLL_USER_INCLUDED
#define NTDLL_USER_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#define IN
#define OUT

#ifndef NTAPI
#define NTAPI __stdcall
#endif

#ifndef NTSYSAPI
#define NTSYSAPI     DECLSPEC_IMPORT
#endif

#ifndef NT_SUCCESS
#define NT_SUCCESS(Status) (((NTSTATUS)(Status)) >= 0)
#endif

#define IMPORT __declspec(dllimport)

#ifndef NTSTATUS
typedef LONG NTSTATUS;
#endif

#pragma pack(push, 1)

typedef struct
{
    USHORT Length;
    USHORT MaximumLength;
    PWSTR  Buffer;

} UNICODE_STRING, *PUNICODE_STRING;
typedef const UNICODE_STRING *PCUNICODE_STRING;

//
// Define the file information class values
//
// WARNING:  The order of the following values are assumed by the I/O system.
//           Any changes made here should be reflected there as well.
//

typedef enum 
{
    FileDirectoryInformation = 1,
    FileFullDirectoryInformation,
    FileBothDirectoryInformation,
    FileBasicInformation,
    FileStandardInformation,
    FileInternalInformation,
    FileEaInformation,
    FileAccessInformation,
    FileNameInformation,
    FileRenameInformation,
    FileLinkInformation,
    FileNamesInformation,
    FileDispositionInformation,
    FilePositionInformation,
    FileFullEaInformation,
    FileModeInformation,
    FileAlignmentInformation,
    FileAllInformation,
    FileAllocationInformation,
    FileEndOfFileInformation,
    FileAlternateNameInformation,
    FileStreamInformation,
    FilePipeInformation,
    FilePipeLocalInformation,
    FilePipeRemoteInformation,
    FileMailslotQueryInformation,
    FileMailslotSetInformation,
    FileCompressionInformation,
    FileCopyOnWriteInformation,
    FileCompletionInformation,
    FileMoveClusterInformation,
    FileOleClassIdInformation,
    FileOleStateBitsInformation,
    FileNetworkOpenInformation,
    FileObjectIdInformation,
    FileOleAllInformation,
    FileOleDirectoryInformation,
    FileContentIndexInformation,
    FileInheritContentIndexInformation,
    FileOleInformation,
    FileMaximumInformation

} FILE_INFORMATION_CLASS, *PFILE_INFORMATION_CLASS;

//
// Define the various structures which are returned on query operations
//

typedef struct 
{
    LARGE_INTEGER CreationTime;
    LARGE_INTEGER LastAccessTime;
    LARGE_INTEGER LastWriteTime;
    LARGE_INTEGER ChangeTime;
    ULONG FileAttributes;

} FILE_BASIC_INFORMATION, *PFILE_BASIC_INFORMATION;

typedef struct _FILE_LINK_INFORMATION {
    BOOLEAN ReplaceIfExists;
    HANDLE  RootDirectory;
    ULONG   FileNameLength;
    WCHAR   FileName[1];
} FILE_LINK_INFORMATION, *PFILE_LINK_INFORMATION;

typedef struct 
{
    LARGE_INTEGER AllocationSize;
    LARGE_INTEGER EndOfFile;
    ULONG NumberOfLinks;
    BOOLEAN DeletePending;
    BOOLEAN Directory;

} FILE_STANDARD_INFORMATION, *PFILE_STANDARD_INFORMATION;

typedef struct 
{
    LARGE_INTEGER CurrentByteOffset;

} FILE_POSITION_INFORMATION, *PFILE_POSITION_INFORMATION;

typedef struct 
{
    ULONG AlignmentRequirement;

} FILE_ALIGNMENT_INFORMATION, *PFILE_ALIGNMENT_INFORMATION;

typedef struct 
{
    LARGE_INTEGER CreationTime;
    LARGE_INTEGER LastAccessTime;
    LARGE_INTEGER LastWriteTime;
    LARGE_INTEGER ChangeTime;
    LARGE_INTEGER AllocationSize;
    LARGE_INTEGER EndOfFile;
    ULONG FileAttributes;

} FILE_NETWORK_OPEN_INFORMATION, *PFILE_NETWORK_OPEN_INFORMATION;

typedef struct 
{
    BOOLEAN DeleteFile;

} FILE_DISPOSITION_INFORMATION, *PFILE_DISPOSITION_INFORMATION;

typedef struct 
{
    LARGE_INTEGER EndOfFile;

} FILE_END_OF_FILE_INFORMATION, *PFILE_END_OF_FILE_INFORMATION;


typedef struct 
{
    ULONG NextEntryOffset;
    UCHAR Flags;
    UCHAR EaNameLength;
    USHORT EaValueLength;
    CHAR EaName[1];

} FILE_FULL_EA_INFORMATION, *PFILE_FULL_EA_INFORMATION;


typedef struct 
{
    ULONG NextEntryOffset;
    ULONG StreamNameLength;
    LARGE_INTEGER StreamSize;
    LARGE_INTEGER StreamAllocationSize;
    WCHAR StreamName[1];

} FILE_STREAM_INFORMATION;


//
// Define the base asynchronous I/O argument types
//

typedef struct 
{
    NTSTATUS Status;
    ULONG Information;

} IO_STATUS_BLOCK, *PIO_STATUS_BLOCK;


DWORD (WINAPI * NtQueryInformationFile)(
    IN  HANDLE                  hFileHandle,
    OUT PIO_STATUS_BLOCK        pIoStatusBlock,
    OUT PVOID                   pFileInformation,
    IN  ULONG                   uLength,
    IN  FILE_INFORMATION_CLASS  FileInformationClass
    );

typedef struct 
{   
    ULONG   Length;
    HANDLE  RootDirectory;
    PUNICODE_STRING ObjectName;
    ULONG   Attributes;
    PSECURITY_DESCRIPTOR SecurityDescriptor;
    PSECURITY_QUALITY_OF_SERVICE SecurityQualityOfService;
} OBJECT_ATTRIBUTES, *POBJECT_ATTRIBUTES;

typedef struct
{
    ULONG   NextEntryOffset;
    ULONG   ObjectCount;
    ULONG   HandleCount;
    ULONG   TypeNumber;
    ULONG   InvalidAttributes;
    ULONG   GenericMapping;
    ULONG   ValidAccessMask;
    DWORD   PoolType;
    UCHAR   Unknown;
    UCHAR   XpPadding[15];
    UNICODE_STRING Name;

} SYSTEM_OBJECT_TYPE_INFORMATION;

typedef struct _OBJECT_HANDLE_INFORMATION // cf. HANDLE_ENTRY
        {
/*000*/ DWORD       HandleAttributes; // cf. HANDLE_ATTRIBUTE_MASK
/*004*/ ACCESS_MASK GrantedAccess;
/*008*/ }
        OBJECT_HANDLE_INFORMATION,
     * POBJECT_HANDLE_INFORMATION,
    **PPOBJECT_HANDLE_INFORMATION;

#define OBJECT_HANDLE_INFORMATION_ \
        sizeof (OBJECT_HANDLE_INFORMATION)

// -----------------------------------------------------------------

typedef struct
{
    ULONG   ProcessId;
    UCHAR   ObjectTypeNumber;
    UCHAR   Flags;
    USHORT  Handle;
    PVOID   Object;
    DWORD   AccessMask;
} SYSTEM_HANDLE_INFORMATION;

typedef struct
{   
    ULONG   NextEntryOffset;
    PVOID   Object;
    ULONG   CreatorProcessId;
    USHORT  Unknown;
    USHORT  Flags;
    ULONG   PointerCount;
    ULONG   HandleCount;
    ULONG   PagedPoolUsage;
    ULONG   NonPagedPoolUsage;
    ULONG   ExclusiveProcessId;
    PVOID   SecurityDescriptor;
    UNICODE_STRING Name;

} SYSTEM_OBJECT_INFORMATION;

#define OBJ_CASE_INSENSITIVE 0x00000040


IMPORT DWORD  __stdcall NtCreateToken(
    PHANDLE                 Token,
    ACCESS_MASK             DesiredAccess,
    POBJECT_ATTRIBUTES      ObjectAttributes,
    TOKEN_TYPE              Type,
    PLUID                   AuthenticationId,
    PLARGE_INTEGER          ExpirationTime,
    PTOKEN_USER             User,
    PTOKEN_GROUPS           Groups,
    PTOKEN_PRIVILEGES       Privileges,
    PTOKEN_OWNER            Owner,
    PTOKEN_PRIMARY_GROUP    PrimaryGroup,
    PTOKEN_DEFAULT_DACL     DefaultDacl,
    PTOKEN_SOURCE           Source);

//  DWORD (WINAPI * NtOpenFile)(HANDLE *, DWORD, OBJECT_ATTRIBUTES *, IO_STATUS_BLOCK *, DWORD, DWORD);
//  DWORD (WINAPI * RtlInitUnicodeString)(UNICODE_STRING *, PCWSTR);

    /*typedef DWORD (__stdcall *NQIF)(
    IN HANDLE FileHandle,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    OUT PVOID FileInformation,
    IN ULONG Length,
    IN FILE_INFORMATION_CLASS FileInformationClass
    );*/

#pragma pack(pop)

typedef enum _KEY_INFORMATION_CLASS {
    KeyBasicInformation,
    KeyNodeInformation,
    KeyFullInformation,
    KeyNameInformation,
    KeyCachedInformation,
    KeyFlagsInformation,
    KeyVirtualizationInformation,
    MaxKeyInfoClass  // MaxKeyInfoClass should always be the last enum
} KEY_INFORMATION_CLASS;

typedef enum _KEY_VALUE_INFORMATION_CLASS {
    KeyValueBasicInformation,
    KeyValueFullInformation,
    KeyValuePartialInformation,
    KeyValueFullInformationAlign64,
    KeyValuePartialInformationAlign64,
    MaxKeyValueInfoClass  // MaxKeyValueInfoClass should always be the last enum
} KEY_VALUE_INFORMATION_CLASS;

typedef enum _SE_OBJECT_TYPE
{
    SE_UNKNOWN_OBJECT_TYPE = 0,
    SE_FILE_OBJECT,
    SE_SERVICE,
    SE_PRINTER,
    SE_REGISTRY_KEY,
    SE_LMSHARE,
    SE_KERNEL_OBJECT,
    SE_WINDOW_OBJECT,
    SE_DS_OBJECT,
    SE_DS_OBJECT_ALL,
    SE_PROVIDER_DEFINED_OBJECT,
    SE_WMIGUID_OBJECT,
    SE_REGISTRY_WOW64_32KEY
} SE_OBJECT_TYPE;

typedef enum _EVENT_TYPE {
    NotificationEvent,
    SynchronizationEvent
} EVENT_TYPE;

typedef enum _SYSTEM_INFORMATION_CLASS {
    SystemBasicInformation = 0,
    SystemPerformanceInformation = 2,
    SystemTimeOfDayInformation = 3,
    SystemProcessInformation = 5,
    SystemProcessorPerformanceInformation = 8,
    SystemInterruptInformation = 23,
    SystemExceptionInformation = 33,
    SystemRegistryQuotaInformation = 37,
    SystemLookasideInformation = 45
} SYSTEM_INFORMATION_CLASS;

typedef VOID (NTAPI *PIO_APC_ROUTINE)(
                                      PVOID ApcContext,
                                      PIO_STATUS_BLOCK IoStatusBlock,
                                      ULONG Reserved
                                      );

//////////////////////////////////////////////////////////////////////////
// 公用

NTSYSAPI
NTSTATUS
NTAPI
NtClose(
        HANDLE Handle
        );

NTSYSAPI
VOID 
NTAPI
RtlInitUnicodeString(
                     PUNICODE_STRING DestinationString,
                     PCWSTR SourceString
                     );

NTSYSAPI
void 
NTAPI
RtlFreeUnicodeString(
                     PUNICODE_STRING UnicodeString
                     );

NTSYSAPI
SIZE_T 
NTAPI
RtlCompareMemory(
                 const VOID *Source1,
                 const VOID *Source2,
                 SIZE_T Length
                 );

NTSYSAPI
NTSTATUS
NTAPI
NtQuerySystemInformation (
                          SYSTEM_INFORMATION_CLASS SystemInformationClass,
                          PVOID SystemInformation,
                          ULONG SystemInformationLength,
                          PULONG ReturnLength
                          );

NTSYSAPI
NTSTATUS 
NTAPI
RtlStringFromGUID(
                  REFGUID Guid,
                  PUNICODE_STRING GuidString
                  );

NTSYSAPI
NTSTATUS 
NTAPI
RtlGUIDFromString(
                  PCUNICODE_STRING GuidString,
                  GUID *Guid
                  );

NTSYSAPI
NTSTATUS
NTAPI
NtWaitForSingleObject(
                      HANDLE Handle,
                      BOOLEAN Alertable,
                      PLARGE_INTEGER Timeout
                      );

NTSYSAPI
NTSTATUS 
NTAPI
NtDeviceIoControlFile(
                      HANDLE FileHandle,
                      HANDLE Event,
                      PIO_APC_ROUTINE ApcRoutine,
                      PVOID ApcContext,
                      PIO_STATUS_BLOCK IoStatusBlock,
                      ULONG IoControlCode,
                      PVOID InputBuffer,
                      ULONG InputBufferLength,
                      PVOID OutputBuffer,
                      ULONG OutputBufferLength
                      );

NTSYSAPI
NTSTATUS 
NTAPI
NtCreateEvent(
              PHANDLE EventHandle,
              ACCESS_MASK DesiredAccess,
              POBJECT_ATTRIBUTES ObjectAttributes,
              EVENT_TYPE EventType,
              BOOLEAN InitialState
              );

//////////////////////////////////////////////////////////////////////////
// 文件



//////////////////////////////////////////////////////////////////////////
// 注册表
NTSYSAPI
NTSTATUS 
NTAPI
NtOpenKey(
          PHANDLE KeyHandle,
          ACCESS_MASK DesiredAccess,
          POBJECT_ATTRIBUTES ObjectAttributes
          );

//////////////////////////////////////////////////////////////////////////
// 安全
NTSYSAPI
NTSTATUS 
NTAPI
RtlCreateAcl(
             PACL Acl,
             ULONG AclLength,
             ULONG AclRevision
             );

NTSYSAPI
NTSTATUS
NTAPI
RtlAllocateAndInitializeSid(
                            PSID_IDENTIFIER_AUTHORITY IdentifierAuthority,
                            UCHAR SubAuthorityCount,
                            ULONG SubAuthority0,
                            ULONG SubAuthority1,
                            ULONG SubAuthority2,
                            ULONG SubAuthority3,
                            ULONG SubAuthority4,
                            ULONG SubAuthority5,
                            ULONG SubAuthority6,
                            ULONG SubAuthority7,
                            PSID *Sid
                            );

NTSYSAPI
NTSTATUS
NTAPI
NtSetSecurityObject(
                    HANDLE Handle,
                    SECURITY_INFORMATION SecurityInformation,
                    PSECURITY_DESCRIPTOR SecurityDescriptor
                    );

NTSYSAPI
ULONG
NTAPI
RtlLengthSid(
             PSID Sid
             );

NTSYSAPI
NTSTATUS 
NTAPI
RtlAddAccessAllowedAceEx(
                         PACL Acl,
                         ULONG AceRevision,
                         ULONG AceFlags,
                         ACCESS_MASK AccessMask,
                         PSID Sid
                         );

NTSYSAPI
NTSTATUS
NTAPI
RtlCreateSecurityDescriptor(
                            PSECURITY_DESCRIPTOR SecurityDescriptor,
                            ULONG Revision
                            );

NTSYSAPI
NTSTATUS 
NTAPI
RtlSetDaclSecurityDescriptor(
                             PSECURITY_DESCRIPTOR SecurityDescriptor,
                             BOOLEAN DaclPresent,
                             PACL Dacl,
                             BOOLEAN DaclDefaulted
                             );

NTSYSAPI
ULONG 
NTAPI
RtlLengthSecurityDescriptor(
                            PSECURITY_DESCRIPTOR SecurityDescriptor
                            );

NTSYSAPI
NTSTATUS
NTAPI
RtlSetOwnerSecurityDescriptor(
                              PSECURITY_DESCRIPTOR SecurityDescriptor,
                              PSID Owner,
                              BOOLEAN OwnerDefaulted
                              );

NTSYSAPI
PVOID
NTAPI
RtlFreeSid(
           IN PSID Sid
           );

NTSYSAPI
NTSTATUS 
NTAPI
ZwSetSecurityObject(
                    HANDLE Handle,
                    SECURITY_INFORMATION SecurityInformation,
                    PSECURITY_DESCRIPTOR SecurityDescriptor
                    );


NTSYSAPI
NTSTATUS 
NTAPI
ZwEnumerateKey(
               HANDLE KeyHandle,
               ULONG Index,
               KEY_INFORMATION_CLASS KeyInformationClass,
               PVOID KeyInformation,
               ULONG Length,
               PULONG ResultLength
               );

NTSYSAPI
NTSTATUS 
NTAPI
NtQueryKey(
           HANDLE KeyHandle,
           KEY_INFORMATION_CLASS KeyInformationClass,
           PVOID KeyInformation,
           ULONG Length,
           PULONG ResultLength
           );

NTSYSAPI
NTSTATUS 
NTAPI
NtQueryAttributesFile(
                      POBJECT_ATTRIBUTES ObjectAttributes,
                      PFILE_BASIC_INFORMATION FileInformation
                      );

NTSYSAPI
NTSTATUS
NTAPI
NtUnloadKey(
            POBJECT_ATTRIBUTES   DestinationKeyName 
            );

NTSYSAPI
NTSTATUS
NTAPI
NtLoadKey(
          POBJECT_ATTRIBUTES   DestinationKeyName,
          POBJECT_ATTRIBUTES   HiveFileName 
          );

NTSYSAPI
NTSTATUS
NTAPI
NtAdjustPrivilegesToken(
                        HANDLE               TokenHandle,
                        BOOLEAN              DisableAllPrivileges,
                        PTOKEN_PRIVILEGES    TokenPrivileges,
                        ULONG                PreviousPrivilegesLength,
                        PTOKEN_PRIVILEGES    PreviousPrivileges,
                        PULONG               RequiredLength
                        );

NTSYSAPI
NTSTATUS
NTAPI
NtOpenProcessToken(
                   HANDLE               ProcessHandle,
                   ACCESS_MASK          DesiredAccess,
                   PHANDLE              TokenHandle 
                   );

NTSYSAPI
NTSTATUS
NTAPI
NtOpenThreadToken(
                  HANDLE               ThreadHandle,
                  ACCESS_MASK          DesiredAccess,
                  BOOLEAN              OpenAsSelf,
                  PHANDLE              TokenHandle 
                  );

NTSYSAPI
NTSTATUS 
NTAPI
NtCreateKey(
            PHANDLE KeyHandle,
            ACCESS_MASK DesiredAccess,
            POBJECT_ATTRIBUTES ObjectAttributes,
            ULONG TitleIndex,
            PUNICODE_STRING Class,
            ULONG CreateOptions,
            PULONG Disposition
            );

NTSYSAPI
NTSTATUS 
NTAPI
NtDeleteValueKey(
                 HANDLE KeyHandle,
                 PUNICODE_STRING ValueName
                 );

NTSYSAPI
NTSTATUS 
NTAPI
NtQueryValueKey(
                HANDLE KeyHandle,
                PUNICODE_STRING ValueName,
                KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,
                PVOID KeyValueInformation,
                ULONG Length,
                PULONG ResultLength
                );

NTSYSAPI
NTSTATUS 
NTAPI
NtSetValueKey(
              HANDLE KeyHandle,
              PUNICODE_STRING ValueName,
              ULONG TitleIndex,
              ULONG Type,
              PVOID Data,
              ULONG DataSize
              );

NTSYSAPI
NTSTATUS
NTAPI
NtSaveKey(
          HANDLE               KeyHandle,
          HANDLE               FileHandle 
          );

NTSYSAPI
NTSTATUS 
NTAPI
ZwDeleteKey(
            HANDLE KeyHandle
            );

NTSYSAPI
NTSTATUS
NTAPI
NtAllocateUuids(
                PLARGE_INTEGER      Time,
                PULONG              Range,
                PULONG              Sequence 
                );

NTSYSAPI
NTSTATUS
NTAPI
NtResetEvent(
             HANDLE               EventHandle,
             PLONG                PreviousState
             );

//////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
}
#endif

#endif