; // ***** Sample.mc *****

; // This is the header section.

MessageIdTypedef=DWORD


SeverityNames=(
	Success=0x0:STATUS_SEVERITY_SUCCESS
    Informational=0x1:STATUS_SEVERITY_INFORMATIONAL
    Warning=0x2:STATUS_SEVERITY_WARNING
    Error=0x3:STATUS_SEVERITY_ERROR
    )


FacilityNames=(
	KxEBase=0x0:FACILITY_KXEBASE
    KxECfg=0x2:FACILITY_KXECFG
    KxEThread=0x3:FACILITY_KXETHREAD
    KxEEvent=0x4:FACILITY_KXEEVENT
	KxEHttp=0x5:FACILITY_KXEHTTP
	KxEScanApi=0x6:FACILITY_KXESCAN_API
	KxEIdl=0x7:FACILITY_IDL
	KxERegister=0x8:FACILITY_REGISTER
	KxEPassport=0x9:FACILITY_PASSPORT
	KxEServiceMgr=0xa:FACILITY_SERVICEMGR
	KxEIPC=0xb:FACILITY_IPC
	KxEPipe=0xc:FACILITY_PIPE
	
	KxEXmlHttp=0xd:FACILITY_XML_HTTP_REQUEST

	KSE=0xE2:FACILITY_KSE
	KSESqlite=0xE3:FACILITY_KSE_SQLITY
	KSEScanner=0xE4:FACILITY_KSE_SCANNER
	KSEProc=0xE5:FACILITY_KSE_PROC
	KSEFo=0xE6:FACILITY_KSE_FO
	KSEFileEngine=0xE7:FACILITY_KSE_FILEENGINE
	KSEPluginSDK=0xE8:FACILITY_KSE_PLUGIN_SDK
	KSENetDet=0xE9:FACILITY_KSE_NET_DET
	
	KWFS=0xF1:FACILITY_WFS
	KWFDB=0xF2:FACILITY_WFS_FDB
	KWSP=0xF3:FACILITY_WFS_SP
	
	KxEFM=0xA1:FACILITY_FILEMON
	KxEFMSP=0xA2:FACILITY_FILEMON_SP
	KxEFMSDK=0xA3:FACILITY_FILEMON_SDK

    KxESCSP=0xB1:FACILITY_SECURITYCENTER_SP
    KxESRSP=0xA4:FACILITY_SR_SP

        KxEBasicSP=0xA5:FACILITY_BASIC_SP
)


LanguageNames=(English=0x409:MSG00409)
LanguageNames=(Chinese=0x804:MSG00804)

; // The following are message definitions.
;

MessageId=0x1
Severity=Error
Facility=KxEXmlHttp
SymbolicName=E_KXMLHTTP_NOT_OPEN
Language=English
The internet has not been open as the function open has not been called sucessfully yet.
.
Language=Chinese
Internet 尚未被打开，因为函数 open 未被成功调用
.

MessageId=0x2
Severity=Error
Facility=KxEXmlHttp
SymbolicName=E_KXMLHTTP_NOT_ENOUGH_MEMORY
Language=English
Call new operator return a null pointer.
.
Language=Chinese
调用 new 操作符返回空指针
.

MessageId=0x3
Severity=Error
Facility=KxEXmlHttp
SymbolicName=E_KXMLHTTP_INCORRECT_RETURN_DATA
Language=English
The return data from the pipe server is incorrect.
.
Language=Chinese
从管道服务器端返回的数据不正确
.

MessageId=0x4
Severity=Error
Facility=KxEXmlHttp
SymbolicName=E_KXMLHTTP_INVALID_PARAMETER
Language=English
The parameter is invalid.
.
Language=Chinese
参数有误
.

MessageId=0x5
Severity=Error
Facility=KxEXmlHttp
SymbolicName=E_KXMLHTTP_PIPE_SERVER_NOT_RUNNING
Language=English
The named pipe server is not running right now.
.
Language=Chinese
命名管道服务器当前没有运行
.

MessageId=0x6
Severity=Error
Facility=KxEXmlHttp
SymbolicName=E_KXMLHTTP_CREATE_CLIENT_PIPE_INSTANCE_FAILED
Language=English
Create named pipe instance by call API CreateFile failed on the client end.
.
Language=Chinese
客户端通过调用API CreateFile 创建命名管道失败
.

MessageId=0x7
Severity=Error
Facility=KxEXmlHttp
SymbolicName=E_KXMLHTTP_CREATE_IO_COMPLETION_PORT_FAILED
Language=English
Create IO Completion Port( IOCP ) for the named pipe failed on the client end.
.
Language=Chinese
客户端为命名管道创建 IO 完成端口失败
.

MessageId=0x8
Severity=Error
Facility=KxEXmlHttp
SymbolicName=E_KXMLHTTP_CREATE_THREAD_FAILED
Language=English
Create thread for processing I/O request failed.
.
Language=Chinese
创建处理 I/O 请求的线程失败
.

MessageId=0x9
Severity=Error
Facility=KxEXmlHttp
SymbolicName=E_KXMLHTTP_NOT_ALL_IO_PROCESSING_THREADS_WERE_NOTIFIED_TO_EXIT
Language=English
Not all of the threads for processing I/O request were notifyed to stop.
.
Language=Chinese
不是所有处理 I/O 请求的线程都被通告退出
.

MessageId=0xa
Severity=Error
Facility=KxEXmlHttp
SymbolicName=E_KXMLHTTP_WAIT_FOR_THREAD_EXIT_FAILED
Language=English
Wait for all the threads for processing I/O request failed.
.
Language=Chinese
等待所有处理 I/O 请求的线程的终止失败
.

MessageId=0xb
Severity=Error
Facility=KxEXmlHttp
SymbolicName=E_KXMLHTTP_PIPE_WRITE_FILE_FAILED
Language=English
Call the API WriteFile to write pipe failed.
.
Language=Chinese
调用API WriteFile 以向管道写入数据时失败
.

MessageId=0xc
Severity=Error
Facility=KxEXmlHttp
SymbolicName=E_KXMLHTP_PIPE_READ_FILE_FAILED
Language=English
Call the API ReadFile to read pipe failed.
.
Language=Chinese
调用API ReadFile 以从管道读出数据时失败
.

MessageId=0xd
Severity=Error
Facility=KxEXmlHttp
SymbolicName=E_KXMLHTTP_PREVIOUS_REQUEST_NOT_COMPLETED
Language=English
Request is denied because previous request has not completed.
.
Language=Chinese
前一次的请求尚未完成导致本次请求被拒绝
.

MessageId=0xe
Severity=Error
Facility=KxEXmlHttp
SymbolicName=E_KXMLHTTP_FAILED_TO_GET_SERVICE_PORT
Language=English
It failed to get service port of the HTTP server.
.
Language=Chinese
获取 HTTP 服务器端口失败
.

MessageId=0xf
Severity=Error
Facility=KxEXmlHttp
SymbolicName=E_KXMLHTTP_TIME_OUT
Language=English
Operation failed with time out.
.
Language=Chinese
操作因超时而失败
.

; // KxEPipe
MessageId=0x1
Severity=Error
Facility=KxEPipe
SymbolicName=E_KXEPIPE_CREATE_NAMED_PIPE_FAILED
Language=English
Create named pipe failed.
.
Language=Chinese

调用CreateNamedPipe创建命名管道失败
.

MessageId=0x2
Severity=Error
Facility=KxEPipe
SymbolicName=E_KXEPIPE_CREATE_THREAD_FAILED
Language=English
Create thread failed.
.
Language=Chinese

调用CreateThread创建线程失败
.

MessageId=0x3
Severity=Error
Facility=KxEPipe
SymbolicName=E_KXEPIPE_CONNECT_PIPE_FAILED
Language=English
The server end of the named pipe connect the client failed.
.
Language=Chinese

命名管道服务器端连接客户端失败
.

MessageId=0x4
Severity=Error
Facility=KxEPipe
SymbolicName=E_KXEPIPE_WAIT_NAMED_PIPE_FAILED
Language=English
The client end of the named pipe connect the server end failed.
.
Language=Chinese

命名管道客户端连接服务器端失败
.

MessageId=0x5
Severity=Error
Facility=KxEPipe
SymbolicName=E_KXEPIPE_CREATE_INSTANCE_ON_THE_PIPE_CLIENT_END_FAILED
Language=English
Call CreateFile to create instance on the pipe client end failed.
.
Language=Chinese

调用CreateFile以创建命名管道客户端失败
.

MessageId=0x6
Severity=Error
Facility=KxEPipe
SymbolicName=E_KXEPIPE_DISCONNCET_NAMED_PIPE_FAILED
Language=English
The server end of the named pipe disconncet the pipe failed.
.
Language=Chinese

命名管道服务端调用DisconncetNamedPipe以关闭管道时失败
.

MessageId=0x7
Severity=Error
Facility=KxEPipe
SymbolicName=E_KXEPIPE_NAMED_PIPE_NOT_START
Language=English
The named pipe is not started as the user have not call function Start.
.
Language=Chinese

命名管道没有启动，因为用户没有调用Start函数
.

MessageId=0x8
Severity=Error
Facility=KxEPipe
SymbolicName=E_KXEPIPE_SERVER_WRITE_FILE_FAILED
Language=English
The server end write into the named pipe failed.
.
Language=Chinese

服务端向命名管道中写入数据失败
.

MessageId=0x9
Severity=Error
Facility=KxEPipe
SymbolicName=E_KXEPIPE_CLIENT_WRITE_FILE_FAILED
Language=English
The client end write into the named pipe failed.
.
Language=Chinese

客户端向命名管道中写入数据失败
.

MessageId=0xa
Severity=Error
Facility=KxEPipe
SymbolicName=E_KXEPIPE_SERVER_READ_FILE_FAILED
Language=English
The server end read from the named pipe failed.
.
Language=Chinese

服务端从命名管道中读取数据失败
.

MessageId=0xb
Severity=Error
Facility=KxEPipe
SymbolicName=E_KXEPIPE_CLIENT_READ_FILE_FAILED
Language=English
The client end read from the named pipe failed.
.
Language=Chinese

客户端从命名管道中读取数据失败
.

MessageId=0xc
Severity=Error
Facility=KxEPipe
SymbolicName=E_KXEPIPE_PRODUCT_ALREADY_REGISTERED
Language=English
The product has been already registered.
.
Language=Chinese

子产品ID已经注册
.

MessageId=0xd
Severity=Error
Facility=KxEPipe
SymbolicName=E_KXEPIPE_INCORRECT_PARAMETER
Language=English
Incorrect parameter(s) passed by caller or delivered in the named pipe.
.
Language=Chinese

调用者提供了不正确的参数或者通过命名管道传递来的数据不正确
.

MessageId=0xe
Severity=Error
Facility=KxEPipe
SymbolicName=E_KXEPIPE_THREAD_COMPELLED_TO_STOP
Language=English
Thread is compelled to stop because the user has stopped the pipe.
.
Language=Chinese

用户关闭了管道导致线程被迫终止
.

MessageId=0xf
Severity=Error
Facility=KxEPipe
SymbolicName=E_KXEPIPE_WAIT_FOR_THREAD_EXIT_FAILED
Language=English
Wait for all threads created by the pipe failed.
.
Language=Chinese

