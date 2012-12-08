#ifndef SCHELPER_PUBLIC_INC_
#define SCHELPER_PUBLIC_INC_

//////////////////////////////////////////////////////////////////////////

#define SCHELPER_NT_NAME				L"\\Device\\ScHelper"
#define SCHELPER_DOS_NAME				L"\\DosDevices\\ScHelper"
#define SCHELPER_LINK_NAME				L"\\\\.\\ScHelper"
#define SCHELPER_LINK_NAME2				L"\\??\\ScHelper"

//////////////////////////////////////////////////////////////////////////

#define FILE_DEVICE_SCHELPER			FILE_DEVICE_UNKNOWN

//////////////////////////////////////////////////////////////////////////

#define IOCTL_SC_OPEN_VOLUME			CTL_CODE(FILE_DEVICE_SCHELPER, \
	0x0000, METHOD_BUFFERED, FILE_ANY_ACCESS)

//////////////////////////////////////////////////////////////////////////

#pragma pack(push, 1)

typedef struct tagVolumeStruct {
	WCHAR vol_name[64];
} VolumeStruct;

#pragma pack(pop)

//////////////////////////////////////////////////////////////////////////

#endif  // !SCHELPER_PUBLIC_INC_

