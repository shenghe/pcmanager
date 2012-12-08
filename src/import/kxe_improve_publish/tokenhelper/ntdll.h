#ifndef NTDLL_USER_INCLUDED
#define NTDLL_USER_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#define IN
#define OUT

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
	IN  HANDLE					hFileHandle,
	OUT PIO_STATUS_BLOCK		pIoStatusBlock,
	OUT PVOID					pFileInformation,
	IN  ULONG					uLength,
	IN  FILE_INFORMATION_CLASS	FileInformationClass
	);

typedef struct 
{	
	ULONG	Length;
	HANDLE  RootDirectory;
	PUNICODE_STRING	ObjectName;
	ULONG	Attributes;
	PSECURITY_DESCRIPTOR SecurityDescriptor;
	PSECURITY_QUALITY_OF_SERVICE SecurityQualityOfService;
} OBJECT_ATTRIBUTES, *POBJECT_ATTRIBUTES;

typedef struct
{
	ULONG	NextEntryOffset;
	ULONG	ObjectCount;
	ULONG	HandleCount;
	ULONG	TypeNumber;
	ULONG	InvalidAttributes;
	ULONG   GenericMapping;
	ULONG   ValidAccessMask;
	DWORD   PoolType;
	UCHAR	Unknown;
	UCHAR	XpPadding[15];
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

typedef struct _OBJECT_NAME_INFORMATION
        {
/*000*/ UNICODE_STRING Name; // points to Buffer[]
/*008*/ WORD           Buffer [];
/*???*/ }
        OBJECT_NAME_INFORMATION,
     * POBJECT_NAME_INFORMATION,
    **PPOBJECT_NAME_INFORMATION;

typedef struct _OBJECT_TYPE_INFO
        {
/*000*/ UNICODE_STRING  Name; // points to Buffer[]
/*008*/ DWORD           ObjectCount;
/*00C*/ DWORD           HandleCount;
/*010*/ DWORD           Reserved1 [4];
/*020*/ DWORD           PeakObjectCount;
/*024*/ DWORD           PeakHandleCount;
/*028*/ DWORD           Reserved2 [4];
/*038*/ DWORD           InvalidAttributes;
/*03C*/ GENERIC_MAPPING GenericMapping;
/*04C*/ ACCESS_MASK     ValidAccessMask;
/*050*/ BOOLEAN         SecurityRequired;
/*051*/ BOOLEAN         MaintainHandleCount;
/*052*/ WORD            Reserved3;
/*054*/ BOOL            PagedPool;
/*058*/ DWORD           DefaultPagedPoolCharge;
/*05C*/ DWORD           DefaultNonPagedPoolCharge;
/*060*/ WORD            Buffer [];
/*???*/ }
        OBJECT_TYPE_INFORMATION,
     * POBJECT_TYPE_INFO,
    **PPOBJECT_TYPE_INFO;

typedef struct
{
	ULONG	ProcessId;
	UCHAR	ObjectTypeNumber;
	UCHAR	Flags;
	USHORT	Handle;
	PVOID	Object;
	DWORD	AccessMask;
} SYSTEM_HANDLE_INFORMATION;

typedef struct
{	
	ULONG	NextEntryOffset;
	PVOID	Object;
	ULONG	CreatorProcessId;
	USHORT	Unknown;
	USHORT	Flags;
	ULONG	PointerCount;
	ULONG	HandleCount;
	ULONG	PagedPoolUsage;
	ULONG	NonPagedPoolUsage;
	ULONG	ExclusiveProcessId;
	PVOID	SecurityDescriptor;
	UNICODE_STRING Name;

} SYSTEM_OBJECT_INFORMATION;

#define OBJ_CASE_INSENSITIVE 0x00000040


IMPORT DWORD  __stdcall NtCreateToken(
    PHANDLE					Token,
    ACCESS_MASK				DesiredAccess,
    POBJECT_ATTRIBUTES		ObjectAttributes,
    TOKEN_TYPE				Type,
    PLUID					AuthenticationId,
    PLARGE_INTEGER			ExpirationTime,
    PTOKEN_USER				User,
    PTOKEN_GROUPS			Groups,
    PTOKEN_PRIVILEGES		Privileges,
    PTOKEN_OWNER			Owner,
	PTOKEN_PRIMARY_GROUP	PrimaryGroup,
    PTOKEN_DEFAULT_DACL		DefaultDacl,
    PTOKEN_SOURCE			Source);

 DWORD (WINAPI * NtOpenFile)(HANDLE *, DWORD, OBJECT_ATTRIBUTES *, IO_STATUS_BLOCK *, DWORD, DWORD);
 DWORD (WINAPI * RtlInitUnicodeString)(UNICODE_STRING *, PCWSTR);

	/*typedef DWORD (__stdcall *NQIF)(
	IN HANDLE FileHandle,
	OUT PIO_STATUS_BLOCK IoStatusBlock,
	OUT PVOID FileInformation,
	IN ULONG Length,
	IN FILE_INFORMATION_CLASS FileInformationClass
	);*/

#pragma pack(pop)

#ifdef __cplusplus
}
#endif

#endif