等待所有线程结束的操作失败
.

MessageId=0x10
Severity=Error
Facility=KxEPipe
SymbolicName=E_KXEPIPE_BAD_RETURN_DATA
Language=English
Data returned from the server end or the client end is incorrect.
.
Language=Chinese

从服务端或客户端返回的数据有误
.

MessageId=0x11
Severity=Error
Facility=KxEPipe
SymbolicName=E_KXEPIPE_SERVICE_PROVIDER_ALREADY_REGISTERED
Language=English
The service provider has been already registered.
.
Language=Chinese

服务提供者已经注册
.

MessageId=0x12
Severity=Error
Facility=KxEPipe
SymbolicName=E_KXEPIPE_SERVICE_PROVIDER_NOT_REGISTERED
Language=English
Service provider not registered on the subproduct end.
.
Language=Chinese

子产品端没有注册服务提供者
.

MessageId=0x13
Severity=Error
Facility=KxEPipe
SymbolicName=E_KXEPIPE_PRODUCT_NOT_REGISTERED
Language=English
Product ID has not been registered.
.
Language=Chinese

子产品ID尚未注册
.

MessageId=0x14
Severity=Error
Facility=KxEPipe
SymbolicName=E_KXEPIPE_IMPERSONATE_CLIENT_FAILED
Language=English
The server end of the named pipe impersonate the client end failed.
.
Language=Chinese

命名管道服务端模拟客户端失败
.

MessageId=0x15
Severity=Error
Facility=KxEPipe
SymbolicName=E_KXEPIPE_REVERT_TO_SELF_FAILED
Language=English
The server end of the named pipe revert to self failed.
.
Language=Chinese

命名管道服务端模拟客户端后恢复原状态时失败
.

MessageId=0x16
Severity=Error
Facility=KxEPipe
SymbolicName=E_KXEPIPE_SP_PROCESS_REQUEST_FAILED
Language=English
The service provider failed to process request.
.
Language=Chinese

服务提供者处理请求失败
.

MessageId=0x17
Severity=Error
Facility=KxEPipe
SymbolicName=E_KXEPIPE_CREATE_IO_COMPLETION_PORT_FAILED
Language=English
Create I/O completion port or associate an I/O port to a pipe handle failed.
.
Language=Chinese

创建I/O完成端口失败，或者将完成端口与某个管道句柄关联时失败
.

MessageId=0x18
Severity=Error
Facility=KxEPipe
SymbolicName=E_KXEPIPE_CREATE_EVENT_FAILED
Language=English
Create event failed.
.
Language=Chinese

创建事件对象失败
.

MessageId=0x19
Severity=Error
Facility=KxEPipe
SymbolicName=E_KXEPIPE_NOT_ALL_THREADS_ARE_NOTIFIED_TO_STOP
Language=English
Not all of worker threads are notified to stop, this is an fatal error.
.
Language=Chinese

不是所有的工作线程都被通知终止，这是一个致命错误
.

MessageId=0x1a
Severity=Error
Facility=KxEPipe
SymbolicName=E_KXEPIPE_PIPE_ALREADY_STARTED
Language=English
kxepipe component has been already started.
.
Language=Chinese

kxepipe组件已经启动
.

MessageId=0x1b
Severity=Error
Facility=KxEPipe
SymbolicName=E_KXEPIPE_SET_SECURITY_INFO_FAILED
Language=English
Call API SetSecurityInfo failed after the the pipe instance been created.
.
Language=Chinese

创建命名管道实例后调用API SetSecurityInfo 时失败
.

MessageId=0x1c
Severity=Error
Facility=KxEPipe
SymbolicName=E_KXEPIPE_GET_SECURITY_DESCRIPTOR_SACL_FAILED
Language=English
Call API GetSecurityDescriptorSacl failed after the the pipe instance been created.
.
Language=Chinese

创建命名管道实例后调用API GetSecurityDescriptorSacl 时失败
.

MessageId=0x1d
Severity=Error
Facility=KxEPipe
SymbolicName=E_KXEPIPE_CONVERT_STRING_TO_SECURITY_DESCRIPTOR_FAILED
Language=English
Call API ConvertStringSecurityDescriptorToSecurityDescriptorW failed after the the pipe instance been created.
.
Language=Chinese

创建命名管道实例后调用API ConvertStringSecurityDescriptorToSecurityDescriptorW 时失败
.

MessageId=0x1e
Severity=Error
Facility=KxEPipe
SymbolicName=E_KXEPIPE_NOT_ENOUGH_MEMORY
Language=English
Call new operator to allocate memory failed.
.
Language=Chinese

调用 new 操作符分配内存失败
.

MessageId=0x1f
Severity=Error
Facility=KxEPipe
SymbolicName=E_KXEPIPE_SP_PIPE_NAME_NOT_FOUND_IN_REGISTRY
Language=English
The specified Service Provider's pipe name was not found in registry.
.
Language=Chinese

在注册表中没有找到指定服务提供者的命名管道名
.

; // KxEIPC
MessageId=0x1
Severity=Error
Facility=KxEIPC
SymbolicName=E_KXEIPC_SP_NOT_FOUND
Language=English
The service provider is not fouend.
.
Language=Chinese

服务提供者没有找到
.

MessageId=0x2
Severity=Error
Facility=KxEIPC
SymbolicName=E_KXEIPC_MANAGE_SERVER_ALREADY_INITIALIZED
Language=English
The manage server has been already initialized.
.
Language=Chinese

IPC管理服务器已经初始化
.

MessageId=0x3
Severity=Error
Facility=KxEIPC
SymbolicName=E_KXEIPC_MANAGE_SERVER_NOT_INITIALIZED
Language=English
The manage server has not been initialized yet.
.
Language=Chinese

IPC管理服务器尚未初始化
.

MessageId=0x4
Severity=Error
Facility=KxEIPC
SymbolicName=E_KXEIPC_MANAGE_SERVER_ALREADY_STARTED
Language=English
The manage server has been already started.
.
Language=Chinese

IPC管理服务器已经启动
.

MessageId=0x5
Severity=Error
Facility=KxEIPC
SymbolicName=E_KXEIPC_MANAGE_SERVER_NOT_STARTED
Language=English
The manage server has not been started yet.
.
Language=Chinese

IPC管理服务器尚未启动
.

MessageId=0x6
Severity=Error
Facility=KxEIPC
SymbolicName=E_KXEIPC_MANAGE_CLIENT_ALREADY_INITIALIZED
Language=English
The manage client has been already initialized.
.
Language=Chinese

IPC管理客户端已经初始化
.

MessageId=0x7
Severity=Error
Facility=KxEIPC
SymbolicName=E_KXEIPC_MANAGE_CLIENT_NOT_INITIALIZED
Language=English
The manage client has not been initialized yet.
.
Language=Chinese

IPC管理客户端尚未初始化
.

MessageId=0x8
Severity=Error
Facility=KxEIPC
SymbolicName=E_KXEIPC_MANAGE_CLIENT_ALREADY_STARTED
Language=English
The manage client has been already started.
.
Language=Chinese

IPC管理客户端已经启动
.

MessageId=0x9
Severity=Error
Facility=KxEIPC
SymbolicName=E_KXEIPC_MANAGE_CLIENT_NOT_STARTED
Language=English
The manage client has not been started yet.
.
Language=Chinese

IPC管理客户端尚未启动
.

MessageId=0xa
Severity=Error
Facility=KxEIPC
SymbolicName=E_KXEIPC_NOT_ALL_SERVICE_PROVIDER_REGISTERED
Language=English
Not all of the service provider of the client reregistered to the server successfully.
.
Language=Chinese

不是所有的客户端服务提供者都成功地向服务框架端重新注册
.

MessageId=0xb
Severity=Error
Facility=KxEIPC
SymbolicName=E_KXEIPC_PIPE_SERVER_DOES_NOT_EXIST
Language=English
The given named pipe server doesn't exist .
.
Language=Chinese

指定的命名管道服务器不存在
.

MessageId=0xc
Severity=Error
Facility=KxEIPC
SymbolicName=E_KXEIPC_NOT_ENOUGH_MEMORY
Language=English
Call the new operator to allocate memory failed.
.
Language=Chinese

调用 new 操作符分配内存失败
.

MessageId=0xd
Severity=Error
Facility=KxEIPC
SymbolicName=E_KXEIPC_CREATE_SP_PIPE_INFO_KEY_FAILED
Language=English
It failed to create the registry key for the specified SP's pipe information.
.
Language=Chinese

在注册表中创建指定SP的管道信息失败
.

MessageId=0xe
Severity=Error
Facility=KxEIPC
SymbolicName=E_KXEIPC_SET_SP_PIPE_NAME_KEY_FAILED
Language=English
It failed to set the specified SP's pipe name key in the registry.
.
Language=Chinese

在注册表中设置指定SP的管道名键值失败
.

MessageId=0xf
Severity=Error
Facility=KxEIPC
SymbolicName=E_KXEIPC_DELETE_SP_PIPE_INFO_KEY_FAILED
Language=English
It failed to delete the specified SP's pipe information in the registry.
.
Language=Chinese

在注册表中删除指定SP的管道信息失败
.

MessageId=0x10
Severity=Error
Facility=KxEIPC
SymbolicName=E_KXEIPC_ILLEGAL_URL
Language=English
The request caller's URL is illegal.
.
Language=Chinese

调用者的URL不合法
.

MessageId=0x11
Severity=Error
Facility=KxEIPC
SymbolicName=E_KXEIPC_READ_INI_FILE_FAILED
Language=English
 It failed to read the .ini filewhich stores legal websites.
.
Language=Chinese

读取保存合法链接的配置文件失败
.

; // KxEServiceMgr
MessageId=0x1
Severity=Error
Facility=KxEServiceMgr
SymbolicName=E_KXESERVICEMGR_NOT_INITIALIZED
Language=English
The service manager is not initialized.
.
Language=Chinese

服务管理器还未初始化
.

MessageId=0x2
Severity=Error
Facility=KxEServiceMgr
SymbolicName=E_KXESERVICEMGR_INITIALIZED
Language=English
The service manager is initialized.
.
Language=Chinese

服务管理器已经初始化
.

MessageId=0x3
Severity=Error
Facility=KxEServiceMgr
SymbolicName=E_KXESERVICEMGR_PRODUCTMGR_NOT_INITIALIZED
Language=English
The product manager is not initialized.
.
Language=Chinese

产品管理器还未初始化
.

MessageId=0x4
Severity=Error
Facility=KxEServiceMgr
SymbolicName=E_KXESERVICEMGR_PRODUCTMGR_INITIALIZED
Language=English
The product manager is initialized.
.
Language=Chinese

产品管理器已经初始化
.

MessageId=0x5
Severity=Error
Facility=KxEServiceMgr
SymbolicName=E_KXESERVICEMGR_PRODUCTMGR_PRODUCT_EXIST
Language=English
The product is exist.
.
Language=Chinese

指定的产品已经存在
.

MessageId=0x6
Severity=Error
Facility=KxEServiceMgr
SymbolicName=E_KXESERVICEMGR_PRODUCTMGR_PRODUCT_NOT_EXIST
Language=English
The product is not exist.
.
Language=Chinese

