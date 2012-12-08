#ifndef KNDISFLT_PUBLIC_INC_
#define KNDISFLT_PUBLIC_INC_


//////////////////////////////////////////////////////////////////////////

#define KNDISFLT_NTDEVICE_STRING        L"\\Device\\KNdisFlt"
#define KNDISFLT_LINKNAME_STRING        L"\\DosDevices\\KNdisFlt"
#define KNDISFLT_DRIVER_STRING          L"\\Driver\\KNdisFlt"

//////////////////////////////////////////////////////////////////////////

#ifndef REMOTE_FILTER_HELP
#define REMOTE_FILTER_HELP

typedef VOID (__stdcall *SET_PROCESS_FILTER_ROUTINE)(ULONG dwFilterAddress);
typedef VOID (__stdcall *SET_BUFFER_FILTER_ROUTINE)(ULONG dwFilterAddress);

typedef INT (__stdcall *PROCESS_FILTER_ROUTINE)(ULONG dwProcessId, ULONG dwThreadId);
typedef INT (__stdcall *BUFFER_FILTER_ROUTINE)(PVOID pBuffer, ULONG PacketSize, BOOLEAN Outbound);


#endif  // !REMOTE_FILTER_HELP

//////////////////////////////////////////////////////////////////////////


#endif  // !KNDISFLT_PUBLIC_INC_

