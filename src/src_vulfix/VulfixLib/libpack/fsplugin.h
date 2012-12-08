// contents of fsplugin.h  version 1.3 (10.Dec.2002)

// ids for FsGetFile
#define FS_FILE_OK 0
#define FS_FILE_EXISTS 1
#define FS_FILE_NOTFOUND 2
#define FS_FILE_READERROR 3
#define FS_FILE_WRITEERROR 4
#define FS_FILE_USERABORT 5
#define FS_FILE_NOTSUPPORTED 6
#define FS_FILE_EXISTSRESUMEALLOWED 7

#define FS_EXEC_OK 0
#define FS_EXEC_ERROR 1
#define FS_EXEC_YOURSELF -1
#define FS_EXEC_SYMLINK -2

#define FS_COPYFLAGS_OVERWRITE 1
#define FS_COPYFLAGS_RESUME 2
#define FS_COPYFLAGS_MOVE 4
#define FS_COPYFLAGS_EXISTS_SAMECASE 8
#define FS_COPYFLAGS_EXISTS_DIFFERENTCASE 16
 
// flags for tRequestProc
#define RT_Other 0
#define RT_UserName 1
#define RT_Password 2
#define RT_Account 3
#define RT_UserNameFirewall 4
#define RT_PasswordFirewall 5
#define RT_TargetDir 6
#define RT_URL 7
#define RT_MsgOK 8
#define RT_MsgYesNo 9
#define RT_MsgOKCancel 10

// flags for tLogProc
#define MSGTYPE_CONNECT 1
#define MSGTYPE_DISCONNECT 2
#define MSGTYPE_DETAILS 3
#define MSGTYPE_TRANSFERCOMPLETE 4
#define MSGTYPE_CONNECTCOMPLETE 5
#define MSGTYPE_IMPORTANTERROR 6
#define MSGTYPE_OPERATIONCOMPLETE 7

// flags for FsStatusInfo
#define FS_STATUS_START 0
#define FS_STATUS_END 1

#define FS_STATUS_OP_LIST 1
#define FS_STATUS_OP_GET_SINGLE 2
#define FS_STATUS_OP_GET_MULTI 3
#define FS_STATUS_OP_PUT_SINGLE 4
#define FS_STATUS_OP_PUT_MULTI 5
#define FS_STATUS_OP_RENMOV_SINGLE 6
#define FS_STATUS_OP_RENMOV_MULTI 7
#define FS_STATUS_OP_DELETE 8
#define FS_STATUS_OP_ATTRIB 9
#define FS_STATUS_OP_MKDIR 10
#define FS_STATUS_OP_EXEC 11
#define FS_STATUS_OP_CALCSIZE 12
#define FS_STATUS_OP_SEARCH 13
#define FS_STATUS_OP_SEARCH_TEXT 14
#define FS_STATUS_OP_SYNC_SEARCH 15
#define FS_STATUS_OP_SYNC_GET 16
#define FS_STATUS_OP_SYNC_PUT 17
#define FS_STATUS_OP_SYNC_DELETE 18

#define FS_ICONFLAG_SMALL 1
#define FS_ICONFLAG_BACKGROUND 2

#define FS_ICON_USEDEFAULT 0
#define FS_ICON_EXTRACTED 1
#define FS_ICON_EXTRACTED_DESTROY 2
#define FS_ICON_DELAYED 3

typedef struct {
    DWORD SizeLow,SizeHigh;
    FILETIME LastWriteTime;
    int Attr;
} RemoteInfoStruct;

typedef struct {
	int size;
	DWORD PluginInterfaceVersionLow;
	DWORD PluginInterfaceVersionHi;
	char DefaultIniName[MAX_PATH];
} FsDefaultParamStruct;

// callback functions
typedef int (__stdcall *tProgressProc)(int PluginNr,char* SourceName,
             char* TargetName,int PercentDone);
typedef void (__stdcall *tLogProc)(int PluginNr,int MsgType,char* LogString);
typedef BOOL (__stdcall *tRequestProc)(int PluginNr,int RequestType,char* CustomTitle,
              char* CustomText,char* ReturnedText,int maxlen);

// Function prototypes
int __stdcall FsInit(int PluginNr,tProgressProc pProgressProc,
                     tLogProc pLogProc,tRequestProc pRequestProc);
HANDLE __stdcall FsFindFirst(char* Path,WIN32_FIND_DATA *FindData);
BOOL __stdcall FsFindNext(HANDLE Hdl,WIN32_FIND_DATA *FindData);
int __stdcall FsFindClose(HANDLE Hdl);
BOOL __stdcall FsMkDir(char* Path);
int __stdcall FsExecuteFile(HWND MainWin,char* RemoteName,char* Verb);
int __stdcall FsRenMovFile(char* OldName,char* NewName,BOOL Move,
                           BOOL OverWrite,RemoteInfoStruct* ri);
int __stdcall FsGetFile(char* RemoteName,char* LocalName,int CopyFlags,
                        RemoteInfoStruct* ri);
int __stdcall FsPutFile(char* LocalName,char* RemoteName,int CopyFlags);
BOOL __stdcall FsDeleteFile(char* RemoteName);
BOOL __stdcall FsRemoveDir(char* RemoteName);
BOOL __stdcall FsDisconnect(char* DisconnectRoot);
BOOL __stdcall FsSetAttr(char* RemoteName,int NewAttr);
BOOL __stdcall FsSetTime(char* RemoteName,FILETIME *CreationTime,
      FILETIME *LastAccessTime,FILETIME *LastWriteTime);
void __stdcall FsStatusInfo(char* RemoteDir,int InfoStartEnd,int InfoOperation);
void __stdcall FsGetDefRootName(char* DefRootName,int maxlen);
int __stdcall FsExtractCustomIcon(char* RemoteName,int ExtractFlags,HICON* TheIcon);
void __stdcall FsSetDefaultParams(FsDefaultParamStruct* dps);