指定的产品不存在
.

MessageId=0x7
Severity=Error
Facility=KxEServiceMgr
SymbolicName=E_KXESERVICEMGR_PRODUCTMGR_CLSID_DUPLICATE
Language=English
The component's clsid is duplicate
.
Language=Chinese

指定的组件CLSID已经存在
.

MessageId=0x8
Severity=Error
Facility=KxEServiceMgr
SymbolicName=E_KXESERVICEMGR_PRODUCTMGR_PATH_INVALID
Language=English
The product's path is invalid
.
Language=Chinese

指定的产品目录无效
.

MessageId=0x9
Severity=Error
Facility=KxEServiceMgr
SymbolicName=E_KXESERVICEMGR_PRODUCTMGR_PLUGIN_PATH_INVALID
Language=English
The product's plugin path is invalid
.
Language=Chinese

指定的产品plugin目录无效
.

MessageId=0xa
Severity=Error
Facility=KxEServiceMgr
SymbolicName=E_KXESERVICEMGR_PRODUCTMGR_WEB_PATH_INVALID
Language=English
The product's web path is invalid
.
Language=Chinese

指定的产品web目录无效
.

MessageId=0xb
Severity=Error
Facility=KxEServiceMgr
SymbolicName=E_KXESERVICEMGR_PRODUCTMGR_SP_NOT_COMPONENT
Language=English
The service provider file is not in component file.
.
Language=Chinese

指定的服务提供者文件不是一个SCOM组件
.

MessageId=0xc
Severity=Error
Facility=KxEServiceMgr
SymbolicName=E_KXESERVICEMGR_PRODUCTMGR_SP_CLSID_NOT_COMPONENT
Language=English
The service provider clsid is not in component clsid.
.
Language=Chinese

指定的服务提供者clsid不在scom组件中
.

MessageId=0xd
Severity=Error
Facility=KxEServiceMgr
SymbolicName=E_KXESERVICEMGR_PRODUCTMGR_SP_CLSID_DUPLICATE
Language=English
The service provider component's clsid is duplicate
.
Language=Chinese

指定的服务提供者组件CLSID已经存在
.

MessageId=0xe
Severity=Error
Facility=KxEServiceMgr
SymbolicName=E_KXESERVICEMGR_ALL_SP_FAIL_IN_PRODUCT
Language=English
The all service providers in a product is failed.
.
Language=Chinese

指定产品的所有SP加载失败.
.

MessageId=0xf
Severity=Error
Facility=KxEServiceMgr
SymbolicName=E_KXESERVICEMGR_PRODUCT_NOT_RUNNING
Language=English
The product is not running.
.
Language=Chinese

指定的产品没有在运行中.
.

MessageId=0x10
Severity=Error
Facility=KxEServiceMgr
SymbolicName=E_KXESERVICEMGR_PRODUCT_RUNNING
Language=English
The product is running.
.
Language=Chinese

指定的产品在运行中.
.

MessageId=0x11
Severity=Error
Facility=KxEServiceMgr
SymbolicName=E_KXESERVICEMGR_GUID_INVALID
Language=English
The guid is invalid.
.
Language=Chinese

guid是无效的.
.

MessageId=0x12
Severity=Error
Facility=KxEServiceMgr
SymbolicName=E_KXESERVICEMGR_FILE_PATH_INVALID
Language=English
The file path is invalid.
.
Language=Chinese

文件路径是无效的.
.

MessageId=0x13
Severity=Error
Facility=KxEServiceMgr
SymbolicName=E_KXESERVICEMGR_COMPONENT_NOT_EQUAL
Language=English
The component file in config is not equal in count
.
Language=Chinese

读取到组件的文件数目与配置文件中标志数目不一致
.

MessageId=0x14
Severity=Error
Facility=KxEServiceMgr
SymbolicName=E_KXESERVICEMGR_SP_COMPONENT_NOT_EQUAL
Language=English
The sp component file in config is not equal in count
.
Language=Chinese

读取到服务提供者组件的文件数目与配置文件中标志数目不一致
.

MessageId=0x15
Severity=Error
Facility=KxEServiceMgr
SymbolicName=E_KXESERVICEMGR_CHILD_PRODUCT_NOT_EQUAL
Language=English
The child product in config is not equal in count
.
Language=Chinese

读取到子产品数目与配置文件中标志数目不一致
.

MessageId=0x16
Severity=Error
Facility=KxEServiceMgr
SymbolicName=E_KXESERVICEMGR_TASK_EXCEED_MAX
Language=English
The task is exceed the max number.
.
Language=Chinese

任务数量超过了最大值
.

MessageId=0x17
Severity=Error
Facility=KxEServiceMgr
SymbolicName=E_KXESERVICEMGR_TASK_NOT_FOUND
Language=English
The task is not found.
.
Language=Chinese

指定的任务没有找到.
.

MessageId=0x18
Severity=Error
Facility=KxEServiceMgr
SymbolicName=E_KXESERVICEMGR_TASK_CAN_NOT_CLOSE
Language=English
The task can't be closed.
.
Language=Chinese

指定的任务不能被关闭
.

MessageId=0x19
Severity=Error
Facility=KxEServiceMgr
SymbolicName=E_KXESERVICEMGR_PROCESS_DETACHED
Language=English
The process is detached.
.
Language=Chinese

指定的进程detached.
.

; // KxEPassport
MessageId=0x1
Severity=Error
Facility=KxEPassport
SymbolicName=E_KXEPASSPORT_USERNAME_NOT_ASCII
Language=English
The user name is not ascii.
.
Language=Chinese

用户名中包含非ascii码.
.

MessageId=0x2
Severity=Error
Facility=KxEPassport
SymbolicName=E_KXEPASSPORT_PPWIZ_IS_RUNNING
Language=English
The Ppwizard is running.
.
Language=Chinese

PPWIZARD正在运行中.
.

MessageId=0x3
Severity=Error
Facility=KxEPassport
SymbolicName=E_KXEPASSPORT_PPWIZ_NOT_UP_TO_SNUFF
Language=English
The scene is not up to snuff.
.
Language=Chinese

不符合调起ppwizard的条件.
.

MessageId=0x4
Severity=Error
Facility=KxEPassport
SymbolicName=E_KXEPASSPORT_SP_NOT_INITIALIZED
Language=English
The Passport service provider has not been initialized.
.
Language=Chinese

Passport SP 未被初始化.
.

MessageId=0x5
Severity=Error
Facility=KxEPassport
SymbolicName=E_KXEPASSPORT_NO_PPWIZ_WINDOW_TO_SHOW
Language=English
There is no ppwizard window to show.
.
Language=Chinese

没有可前置的ppwizard页面.
.

; // KxERegister
MessageId=0x1
Severity=Error
Facility=KxERegister
SymbolicName=E_KXEREG_FAIL_LOAD_CONFIG
Language=English
It failed to load config file.
.
Language=Chinese

加载配置文件出错.
.

MessageId=0x2
Severity=Error
Facility=KxERegister
SymbolicName=E_KXEREG_ZERO_FILE
Language=English
The file list is empty.
.
Language=Chinese

文件列表是空的
.

; // KxEIdl
MessageId=0x1
Severity=Error
Facility=KxEIdl
SymbolicName=E_KXEIDL_XML_FAIL_TO_LOAD_FILE
Language=English
It failed to load xml file
.
Language=Chinese

加载XML文件出错.
.

MessageId=0x2
Severity=Error
Facility=KxEIdl
SymbolicName=E_KXEIDL_XML_FAIL_TO_LOAD_TEXT
Language=English
It failed to load xml text.
.
Language=Chinese

加载XML内容出错.
.

MessageId=0x3
Severity=Error
Facility=KxEIdl
SymbolicName=E_KXEIDL_XML_FAIL_TO_LOAD_ROOT
Language=English
It failed to load xml root
.
Language=Chinese

加载XML中的root出错.
.

MessageId=0x4
Severity=Error
Facility=KxEIdl
SymbolicName=E_KXEIDL_XML_FAIL_TO_SAVE_FILE
Language=English
It failed to save xml file
.
Language=Chinese

保存XML文件出错.
.

MessageId=0x5
Severity=Error
Facility=KxEIdl
SymbolicName=E_KXEIDL_XML_FAIL_TO_GEN_TEXT
Language=English
It failed to gen text in xml.
.
Language=Chinese

生成xml内容时出错.
.

MessageId=0x6
Severity=Error
Facility=KxEIdl
SymbolicName=E_KXEIDL_XML_PARENT_NODE_IS_NULL
Language=English
The parent node is null in xml.
.
Language=Chinese

在xml中，父接点为null
.

MessageId=0x7
Severity=Error
Facility=KxEIdl
SymbolicName=E_KXEIDL_XML_CURRENT_NODE_IS_NULL
Language=English
The current node is null in xml.
.
Language=Chinese

在xml中，当前接点为null
.

MessageId=0x8
Severity=Error
Facility=KxEIdl
SymbolicName=E_KXEIDL_XML_FAIL_TO_REMOVE_NODE
Language=English
It failed to remove node in xml.
.
Language=Chinese

在xml中，移除接点失败.
.

MessageId=0x9
Severity=Error
Facility=KxEIdl
SymbolicName=E_KXEIDL_XML_FAIL_TO_GET_NODE_NAME
Language=English
It failed to get node name in xml.
.
Language=Chinese

在xml中，获取当前接点名字失败.
.

MessageId=0xa
Severity=Error
Facility=KxEIdl
SymbolicName=E_KXEIDL_XML_FAIL_TO_GET_NODE_TEXT
Language=English
It failed to get node text in xml.
.
Language=Chinese

在xml中，获取当前接点内容失败.
.

MessageId=0xb
Severity=Error
Facility=KxEIdl
SymbolicName=E_KXEIDL_XML_FAIL_TO_SET_NODE_TEXT
Language=English
It failed to set node text in xml.
.
Language=Chinese

在xml中，设置当前接点内容失败.
.

MessageId=0xc
Severity=Error
Facility=KxEIdl
SymbolicName=E_KXEIDL_XML_VALUE_STREAM_ERROR
Language=English
The stream is error in xml.
.
Language=Chinese

处理xml时，stream出错
.

MessageId=0xd
Severity=Error
Facility=KxEIdl
SymbolicName=E_KXEIDL_XML_XMLMANAGER_EXPECTED
Language=English
The xmlmanger is null.
.
Language=Chinese

xmlmanager没有初始化.
.

MessageId=0xe
Severity=Error
Facility=KxEIdl
SymbolicName=E_KXEIDL_XML_SCOPE_STACK_IS_EMPTY
Language=English
The scope stack is empty in xml.
.
Language=Chinese

处理xml时，scope stack为空.
.

MessageId=0xf
Severity=Error
Facility=KxEIdl
SymbolicName=E_KXEIDL_XML_FAIL_TO_INIT_CURSOR
Language=English
It failed to init cursor in xml.
.
Language=Chinese

处理xml时，初始化cursor失败.
.

MessageId=0x10
Severity=Error
Facility=KxEIdl
SymbolicName=E_KXEIDL_XML_FAIL_TO_LOAD_DEST_DOM_ROOT
Language=English
It failed to load dest dom's root in xml.
.
Language=Chinese

处理xml时，加载目标dom's root失败.
.

MessageId=0x11
Severity=Error
Facility=KxEIdl
SymbolicName=E_KXEIDL_XML_FAIL_TO_SET_DEST_DOM_VALUE
Language=English
It failed to set dest dom's value in xml.
.
Language=Chinese

处理xml时，设定目标dom's value失败.
.

MessageId=0x12
Severity=Error
Facility=KxEIdl
SymbolicName=E_KXEIDL_XML_FAIL_TO_COPY_NODE_TO_DEST_DOM
Language=English
It failed to copy node to dest dom in xml.
.
Language=Chinese

处理xml时，复制当前接点到dest dom失败.
.

MessageId=0x13
Severity=Error
Facility=KxEIdl
SymbolicName=E_KXEIDL_XML_INVALID_CURSOR_STATE
Language=English
the cursor is in invalid state in xml.
.
Language=Chinese

处理xml时，cursor的状态不正确.
.


MessageId=0x20
Severity=Error
Facility=KxEIdl
SymbolicName=E_KXEIDL_JSON_FAIL_TO_LOAD_FILE
Language=English
It failed to load json file
.
Language=Chinese

加载json文件出错.
.

MessageId=0x21
Severity=Error
Facility=KxEIdl
SymbolicName=E_KXEIDL_JSON_FAIL_TO_LOAD_TEXT
Language=English
It failed to load json text.
.
Language=Chinese

加载json内容出错.
.

MessageId=0x22
Severity=Error
Facility=KxEIdl
SymbolicName=E_KXEIDL_JSON_FAIL_TO_LOAD_ROOT
Language=English
It failed to load json's root
.
Language=Chinese

加载json中的root出错.
.

MessageId=0x23
Severity=Error
Facility=KxEIdl
SymbolicName=E_KXEIDL_JSON_FAIL_TO_SAVE_FILE
Language=English
It failed to save json file
.
Language=Chinese

保存json文件出错.
.

MessageId=0x24
Severity=Error
Facility=KxEIdl
SymbolicName=E_KXEIDL_JSON_PARENT_NODE_IS_NULL
Language=English
The parent node is null in json.
.
Language=Chinese

在json中，父接点为null
.

MessageId=0x25
Severity=Error
Facility=KxEIdl
SymbolicName=E_KXEIDL_JSON_CURRENT_NODE_IS_NULL
Language=English
The current node is null in json.
.
Language=Chinese

在json中，当前接点为null
.

MessageId=0x26
Severity=Error
Facility=KxEIdl
SymbolicName=E_KXEIDL_JSON_ROOT_NODE_IS_NULL
Language=English
The root node is null in json.
.
Language=Chinese

在json中，root接点为空.
.

MessageId=0x27
Severity=Error
Facility=KxEIdl
SymbolicName=E_KXEIDL_JSON_EXPECT_STRING_TYPE
Language=English
It expected the string type in json.
.
Language=Chinese

在json中，需要字符串类型.
.

MessageId=0x28
Severity=Error
Facility=KxEIdl
SymbolicName=E_KXEIDL_JSON_VALUE_STREAM_ERROR
Language=English
The stream is error in json.
.
Language=Chinese

处理json时，stream出错
.

MessageId=0x29
Severity=Error
Facility=KxEIdl
SymbolicName=E_KXEIDL_JSON_FAIL_TO_GET_NODE_NAME
Language=English
It failed to get node name in json.
.
Language=Chinese

在json中，获取当前接点名字失败.
.

MessageId=0x2a
Severity=Error
Facility=KxEIdl
SymbolicName=E_KXEIDL_XML_FAIL_TO_GET_ROOT_NAME
Language=English
It failed to get root name in json.
.
Language=Chinese

在json中，获取根接点名字失败.
.

MessageId=0x2b
Severity=Error
Facility=KxEIdl
SymbolicName=E_KXEIDL_JSON_FAIL_TO_LOAD_DEST_DOM_ROOT
Language=English
It failed to load dest dom's root in json.
.
Language=Chinese

处理json时，加载目标dom's root失败.
.

MessageId=0x2c
Severity=Error
Facility=KxEIdl
SymbolicName=E_KXEIDL_JSON_FAIL_TO_INIT_CURSOR
Language=English
It failed to init cursor in json.
.
Language=Chinese

处理json时，初始化cursor失败.
.

MessageId=0x2d
Severity=Error
Facility=KxEIdl
SymbolicName=E_KXEIDL_JSON_FAIL_TO_COPY_NODE_TO_DEST_DOM
Language=English
It failed to copy node to dest dom in json.
.
Language=Chinese

处理json时，复制当前接点到dest dom失败.
.

MessageId=0x2e
Severity=Error
Facility=KxEIdl
SymbolicName=E_KXEIDL_JSON_SCOPE_STACK_IS_EMPTY
Language=English
The scope stack is empty in json.
.
Language=Chinese

处理json时，scope stack为空.
.

MessageId=0x40
Severity=Error
Facility=KxEIdl
SymbolicName=E_KXEIDL_KDOM_DATA_OUT_OF_RANGE
Language=English
The dom data is out of range.
.
Language=Chinese

dom data的内容超出限定长度.
.

MessageId=0x41
Severity=Error
Facility=KxEIdl
SymbolicName=E_KXEIDL_KDOM_DATA_TYPE_DO_NOT_SUPPORT_WRITING
Language=English
The data type is not support to write in dom.
.
Language=Chinese

在dom中指定的数据类型不支持写入
.


; // KxEBase

MessageId=0x1
Severity=Error
Facility=KxEBase
SymbolicName=E_KXEBASE_PLUGIN_NOT_FOUND
Language=English
It failed to find the plugin.
.
Language=Chinese

没有找到对应的插件SDK.
.

MessageId=0x2
Severity=Error
Facility=KxEBase
SymbolicName=E_KXEBASE_PLUGIN_FUNCTION_NOT_FOUND
Language=English
It failed to find the function in the plugin.
.
Language=Chinese

在指定的plugin上没有找到对应的函数.
.

MessageId=0x3
Severity=Error
Facility=KxEBase
SymbolicName=E_KXEBASE_CFG_NOT_INIT
Language=English
The config componenet wasn't initialied.
.
Language=Chinese

配置组件还没有被初始化.
.

MessageId=0x4
Severity=Error
Facility=KxEBase
SymbolicName=E_KXEBASE_EVENT_NOT_INIT
Language=English
The event componenet wasn't initialied.
.
Language=Chinese

事件组件还没有被初始化.
.

MessageId=0x5
Severity=Error
Facility=KxEBase
SymbolicName=E_KXEBASE_HTTP_NOT_INIT
Language=English
The HTTP componenet wasn't initialied.
.
Language=Chinese

HTTP组件还没有被初始化.
.

MessageId=0x6
Severity=Error
Facility=KxEBase
SymbolicName=E_KXEBASE_PLUGIN_NOT_INIT
Language=English
The PLUGIN componenet wasn't initialied.
.
Language=Chinese

Plugin组件还没有被初始化.
.

MessageId=0x7
Severity=Error
Facility=KxEBase
SymbolicName=E_KXEBASE_INVALID_PARAMETER
Language=English
The parameter is invalid.
.
Language=Chinese

参数无效.
.

MessageId=0x8
Severity=Error
Facility=KxEBase
SymbolicName=E_KXEBASE_THREAD_NOT_INIT
Language=English
The thread component wasn't initialized.
.
Language=Chinese

线程管理组件没有初始化.
.

MessageId=0x9
Severity=Error
Facility=KxEBase
SymbolicName=E_KXEBASE_NOT_ENOUGH_MEMORY
Language=English
It failed to allocate memory.
.
Language=Chinese

没有足够的内存分配.
.

MessageId=0xa
Severity=Error
Facility=KxEBase
SymbolicName=E_KXEBASE_SERVICECALL_NOT_INIT
Language=English
The service call component wasn't initialized.
.
Language=Chinese

Service Call组件没有初始化.
.

MessageId=0xb
Severity=Error
Facility=KxEBase
SymbolicName=E_KXEBASE_SYSTEM_NOT_SUPPORT
Language=English
It can't support the system.
.
Language=Chinese

该系统不支持.
.

MessageId=0xc
Severity=Error
Facility=KxEBase
SymbolicName=E_KXEBASE_PLUGIN_PATH_LOADED
Language=English
The plugin path has been loaded.
.
Language=Chinese

指定的plugin path已经加载过.
.

MessageId=0xd
Severity=Error
Facility=KxEBase
SymbolicName=E_KXEBASE_PLUGIN_PATH_NOT_LOADED
Language=English
The plugin path doesn't have been loaded.
.
Language=Chinese

指定的plugin路径没有加载
.

MessageId=0xe
Severity=Error
Facility=KxEBase
SymbolicName=E_KXEBASE_ACTIVITY_NOT_INIT
Language=English
The activity stat component is not initialized.
.
Language=Chinese

活跃度统计组件没有被初始化
.

MessageId=0xf
Severity=Error
Facility=KxEBase
SymbolicName=E_KXEBASE_ACTIVITY_INITED
Language=English
The activity stat component is initialized.
.
Language=Chinese

活跃度统计组件已经被初始化
.

MessageId=0x10
Severity=Error
Facility=KxEBase
SymbolicName=E_KXEBASE_FUNCTION_NOT_IMPLEMENTED
Language=English
The function is not implemented.
.
Language=Chinese

指定的函数没有实现
.

MessageId=0x11
Severity=Error
Facility=KxEBase
SymbolicName=E_KXEBASE_SERVICE_CAN_NOT_UNINSTALL
Language=English
The service can't be uninstalled.
.
Language=Chinese

服务还不能被卸载
.

MessageId=0x12
Severity=Error
Facility=KxEBase
SymbolicName=E_KXEBASE_INITED
Language=English
The kxebase is initied.
.
Language=Chinese

kxebase已经被初始化
.

MessageId=0x13
Severity=Error
Facility=KxEBase
SymbolicName=E_KXEBASE_NOT_INITED
Language=English
The kxebase is not initied.
.
Language=Chinese

kxebase未被初始化.
.

MessageId=0x14
Severity=Error
Facility=KxEBase
SymbolicName=E_KXEBASE_SERVICE_CAN_NOT_STOPPED
Language=English
The service can't be stopped.
.
Language=Chinese

服务还不能被停止
.

MessageId=0x15
Severity=Error
Facility=KxEBase
SymbolicName=E_KXEBASE_ALL_SP_INITIALIZE_FAILED
Language=English
The service must terminate because none of the SPs was initialized successfully.
.
Language=Chinese

所有 SP 都没有初始化成功导致服务必须终止
.

MessageId=0x16
Severity=Error
Facility=KxEBase
SymbolicName=E_KXEBASE_ALL_SP_START_FAILED
Language=English
No service provider in the provider list started successfully.
.
Language=Chinese

所有 SP 都没有启动成功导致服务必须终止
.

MessageId=0x17
Severity=Error
Facility=KxEBase
SymbolicName=E_KXEBASE_DLL_PATH_NOT_ACCQUIRED_FROM_CONFIGURATION_FILE
Language=English
It failed to accquire kxebase.dll's registry path from the configuration file 'kxecommon.dat'.
.
Language=Chinese

从配置文件 kxecommon.dat 中读取 kxebase.dll 的注册表路径失败
.

MessageId=0x18
Severity=Error
Facility=KxEBase
SymbolicName=E_KXEBASE_START_SERVICE_STOPPED
Language=English
Service auto stop when start service.
.
Language=Chinese

启动服务时服务自动终止 
.

; // KxECfg

MessageId=0x1
Severity=Error
Facility=KxECfg
SymbolicName=E_KXECFG_VALUE_NOT_CORRECT
Language=English
The value in registry is not correct.
.
Language=Chinese

注册表中的value值不正确
.

MessageId=0x2
Severity=Error
Facility=KxECfg
SymbolicName=E_KXECFG_KXENGINE_CONFIG_PATH_NOT_ACCQUIRED_FROM_CONFIGURATION_FILE
Language=English
It failed to accquire KXENGINE's registry path from the configuration file 'kxecommon.dat'.
.
Language=Chinese

从配置文件 kxecommon.dat 中读取大引擎配置起始路径(即 KXENGINE )的注册表路径失败
.

; // KxEThread

MessageId=0x1
Severity=Error
Facility=KxEThread
SymbolicName=E_KXETHREAD_INVALID_HANDLE
Language=English
The job handle is invalid.
.
Language=Chinese

传入的Job句柄无效.
.

MessageId=0x2
Severity=Error
Facility=KxEThread
SymbolicName=E_KXETHREAD_JOB_FINISHED
Language=English
The target job is finished.
.
Language=Chinese

指定的Job已经运行完成.
.

MessageId=0x3
Severity=Error
Facility=KxEThread
SymbolicName=E_KXETHREAD_JOB_READY
Language=English
The target job is ready.
.
Language=Chinese

指定的Job已经准备好.
.

MessageId=0x4
Severity=Error
Facility=KxEThread
SymbolicName=E_KXETHREAD_JOB_RUNNING
Language=English
The job is running.
.
Language=Chinese

指定的Job正在运行.
.

MessageId=0x5
Severity=Error
Facility=KxEThread
SymbolicName=E_KXETHREAD_JOB_ERROR
Language=English
The target job is in error state.
.
Language=Chinese

指令的Job已经出错,无法运行.
.

MessageId=0x6
Severity=Error
Facility=KxEThread
SymbolicName=E_KXETHREAD_JOB_WAIT_NUMBER_EXCEED
Language=English
The number of job to wait is exceed 64.
.
Language=Chinese

需要等待的Job数量超过了64个
.

MessageId=0x7
Severity=Error
Facility=KxEThread
SymbolicName=E_KXETHREAD_JOB_CLOSED
Language=English
The job is closed.
.
Language=Chinese

指定的Job已经关闭.
.

MessageId=0x8
Severity=Error
Facility=KxEThread
SymbolicName=E_KXETHREAD_JOB_NOT_FINISHED
Language=English
The target job is not finished.
.
Language=Chinese

指定的Job还没有运行完成.
.

MessageId=0x9
Severity=Error
Facility=KxEThread
SymbolicName=E_KXETHREAD_JOB_NOT_READY
Language=English
The target job is not ready.
.
Language=Chinese

指定的Job未准备好.
.

MessageId=0xa
Severity=Error
Facility=KxEThread
SymbolicName=E_KXETHREAD_JOB_NOT_RUNNING
Language=English
The job is not running.
.
Language=Chinese

指定的Job没有运行.
.

MessageId=0xb
Severity=Error
Facility=KxEThread
SymbolicName=E_KXETHREAD_JOB_IN_STOP_CMD
Language=English
The job has recved the stop command.
.
Language=Chinese

已经向指定的Job发送了停止命令
.

MessageId=0xc
Severity=Error
Facility=KxEThread
SymbolicName=E_KXETHREAD_JOB_IN_PAUSE_CMD
Language=English
The job has recved the pause command.
.
Language=Chinese

已经向指定的Job发送了暂停命令
.

MessageId=0xd
Severity=Error
Facility=KxEThread
SymbolicName=E_KXETHREAD_JOB_NOT_PAUSE_CMD
Language=English
The job didn't have recved the pause command.
.
Language=Chinese

没有向指定的Job发送过暂停命令
.

MessageId=0xe
Severity=Error
Facility=KxEThread
SymbolicName=E_KXETHREAD_MANAGER_INITIALIZED
Language=English
The job manager is initialized.
.
Language=Chinese

线程管理器已经初始化.
.

MessageId=0xf
Severity=Error
Facility=KxEThread
SymbolicName=E_KXETHREAD_MANAGER_UNINITIALIZED
Language=English
The job manager is uninitialized.
.
Language=Chinese

线程管理器已经反初始化.
.

MessageId=0x10
Severity=Error
Facility=KxEThread
SymbolicName=E_KXETHREAD_MANAGER_STARTED
Language=English
The job manager is started.
.
Language=Chinese

线程管理器已经启动.
.

MessageId=0x11
Severity=Error
Facility=KxEThread
SymbolicName=E_KXETHREAD_MANAGER_STOPPED
Language=English
The job manager is stopped.
.
Language=Chinese

线程管理器已经停止.
.

MessageId=0x12
Severity=Error
Facility=KxEThread
SymbolicName=E_KXETHREAD_THREAD_POOL_INITIALIZED
Language=English
The threadpool is initialized.
.
Language=Chinese

线程池已经初始化.
.

MessageId=0x13
Severity=Error
Facility=KxEThread
SymbolicName=E_KXETHREAD_THREAD_POOL_UNINITIALIZED
Language=English
The threadpool is uninitialized.
.
Language=Chinese

线程池已经反初始化.
.

MessageId=0x14
Severity=Error
Facility=KxEThread
SymbolicName=E_KXETHREAD_THREAD_POOL_STARTED
Language=English
The thread pool is started.
.
Language=Chinese

线程池已经启动.
.

MessageId=0x15
Severity=Error
Facility=KxEThread
SymbolicName=E_KXETHREAD_THREAD_POOL_STOPPED
Language=English
The threadpool is stopped.
.
Language=Chinese

线程池已经停止.
.

; // KxEEvent
;
MessageId=0x1
Severity=Error
Facility=KxEEvent
SymbolicName=E_KXEEVENT_MANAGER_STOPPED
Language=English
The event manager is not startup.
.
Language=Chinese

事件管理器没有启动
.

MessageId=0x2
Severity=Error
Facility=KxEEvent
SymbolicName=E_KXEEVENT_MANAGER_STARTED
Language=English
The event manager is started.
.
Language=Chinese

事件管理器已经启动.
.

MessageId=0x3
Severity=Error
Facility=KxEEvent
SymbolicName=E_KXEEVENT_EVENT_SUBSYSTEM_REGISTER_NOT_FOUND
Language=English
It can't found the subsystem id in register.
.
Language=Chinese

没有找到指定的子系统id
.

MessageId=0x4
Severity=Error
Facility=KxEEvent
SymbolicName=E_KXEEVENT_EVENTNOTIFY_STOPPED
Language=English
The event notify manager is stopped.
.
Language=Chinese

事件通知管理器已经停止.
.

MessageId=0x5
Severity=Error
Facility=KxEEvent
SymbolicName=E_KXEEVENT_EVENTNOTIFY_STARTED
Language=English
The event notify manager is started.
.
Language=Chinese

事件通知管理器已经启动.
.

MessageId=0x6
Severity=Error
Facility=KxEEvent
SymbolicName=E_KXEEVENT_EVENT_REGISTER_ID_NOT_FOUND
Language=English
It can't found the register event id.
.
Language=Chinese

没有找到注册的指定事件id.
.

MessageId=0x7
Severity=Error
Facility=KxEEvent
SymbolicName=E_KXEEVENT_EVENT_REGISTER_NOTIFY_NOT_FOUND
Language=English
It can't found the register event notify pointer.
.
Language=Chinese

没有找到注册的指定事件响应接口.
.

; // KxEHttp

MessageId=0x1
Severity=Error
Facility=KxEHttp
SymbolicName=E_KXEHTTP_PORT_USED
Language=English
The port has been used.
.
Language=Chinese

指定端口已经被占用.
.

MessageId=0x2
Severity=Error
Facility=KxEHttp
SymbolicName=E_KXEHTTP_URI_DISPATCH_NOT_FOUND
Language=English
The uri dispatch is not found.
.
Language=Chinese

没有找到指定的uri的dispatch.
.

MessageId=0x3
Severity=Error
Facility=KxEHttp
SymbolicName=E_KXEHTTP_INITIALIZED
Language=English
The http is initialized.
.
Language=Chinese

http管理器已经初始化.
.

MessageId=0x4
Severity=Error
Facility=KxEHttp
SymbolicName=E_KXEHTTP_UNINITIALIZED
Language=English
The http is uninitialized.
.
Language=Chinese

http管理器已经反初始化.
.

MessageId=0x5
Severity=Error
Facility=KxEHttp
SymbolicName=E_KXEHTTP_STARTED
Language=English
The http is started.
.
Language=Chinese

http管理器已经启动.
.

MessageId=0x6
Severity=Error
Facility=KxEHttp
SymbolicName=E_KXEHTTP_STOPPED
Language=English
The http is stopped.
.
Language=Chinese

http管理器已经停止.
.

MessageId=0x7
Severity=Error
Facility=KxEHttp
SymbolicName=E_KXEHTTP_REQUEST_NOT_FOUND
Language=English
The http request is not found.
.
Language=Chinese

没有找到指定的http request.
.

MessageId=0x8
Severity=Error
Facility=KxEHttp
SymbolicName=E_KXEHTTP_SERVICE_METHOD_NOT_FOUND
Language=English
It can't find the service's method.
.
Language=Chinese

没有找到的指定服务的指定方法.
.

MessageId=0x9
Severity=Error
Facility=KxEHttp
SymbolicName=E_KXEHTTP_REQUEST_CONTROL_NOT_EXIST
Language=English
The request control is not exist.
.
Language=Chinese

指定的request control不存在.
.

MessageId=0xa
Severity=Error
Facility=KxEHttp
SymbolicName=E_KXEHTTP_AUTHORIZED_FAIL
Language=English
It failed to authorize.
.
Language=Chinese

授权失败.
.

MessageId=0xb
Severity=Error
Facility=KxEHttp
SymbolicName=E_KXEHTTP_QUERY_STRING_FAIL
Language=English
It failed to get query string in url.
.
Language=Chinese

从url里查询QUERY_STRING失败.
.

MessageId=0xc
Severity=Error
Facility=KxEHttp
SymbolicName=E_KXEHTTP_REQUEST_METHOD_NOT_SUPPORT
Language=English
The request method is not support.
.
Language=Chinese

不支持指定的http请求的方法
.

MessageId=0xd
Severity=Error
Facility=KxEHttp
SymbolicName=E_HTTP_CLIENT_READ_RESPONSE_FAIL
Language=English
It failed on reading http response.
.
Language=Chinese

http client在读取http response上出错.
.

MessageId=0xe
Severity=Error
Facility=KxEHttp
SymbolicName=E_HTTP_CLIENT_PARASE_STATUS_FAIL
Language=English
It failed to parase the status line..
.
Language=Chinese

http client在解析status line时出错
.

MessageId=0xf
Severity=Error
Facility=KxEHttp
SymbolicName=E_HTTP_CLIENT_PARASE_HEADER_FAIL
Language=English
It failed to parase the response header..
.
Language=Chinese

http client在解析response header时出错
.

MessageId=0x10
Severity=Error
Facility=KxEHttp
SymbolicName=E_HTTP_CLIENT_READ_SOCKET_FAIL
Language=English
It failed to read socket.
.
Language=Chinese

http client在读取socket内容时出错.
.

MessageId=0x11
Severity=Error
Facility=KxEHttp
SymbolicName=E_HTTP_CLIENT_READ_BODY_FAIL
Language=English
It failed to read body.
.
Language=Chinese

http client在获得body内容时出错.
.

MessageId=0x12
Severity=Error
Facility=KxEHttp
SymbolicName=E_HTTP_CLIENT_READ_CHUNKEDBODY_FAIL
Language=English
failed to read a 'Transfer-Encoding: chunked' response body
.
Language=Chinese

http client在获得'Transfer-Encoding: chunked' response body内容时出错.
.

MessageId=0x13
Severity=Error
Facility=KxEHttp
SymbolicName=E_HTTP_CLIENT_NOT_READ_RESPONSE
Language=English
we haven't started reading the response.
.
Language=Chinese

还没有开始读response.
.

MessageId=0x14
Severity=Error
Facility=KxEHttp
SymbolicName=E_KXEHTTP_EXCEED_POST_DATA_SIZE
Language=English
The post data size is exceed.
.
Language=Chinese

POST的数据的大小超过了最大值.
.

MessageId=0x15
Severity=Error
Facility=KxEHttp
SymbolicName=E_KXEHTTP_REQUEST_STATUS_NOT_CORRECT
Language=English
The request status may be in dispatching, but recv the data.
.
Language=Chinese

该请求正在分派却收到了新的数据.
.

MessageId=0x16
Severity=Error
Facility=KxEHttp
SymbolicName=E_KXEHTTP_SECURITY_CODE_NOT_APPENDED
Language=English
The security code not appended in the request data.
.
Language=Chinese

请求数据中没有包含验证码
.

MessageId=0x17
Severity=Error
Facility=KxEHttp
SymbolicName=E_KXEHTTP_INVALID_SECURITY_CODE
Language=English
The security code in the request data is invalid.
.
Language=Chinese

请求数据中包含的验证码无效
.

MessageId=0x18
Severity=Error
Facility=KxEHttp
SymbolicName=E_KXEHTTP_REQUEST_GET_METHOD_WITH_DATA
Language=English
The request is get, but with data.
.
Language=Chinese

HTTP的GET请求，却包含了数据.
.

; // KxEScanApi

; // KxEScanSystem

MessageId=0x1
Severity=Error
Facility=KSE
SymbolicName=E_KSE_OUT_OF_RANGE
Language=English
Out of range.
.
Language=Chinese
越界.
.

MessageId=0x2
Severity=Error
Facility=KSE
SymbolicName=E_KSE_INVALID_KSE_HANDLE
Language=English
Invalid handle.
.
Language=Chinese
无效句柄.
.

MessageId=0x3
Severity=Error
Facility=KSE
SymbolicName=E_KSE_DLL_CAN_NOT_BE_UNLOAD_SAFELY
Language=English
The dll can not be unload safely.
.
Language=Chinese
DLL不能被安全卸载.
.

MessageId=0x100
Severity=Error
Facility=KSE
SymbolicName=E_KSE_TASK_EXIST
Language=English
Task Exit.
.
Language=Chinese
Task Exit.
.

MessageId=0x101
Severity=Error
Facility=KSE
SymbolicName=E_KSE_TASK_IS_ACTIVE
Language=English
The Task is active.
.
Language=Chinese
The Task is active.
.

MessageId=0x102
Severity=Error
Facility=KSE
SymbolicName=E_KSE_UNKNOWN_CREATION_DISPOSITION
Language=English
Unknown creation disposition.
.
Language=Chinese
Unknown creation disposition.
.

MessageId=0x200
Severity=Error
Facility=KSE
SymbolicName=E_KSE_STATUS_OBJ_NOT_FOUND
Language=English
The status object can not be found.
.
Language=Chinese
找不到状态对象.
.

MessageId=0x201
Severity=Error
Facility=KSE
SymbolicName=E_KSE_STATUS_OBJ_NOT_CREATE
Language=English
The status object can not be created.
.
Language=Chinese
无法创建状态对象.
.

MessageId=0x202
Severity=Error
Facility=KSE
SymbolicName=E_KSE_UNSUPPORTED_TARGET_TYPE
Language=English
The target type is unsupported.
.
Language=Chinese
不支持的目标类型.
.

MessageId=0x203
Severity=Error
Facility=KSE
SymbolicName=E_KSE_UNSUPPORTED_OBJ_TYPE
Language=English
The object type is unsupported.
.
Language=Chinese
不支持的对象类型.
.

MessageId=0x204
Severity=Error
Facility=KSE
SymbolicName=E_KSE_UNABLE_TO_GET_DATA_FROM_STAT
Language=English
Unable to get data from status.
.
Language=Chinese
无法从状态对象中取得数据.
.

MessageId=0x205
Severity=Error
Facility=KSE
SymbolicName=E_KSE_COPY_INTERRUPTED
Language=English
This copy operation is interrupted.
.
Language=Chinese
复制操作被中断.
.

MessageId=0x301
Severity=Error
Facility=KSE
SymbolicName=E_KSE_DATA_PROPERTY_NOT_FOUND
Language=English
The data property is not found.
.
Language=Chinese
数据属性不存在.
.

MessageId=0x302
Severity=Error
Facility=KSE
SymbolicName=E_KSE_COMPONENT_NOT_INIT
Language=English
The component is not initialized.
.
Language=Chinese
组件未被初始化.
.

MessageId=0x303
Severity=Error
Facility=KSE
SymbolicName=E_KSE_EMPTY_VIRTUAL_PATH
Language=English
The virtual path is empty.
.
Language=Chinese
虚拟路径为空.
.

MessageId=0x401
Severity=Error
Facility=KSE
SymbolicName=E_KSE_TASK_IS_STOPPED
Language=English
The task is stopped.
.
Language=Chinese
The task is stopped.
.

MessageId=0x401
Severity=Success
Facility=KSE
SymbolicName=S_KSE_TASK_IS_STOPPED
Language=English
The task is stopped.
.
Language=Chinese
The task is stopped.
.

MessageId=0x501
Severity=Error
Facility=KSE
SymbolicName=E_KSE_ADDON_SCANNER_CONFLICT
Language=English
The scanner addons are conflict.
.
Language=Chinese
扫描器插件冲突.
.

MessageId=0x502
Severity=Error
Facility=KSE
SymbolicName=E_KSE_ADDON_PROCESSOR_CONFLICT
Language=English
The processor addons are conflict.
.
Language=Chinese
处理器插件冲突
.

MessageId=0x503
Severity=Error
Facility=KSE
SymbolicName=E_KSE_ADDON_SCAN_STATUS_CONFLICT
Language=English
The scan status addons are conflict.
.
Language=Chinese
扫描状态插件冲突.
.

MessageId=0x504
Severity=Error
Facility=KSE
SymbolicName=E_KSE_SOLUTION_TARGET_CONFLICT
Language=English
The solution target is conflict.
.
Language=Chinese
The solution target is conflict.
.

MessageId=0x505
Severity=Error
Facility=KSE
SymbolicName=E_KSE_SOLUTION_TARGET_IS_TOO_DEEP
Language=English
The solution target is too deep.
.
Language=Chinese
The solution target is too deep.
.

MessageId=0x506
Severity=Error
Facility=KSE
SymbolicName=E_KSE_FAIL_TO_OPEN_SOLUTION_FILE
Language=English
Fail to open solution file.
.
Language=Chinese
Fail to open solution file
.

MessageId=0x601
Severity=Error
Facility=KSE
SymbolicName=E_KSE_CHANNEL_EXIST
Language=English
The channel exist.
.
Language=Chinese
The channel exist.
.

MessageId=0x602
Severity=Error
Facility=KSE
SymbolicName=E_KSE_CHANNEL_DOSE_NOT_EXIST
Language=English
The channel does not exist.
.
Language=Chinese
The channel does not exist.
.

MessageId=0x603
Severity=Error
Facility=KSE
SymbolicName=E_KSE_CHANNEL_IS_DISABLED
Language=English
The channel is disabled.
.
Language=Chinese
The channel is disabled.
.

MessageId=0x701
Severity=Error
Facility=KSE
SymbolicName=E_KSE_ATTACHED_DATA_ITEM_NOT_EXIST
Language=English
The attached data item does not exist.
.
Language=Chinese
附加数据对象不存在.
.

MessageId=0x1
Severity=Error
Facility=KSEScanner
SymbolicName=E_KSE_BAD_TARGET_TYPE
Language=English
The target type is bad.
.
Language=Chinese
错误的目标类型.
.

MessageId=0x2
Severity=Error
Facility=KSEScanner
SymbolicName=E_KSE_NOT_SUPPORT_TARGET
Language=English
The target is unsupported.
.
Language=Chinese
不支持的目标类型.
.

MessageId=0x3
Severity=Error
Facility=KSEScanner
SymbolicName=E_KSE_DODISTRIBUTE
Language=English
The DoDistribute function failed.
.
Language=Chinese
DoDistribute函数失败.
.

MessageId=0x4
Severity=Error
Facility=KSEScanner
SymbolicName=E_KSE_INVALID_TARGET_ID
Language=English
The target ID is invalid.
.
Language=Chinese
无效的目标ID值.
.

MessageId=0x5
Severity=Error
Facility=KSEScanner
SymbolicName=E_KSE_AUTORUN_DLL_FAIL
Language=English
The kaeautorunex.dll error.
.
Language=Chinese
kaeautorunex.dll模块相关错误。
.

MessageId=0x6
Severity=Error
Facility=KSEScanner
SymbolicName=E_KSE_NOT_SUPPORTED_EVN_VAR
Language=English
This environment variable is not supported.
.
Language=Chinese
不支持的环境变量.
.

MessageId=0x7
Severity=Error
Facility=KSEScanner
SymbolicName=E_KSE_NOT_IMPLEMENT_GETEVVBYSELF_FUNC
Language=English
This environment variable used a not implemented GetEVVBySelf function.
.
Language=Chinese
此环境变量用到了一个没有实现的取值函数.
.

MessageId=0x1
Severity=Error
Facility=KSEFo
SymbolicName=E_KSE_FILE_NOT_EXIST
Language=English
The file does not exist.
.
Language=Chinese
文件不存在.
.

MessageId=0x2
Severity=Error
Facility=KSEFo
SymbolicName=E_KSE_CAN_NOT_OPEN_FILE
Language=English
The file can not be opened.
.
Language=Chinese
打开文件失败.
.

MessageId=0x3
Severity=Error
Facility=KSEFo
SymbolicName=E_KSE_CAN_NOT_DELETE_FILE
Language=English
The file can not be deleted.
.
Language=Chinese
删除文件失败.
.

MessageId=0x4
Severity=Error
Facility=KSEFo
SymbolicName=E_KSE_TEMP_FILE_OUT_OF_BUFF_SIZE
Language=English
The temp file buffer size is too small.
.
Language=Chinese
临时文件的内存缓冲区不够大。
.

MessageId=0x1
Severity=Error
Facility=KSEFileEngine
SymbolicName=E_KSE_GET_FILE_TYPE
Language=English
GetFileType function failed.
.
Language=Chinese
GetFileType函数失败.
.

MessageId=0x2
Severity=Error
Facility=KSEFileEngine
SymbolicName=E_KSE_BWM_SCAN
Language=English
BWM scan failed.
.
Language=Chinese
黑白机扫描失败.
.

MessageId=0x3
Severity=Error
Facility=KSEFileEngine
SymbolicName=E_KSE_BWM_ENDLESS_RESCAN
Language=English
Maybe BWM had a bug, that lead endless rescaning.
.
Language=Chinese
可能黑白机有bug，导致无止尽的重复扫描与清除.
.

MessageId=0x4
Severity=Error
Facility=KSEFileEngine
SymbolicName=E_KSE_BWMCALLER_INIT_FAILED
Language=English
KSEBWMCaller Init function failed.
.
Language=Chinese
黑白机调用模块的初始化函数返回失败.
.

MessageId=0x101
Severity=Error
Facility=KSEFileEngine
SymbolicName=E_KSE_EXTRACT_GET_ARC_TYPE
Language=English
GetArcType function failed.
.
Language=Chinese
GetArcType函数失败.
.

MessageId=0x102
Severity=Error
Facility=KSEFileEngine
SymbolicName=E_KSE_EXTRACT_DO_EXTRACT
Language=English
DoExtract function failed.
.
Language=Chinese
DoExtract函数失败.
.

MessageId=0x103
Severity=Error
Facility=KSEFileEngine
SymbolicName=E_KSE_EXTRACT_HEAD_FAIL
Language=English
Extract head failed.
.
Language=Chinese
解压头信息失败.
.

MessageId=0x104
Severity=Error
Facility=KSEFileEngine
SymbolicName=E_KSE_EXTRACT_DATA_FAIL
Language=English
Extract data failed.
.
Language=Chinese
解压数据信息失败.
.

MessageId=0x201
Severity=Error
Facility=KSEFileEngine
SymbolicName=E_KSE_UNPACK_GET_SHELL_TYPE
Language=English
GetShellType function failed.
.
Language=Chinese
GetShellType函数失败.
.

MessageId=0x202
Severity=Error
Facility=KSEFileEngine
SymbolicName=E_KSE_UNPACK_DO_UNPACK
Language=English
DoUnpack function failed.
.
Language=Chinese
DoUnpack函数失败.
.

MessageId=0x1
Severity=Error
Facility=KSEPluginSDK
SymbolicName=E_KSE_SDK_BAD_POINTER
Language=English
A bad pointer, like NULL, be passed to function.
.
Language=Chinese
向函数传入了一个不可用的指针，如NULL。
.

MessageId=0x2
Severity=Error
Facility=KSEPluginSDK
SymbolicName=E_KSE_SDK_NOT_INITIALIZE
Language=English
The SDK module has not been initialized.
.
Language=Chinese
SDK模块没有被初始化。
.

MessageId=0x3
Severity=Error
Facility=KSEPluginSDK
SymbolicName=E_KSE_SDK_BAD_PARAMETER
Language=English
The function be passed a bad parameter.
.
Language=Chinese
给函数传入了错误参数。
.

MessageId=0x4
Severity=Error
Facility=KSEPluginSDK
SymbolicName=E_KSE_SDK_INVALID_VERSION
Language=English
The version is not invalid.
.
Language=Chinese
版本不正确。
.


MessageId=0x1
Severity=Error
Facility=KSENetDet
SymbolicName=E_KSE_NET_DET_FAILED
Language=English
The net detecting failed.
.
Language=Chinese
网络检测失败。
.


; //	KWFS=0xF1:FACILITY_WFS

MessageId=0x1
Severity=Error
Facility=KWFS
SymbolicName=E_KW_NEW_OBJECT_FAILED
Language=English
It's failed to new object.
.
Language=Chinese
分配对象失败
.
	
MessageId=0x2
Severity=Error
Facility=KWFS
SymbolicName=E_KW_FDB_INITIALIZE_ERROR
Language=English
It's failed to initialize fdb.
.
Language=Chinese
初始化白名单数据库失败
.
	
MessageId=0x3
Severity=Error
Facility=KWFS
SymbolicName=E_KW_FV_INITIALIZE_ERROR
Language=English
It's failed to initialize fv.
.
Language=Chinese
初始化文件验证模块失败
.
	
MessageId=0x4
Severity=Error
Facility=KWFS
SymbolicName=E_KW_CALL_FM_FAILE
Language=English
It's failed to call FM SDK.
.
Language=Chinese
调用文件监控SDK失败
.
	
MessageId=0x5
Severity=Error
Facility=KWFS
SymbolicName=E_KW_CREATE_DBFILE_ERROR
Language=English
It's failed to create DB file.
.
Language=Chinese
创建或打开数据库文件失败
.
	
MessageId=0x6
Severity=Error
Facility=KWFS
SymbolicName=E_KWFV_START_THREAD_FAILED
Language=English
It's failed to start thread.
.
Language=Chinese
创建线程失败
.

MessageId=0x7
Severity=Error
Facility=KWFS
SymbolicName=E_KW_NOT_INIT
Language=English
Need initialize first.
.
Language=Chinese
请先初始化
.

MessageId=0x8
Severity=Error
Facility=KWFS
SymbolicName=E_KWFV_VERIFY_FILE_FAILED
Language=English
It's failed to add file to verify.
.
Language=Chinese
验证文件失败
.

MessageId=0x9
Severity=Error
Facility=KWFS
SymbolicName=E_KW_UNSUCCESS
Language=English
failed,unsuccess
.
Language=Chinese
没成功，失败了
.

MessageId=0xa
Severity=Error
Facility=KWFS
SymbolicName=E_KW_NOT_STOP
Language=English
Need stop first
.
Language=Chinese
奇怪先停止
.

; //	KWFDB=0xF2:FACILITY_WFS_FDB

MessageId=0x1
Severity=Error
Facility=KWFDB
SymbolicName=E_KWFDB_INVALID_PARAMETER
Language=English
The parameter is invalid
.
Language=Chinese
错误的变量
.

MessageId=0x2
Severity=Error
Facility=KWFDB
SymbolicName=E_KWFDB_FILE_ALREADY_EXISTS
Language=English
The file already exists.
.
Language=Chinese
文件已经存在
.

MessageId=0x3
Severity=Error
Facility=KWFDB
SymbolicName=E_KWFDB_GET_FILE_INFO
Language=English
It's failed to get file information.
.
Language=Chinese
获取文件信息失败
.
MessageId=0x4
Severity=Error
Facility=KWFDB
SymbolicName=E_KWFDB_FILE_NOT_FOUND
Language=English
The white file system connot find the file specified.
.
Language=Chinese
无法具体的找到这个文件
.

MessageId=0x5
Severity=Error
Facility=KWFDB
SymbolicName=E_KWFDB_FILE_TOO_LARGE
Language=English
The file size exceeds the limit allowed and cannot be saved.
.
Language=Chinese
文件大小超过限制
.

MessageId=0x6
Severity=Error
Facility=KWFDB
SymbolicName=E_KWFDB_ADD_FILE_FAILED
Language=English
It's failed to add the file information into file DB.
.
Language=Chinese
添加文件信息失败
.

MessageId=0x7
Severity=Error
Facility=KWFDB
SymbolicName=E_KWFDB_FIND_FILE_FAILED
Language=English
It's failed to find the file information in file DB.
.
Language=Chinese
在数据库中无法找到该文件的信息
.
	
MessageId=0x8
Severity=Error
Facility=KWFDB
SymbolicName=E_KWFDB_DELETE_FILE_FAILED
Language=English
it's failed to delete file information.
.
Language=Chinese
删除文件信息失败
.

MessageId=0x9
Severity=Error
Facility=KWFDB
SymbolicName=E_KWFDB_UPDATE_FILE_FAILED
Language=English
it's failed to update file information.
.
Language=Chinese
更新文件信息失败
.

MessageId=0xa
Severity=Error
Facility=KWFDB
SymbolicName=E_KWFDB_CALL_DB_FAILED
Language=English
It's failed to call DB.
.
Language=Chinese
数据库调用失败
.

MessageId=0xb
Severity=Error
Facility=KWFDB
SymbolicName=E_KWFDB_DB_NOT_RETURN_DATA
Language=English
The calling of DB is return none data.
.
Language=Chinese
数据库调用没有返回数据
.

MessageId=0xc
Severity=Error
Facility=KWFDB
SymbolicName=E_KWFDB_DB_EMPTY
Language=English
The file information DB is empty.
.
Language=Chinese
数据库为空
.

MessageId=0xd
Severity=Error
Facility=KWFDB
SymbolicName=E_KWFDB_FILE_INFO_EXPIRE
Language=English
The file information in DB is out of time.
.
Language=Chinese
文件信息已经过期
.

MessageId=0xe
Severity=Error
Facility=KWFDB
SymbolicName=E_KWFV_OPEN_FILE_FAILED
Language=English
It is failed to open file.
.
Language=Chinese
打开文件失败
.

MessageId=0xf
Severity=Error
Facility=KWFDB
SymbolicName=E_KWFDB_END_OF_DB
Language=English
It is end of DB
.
Language=Chinese
已经到数据库结尾.
.

MessageId=0x10
Severity=Error
Facility=KWFDB
SymbolicName=E_KWFDB_FILE_NEED_VERIFY
Language=English
Please verify the file first
.
Language=Chinese
请先验证文件
.

MessageId=0x11
Severity=Error
Facility=KWFDB
SymbolicName=E_KWFDB_NEED_START
Language=English
Please start the thread first
.
Language=Chinese
请开启线程
.

MessageId=0x12
Severity=Error
Facility=KWFDB
SymbolicName=E_KWFDB_COMPRESS_FAILED
Language=English
Compress out of bounds
.
Language=Chinese
压缩解压越界
.

;//	KWFDB=0xF3:FACILITY_WFS_SP

MessageId=0x1
Severity=Error
Facility=KWSP
SymbolicName=E_KWSP_INITIALIZE_FAILED
Language=English
It's failed to initialize .
.
Language=Chinese
白名单系统服务提供者初始化失败
.

MessageId=0x2
Severity=Error
Facility=KWSP
SymbolicName=E_KWSP_START_FAILED
Language=English
It's failed to start .
.
Language=Chinese
白名单系统服务提供者开始启动失败
.


;//KxEFM=0xA1:FACILITY_FILEMON

MessageId=0x1
Severity=Error
Facility=KxEFM
SymbolicName=E_KXEFILEMON_LOADDRIVER_FAILED
Language=English
It's failed to load driver.
.
Language=Chinese
加载驱动失败
.

MessageId=0x2
Severity=Error
Facility=KxEFM
SymbolicName=E_KXEFILEMON_GETSHAREMEM_FAILED
Language=English
It's failed to get share memory.
.
Language=Chinese
获取共享内存失败
.

MessageId=0x3
Severity=Error
Facility=KxEFM
SymbolicName=E_KXEFILEMON_INITSCANENGINE_FAILED
Language=English
It's failed to init scan engine.
.
Language=Chinese
初始化引擎失败
.

MessageId=0x4
Severity=Error
Facility=KxEFM
SymbolicName=E_KXEFILEMON_ERROR_INVALID_PARAMETER
Language=English
Parameter is invlided.
.
Language=Chinese
参数不正确
.

MessageId=0x5
Severity=Error
Facility=KxEFM
SymbolicName=E_KXEFILEMON_ERROR_INITIALIZE_FAILED
Language=English
It's failed to initialize.
.
Language=Chinese
文件监控子系统初始化失败
.

MessageId=0x6
Severity=Error
Facility=KxEFM
SymbolicName=E_KXEFILEMON_ERROR_UNINITIALIZE_FAILED
Language=English
It's failed to uninitialize.
.
Language=Chinese
文件监控子系统反初始化失败
.

MessageId=0x7
Severity=Error
Facility=KxEFM
SymbolicName=E_KXEFILEMON_STARTFILEMON_FAILED
Language=English
It's failed to startfilemon.
.
Language=Chinese
文件监控子系统启动失败
.

MessageId=0x8
Severity=Error
Facility=KxEFM
SymbolicName=E_KXEFILEMON_IS_ALREADY_STOP
Language=English
It's already stop.
.
Language=Chinese
文件监控子系统已经关闭
.

MessageId=0x9
Severity=Error
Facility=KxEFM
SymbolicName=E_KXEFILEMON_IS_NOT_INIT
Language=English
It's not init.
.
Language=Chinese
文件监控子系统没有进行初始化
.

MessageId=0xa
Severity=Error
Facility=KxEFM
SymbolicName=E_KXEFILEMON_ADDFILTERPROCESSID_ERROR
Language=English
It's failed to add filter processid.
.
Language=Chinese
文件监控子系统添加可信进程ID失败
.

MessageId=0xb
Severity=Error
Facility=KxEFM
SymbolicName=E_KXEFILEMON_REMOVFILTERPROCESSID_ERROR
Language=English
It's failed to remove filter processid.
.
Language=Chinese
文件监控子系统移除可信进程ID失败
.

MessageId=0xc
Severity=Error
Facility=KxEFM
SymbolicName=E_KXEFILEMON_WRITETEMPWHITELIST_ERROR
Language=English
It's failed to write temp whitelist.
.
Language=Chinese
文件监控子系统将动态白名单写入临时文件失败
.

MessageId=0xd
Severity=Error
Facility=KxEFM
SymbolicName=E_KXEFILEMON_FREEMD5TABLE_ERROR
Language=English
It's failed to free md5table.
.
Language=Chinese
文件监控子系统情况动态白名单失败
.

;//KxEFMSP=0xA2:FACILITY_FILEMON_SP

MessageId=0x1
Severity=Error
Facility=KxEFMSP
SymbolicName=E_KXEFILEMONSP_INITIALIZESERVICE_FAILED
Language=English
It's failed to initializeservice.
.
Language=Chinese
文件监控SP初始化失败
.

MessageId=0x2
Severity=Error
Facility=KxEFMSP
SymbolicName=E_KXEFILEMONSP_STARTFILEMON_FAILED
Language=English
It's failed to startfilemon.
.
Language=Chinese
文件监控SP启动文件监控失败
.

MessageId=0x3
Severity=Error
Facility=KxEFMSP
SymbolicName=E_KXEFILEMONSP_STOPFILEMON_FAILED
Language=English
It's failed to stopfilemon.
.
Language=Chinese
文件监控SP停止文件监控失败
.

MessageId=0x4
Severity=Error
Facility=KxEFMSP
SymbolicName=E_KXEFILEMONSP_FILEMONINTERFACE_ISNULL
Language=English
The FileMonInterface is null.
.
Language=Chinese
文件监控SP的FileMonInterface为空
.

MessageId=0x5
Severity=Error
Facility=KxEFMSP
SymbolicName=E_KXEFILEMONSP_QUERYFILEMON_ERROR
Language=English
It's failed to queryfilemon.
.
Language=Chinese
文件监控SP查询文件监控状态失败
.

MessageId=0x6
Severity=Error
Facility=KxEFMSP
SymbolicName=E_KXEFILEMONSP_SETFILEMONSETTING_ERROR
Language=English
It's failed to setfilemon.
.
Language=Chinese
文件监控SP设置文件监控是否随机启动失败
.

MessageId=0x7
Severity=Error
Facility=KxEFMSP
SymbolicName=E_KXEFILEMONSP_ADDFILTERPROCESSID_ERROR
Language=English
It's failed to addfilterprocessid.
.
Language=Chinese
文件监控SP添加可信进程ID失败
.

MessageId=0x8
Severity=Error
Facility=KxEFMSP
SymbolicName=E_KXEFILEMONSP_REMOVEFILTERPROCESSID_ERROR
Language=English
It's failed to removefilterprocessid.
.
Language=Chinese
文件监控SP移除可信进程ID失败
.

MessageId=0x9
Severity=Error
Facility=KxEFMSP
SymbolicName=E_KXEFILEMONSP_QUERYFILEMONSETTING_ERROR
Language=English
It's failed to queryfilemonsetting.
.
Language=Chinese
文件监控SP查询是否随机启动失败
.

MessageId=0xA
Severity=Error
Facility=KxEFMSP
SymbolicName=E_KXEFILEMONSP_QUERYFILEMONSCANINFO_ERROR
Language=English
It's failed to QueryFileMonScanInfo.
.
Language=Chinese
文件监控SP查询监控信息失败
.
MessageId=0xB
Severity=Error
Facility=KxEFMSP
SymbolicName=E_KXEFILEMONSP_NOTIFYENGINEUPDATED_ERROR
Language=English
It's failed to NotifyEngineUpdated
.
Language=Chinese
文件监控SP:通知文件监控Engine(病毒库或其他文件)更新需要重加载白名单时失败
.
MessageId=0xC
Severity=Error
Facility=KxEFMSP
SymbolicName=E_KXEFILEMONSP_CONFIG_TRUST_PATH_ERROR
Language=English
It's failed to ConfigTrustPath
.
Language=Chinese
文件监控SP:配置用户信任路径时失败
.
MessageId=0xD
Severity=Error
Facility=KxEFMSP
SymbolicName=E_KXEFILEMONSP_CONFIG_TRUST_FILE_TYPE_ERROR
Language=English
It's failed to ConfigTrustFileType
.
Language=Chinese
文件监控SP:配置用户信任文件类型（根据后缀判断）时失败
.
MessageId=0xE
Severity=Error
Facility=KxEFMSP
SymbolicName=E_KXEFILEMON_SET_WORK_MODE_ERROR
Language=English
It's failed to SetWorkMode
.
Language=Chinese
文件监控SP:配置监控详细工作模式时失败
.
MessageId=0xF
Severity=Error
Facility=KxEFMSP
SymbolicName=E_KXEFILEMON_SWITCH_WORK_MODE_ERROR
Language=English
It's failed to SwitchWorkMode
.
Language=Chinese
文件监控SP:切换监控工作模式时失败
.
; // KxESCSP

MessageId=0x1
Severity=Error
Facility=KxESCSP
SymbolicName=E_KXE_SCSP_LOAD_XML_FAILED
Language=English
It's failed to load the xml file.
.
Language=Chinese
加载xml文件失败
.

MessageId=0x2
Severity=Error
Facility=KxESCSP
SymbolicName=E_KXE_SCSP_BAD_XML_FILE
Language=English
The xml file is bad.
.
Language=Chinese
坏的xml文件
.

MessageId=0x3
Severity=Error
Facility=KxESCSP
SymbolicName=E_KXE_SCSP_NOT_LOAD_CFG_FILE
Language=English
The config file has not been loaded.
.
Language=Chinese
未加载配置文件
.

MessageId=0x4
Severity=Error
Facility=KxESCSP
SymbolicName=E_KXE_SCSP_NOT_LOAD_REG_INFO
Language=English
The registered product info has not been loaded.
.
Language=Chinese
未加载产品注册信息
.

MessageId=0x5
Severity=Error
Facility=KxESCSP
SymbolicName=E_KXE_SCSP_UNKNOWN_PRODUCT_ID
Language=English
The product id is unknowned.
.
Language=Chinese
未知产品ID
.

MessageId=0x6
Severity=Error
Facility=KxESCSP
SymbolicName=E_KXE_SCSP_UNKNOWN_VIP_FUNC_ID
Language=English
The vip function id is unknowned.
.
Language=Chinese
未知VIP功能ID
.

MessageId=0x7
Severity=Error
Facility=KxESCSP
SymbolicName=E_KXE_SCSP_INSTALL_PROCESS_EXIST
Language=English
The installing process already exist.
.
Language=Chinese
安装进程已启动
.

MessageId=0x8
Severity=Error
Facility=KxESCSP
SymbolicName=E_KXE_SCSP_CONFLICT_CFG_FILES
Language=English
The config file of the product is conflict with the config file of the Security Center.
.
Language=Chinese
产品的配置文件与安全中心的配置文件冲突。
.

MessageId=0x9
Severity=Error
Facility=KxESCSP
SymbolicName=E_KXE_SCSP_BAD_PARAMETER
Language=English
The parameter is invalid.
.
Language=Chinese
非法的调用参数。
.

MessageId=0xa
Severity=Error
Facility=KxESCSP
SymbolicName=E_KXE_SCSP_CFG_NOT_INSTALLED_PRODUCT
Language=English
The product which be configuring, has not been installed.
.
Language=Chinese
要配置的产品没有安装
.


; // KxESRSP

MessageId=0x1
Severity=Error
Facility=KxESRSP
SymbolicName=E_KXESRSP_UUID_CONFLICT
Language=English
The UUID has been used.
.
Language=Chinese
已使用的UUID。
.

MessageId=0x2
Severity=Error
Facility=KxESRSP
SymbolicName=E_KXESRSP_NOT_INITED
Language=English
The object has not been inited;
.
Language=Chinese
对象未被初始化
.

MessageId=0x3
Severity=Error
Facility=KxESRSP
SymbolicName=E_KXESRSP_INDEX_OUT_OF_RANGE
Language=English
The index is out of range.
.
Language=Chinese
索引越界
.

MessageId=0x4
Severity=Error
Facility=KxESRSP
SymbolicName=E_KXESRSP_USER_STOP
Language=English
Stop by user.
.
Language=Chinese
用户要求取消
.

MessageId=0x5
Severity=Error
Facility=KxESRSP
SymbolicName=E_KXESRSP_SESSION_NUM_MAX_LIMIT
Language=English
The count of sessions has to be the max limit.
.
Language=Chinese
会话数目达到上限
.

MessageId=0x1
Severity=Error
Facility=KxEBasicSP
SymbolicName=E_KXEBASIC_SERVICE_PROVIDER_STARTED
Language=English
The service provider is started.
.
Language=Chinese
指定的SP已经启动
.

