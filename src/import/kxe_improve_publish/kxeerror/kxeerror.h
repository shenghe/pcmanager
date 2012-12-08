 // ***** Sample.mc *****
 // This is the header section.
 // The following are message definitions.

//
//  Values are 32 bit values layed out as follows:
//
//   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
//   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
//  +---+-+-+-----------------------+-------------------------------+
//  |Sev|C|R|     Facility          |               Code            |
//  +---+-+-+-----------------------+-------------------------------+
//
//  where
//
//      Sev - is the severity code
//
//          00 - Success
//          01 - Informational
//          10 - Warning
//          11 - Error
//
//      C - is the Customer code flag
//
//      R - is a reserved bit
//
//      Facility - is the facility code
//
//      Code - is the facility's status code
//
//
// Define the facility codes
//
#define FACILITY_XML_HTTP_REQUEST        0xD
#define FACILITY_KXETHREAD               0x3
#define FACILITY_SR_SP                   0xA4
#define FACILITY_SERVICEMGR              0xA
#define FACILITY_SECURITYCENTER_SP       0xB1
#define FACILITY_KXESCAN_API             0x6
#define FACILITY_REGISTER                0x8
#define FACILITY_PIPE                    0xC
#define FACILITY_PASSPORT                0x9
#define FACILITY_IPC                     0xB
#define FACILITY_IDL                     0x7
#define FACILITY_KXEHTTP                 0x5
#define FACILITY_FILEMON_SP              0xA2
#define FACILITY_FILEMON_SDK             0xA3
#define FACILITY_FILEMON                 0xA1
#define FACILITY_KXEEVENT                0x4
#define FACILITY_KXECFG                  0x2
#define FACILITY_BASIC_SP                0xA5
#define FACILITY_KXEBASE                 0x0
#define FACILITY_WFS_SP                  0xF3
#define FACILITY_WFS                     0xF1
#define FACILITY_WFS_FDB                 0xF2
#define FACILITY_KSE_SQLITY              0xE3
#define FACILITY_KSE_SCANNER             0xE4
#define FACILITY_KSE_PROC                0xE5
#define FACILITY_KSE_PLUGIN_SDK          0xE8
#define FACILITY_KSE_NET_DET             0xE9
#define FACILITY_KSE_FO                  0xE6
#define FACILITY_KSE_FILEENGINE          0xE7
#define FACILITY_KSE                     0xE2


//
// Define the severity codes
//
#define STATUS_SEVERITY_WARNING          0x2
#define STATUS_SEVERITY_SUCCESS          0x0
#define STATUS_SEVERITY_INFORMATIONAL    0x1
#define STATUS_SEVERITY_ERROR            0x3


//
// MessageId: E_KXMLHTTP_NOT_OPEN
//
// MessageText:
//
//  The internet has not been open as the function open has not been called sucessfully yet.
//
#define E_KXMLHTTP_NOT_OPEN              ((DWORD)0xE00D0001L)

//
// MessageId: E_KXMLHTTP_NOT_ENOUGH_MEMORY
//
// MessageText:
//
//  Call new operator return a null pointer.
//
#define E_KXMLHTTP_NOT_ENOUGH_MEMORY     ((DWORD)0xE00D0002L)

//
// MessageId: E_KXMLHTTP_INCORRECT_RETURN_DATA
//
// MessageText:
//
//  The return data from the pipe server is incorrect.
//
#define E_KXMLHTTP_INCORRECT_RETURN_DATA ((DWORD)0xE00D0003L)

//
// MessageId: E_KXMLHTTP_INVALID_PARAMETER
//
// MessageText:
//
//  The parameter is invalid.
//
#define E_KXMLHTTP_INVALID_PARAMETER     ((DWORD)0xE00D0004L)

//
// MessageId: E_KXMLHTTP_PIPE_SERVER_NOT_RUNNING
//
// MessageText:
//
//  The named pipe server is not running right now.
//
#define E_KXMLHTTP_PIPE_SERVER_NOT_RUNNING ((DWORD)0xE00D0005L)

//
// MessageId: E_KXMLHTTP_CREATE_CLIENT_PIPE_INSTANCE_FAILED
//
// MessageText:
//
//  Create named pipe instance by call API CreateFile failed on the client end.
//
#define E_KXMLHTTP_CREATE_CLIENT_PIPE_INSTANCE_FAILED ((DWORD)0xE00D0006L)

//
// MessageId: E_KXMLHTTP_CREATE_IO_COMPLETION_PORT_FAILED
//
// MessageText:
//
//  Create IO Completion Port( IOCP ) for the named pipe failed on the client end.
//
#define E_KXMLHTTP_CREATE_IO_COMPLETION_PORT_FAILED ((DWORD)0xE00D0007L)

//
// MessageId: E_KXMLHTTP_CREATE_THREAD_FAILED
//
// MessageText:
//
//  Create thread for processing I/O request failed.
//
#define E_KXMLHTTP_CREATE_THREAD_FAILED  ((DWORD)0xE00D0008L)

//
// MessageId: E_KXMLHTTP_NOT_ALL_IO_PROCESSING_THREADS_WERE_NOTIFIED_TO_EXIT
//
// MessageText:
//
//  Not all of the threads for processing I/O request were notifyed to stop.
//
#define E_KXMLHTTP_NOT_ALL_IO_PROCESSING_THREADS_WERE_NOTIFIED_TO_EXIT ((DWORD)0xE00D0009L)

//
// MessageId: E_KXMLHTTP_WAIT_FOR_THREAD_EXIT_FAILED
//
// MessageText:
//
//  Wait for all the threads for processing I/O request failed.
//
#define E_KXMLHTTP_WAIT_FOR_THREAD_EXIT_FAILED ((DWORD)0xE00D000AL)

//
// MessageId: E_KXMLHTTP_PIPE_WRITE_FILE_FAILED
//
// MessageText:
//
//  Call the API WriteFile to write pipe failed.
//
#define E_KXMLHTTP_PIPE_WRITE_FILE_FAILED ((DWORD)0xE00D000BL)

//
// MessageId: E_KXMLHTP_PIPE_READ_FILE_FAILED
//
// MessageText:
//
//  Call the API ReadFile to read pipe failed.
//
#define E_KXMLHTP_PIPE_READ_FILE_FAILED  ((DWORD)0xE00D000CL)

//
// MessageId: E_KXMLHTTP_PREVIOUS_REQUEST_NOT_COMPLETED
//
// MessageText:
//
//  Request is denied because previous request has not completed.
//
#define E_KXMLHTTP_PREVIOUS_REQUEST_NOT_COMPLETED ((DWORD)0xE00D000DL)

//
// MessageId: E_KXMLHTTP_FAILED_TO_GET_SERVICE_PORT
//
// MessageText:
//
//  It failed to get service port of the HTTP server.
//
#define E_KXMLHTTP_FAILED_TO_GET_SERVICE_PORT ((DWORD)0xE00D000EL)

//
// MessageId: E_KXMLHTTP_TIME_OUT
//
// MessageText:
//
//  Operation failed with time out.
//
#define E_KXMLHTTP_TIME_OUT              ((DWORD)0xE00D000FL)

 // KxEPipe
//
// MessageId: E_KXEPIPE_CREATE_NAMED_PIPE_FAILED
//
// MessageText:
//
//  Create named pipe failed.
//
#define E_KXEPIPE_CREATE_NAMED_PIPE_FAILED ((DWORD)0xE00C0001L)

//
// MessageId: E_KXEPIPE_CREATE_THREAD_FAILED
//
// MessageText:
//
//  Create thread failed.
//
#define E_KXEPIPE_CREATE_THREAD_FAILED   ((DWORD)0xE00C0002L)

//
// MessageId: E_KXEPIPE_CONNECT_PIPE_FAILED
//
// MessageText:
//
//  The server end of the named pipe connect the client failed.
//
#define E_KXEPIPE_CONNECT_PIPE_FAILED    ((DWORD)0xE00C0003L)

//
// MessageId: E_KXEPIPE_WAIT_NAMED_PIPE_FAILED
//
// MessageText:
//
//  The client end of the named pipe connect the server end failed.
//
#define E_KXEPIPE_WAIT_NAMED_PIPE_FAILED ((DWORD)0xE00C0004L)

//
// MessageId: E_KXEPIPE_CREATE_INSTANCE_ON_THE_PIPE_CLIENT_END_FAILED
//
// MessageText:
//
//  Call CreateFile to create instance on the pipe client end failed.
//
#define E_KXEPIPE_CREATE_INSTANCE_ON_THE_PIPE_CLIENT_END_FAILED ((DWORD)0xE00C0005L)

//
// MessageId: E_KXEPIPE_DISCONNCET_NAMED_PIPE_FAILED
//
// MessageText:
//
//  The server end of the named pipe disconncet the pipe failed.
//
#define E_KXEPIPE_DISCONNCET_NAMED_PIPE_FAILED ((DWORD)0xE00C0006L)

//
// MessageId: E_KXEPIPE_NAMED_PIPE_NOT_START
//
// MessageText:
//
//  The named pipe is not started as the user have not call function Start.
//
#define E_KXEPIPE_NAMED_PIPE_NOT_START   ((DWORD)0xE00C0007L)

//
// MessageId: E_KXEPIPE_SERVER_WRITE_FILE_FAILED
//
// MessageText:
//
//  The server end write into the named pipe failed.
//
#define E_KXEPIPE_SERVER_WRITE_FILE_FAILED ((DWORD)0xE00C0008L)

//
// MessageId: E_KXEPIPE_CLIENT_WRITE_FILE_FAILED
//
// MessageText:
//
//  The client end write into the named pipe failed.
//
#define E_KXEPIPE_CLIENT_WRITE_FILE_FAILED ((DWORD)0xE00C0009L)

//
// MessageId: E_KXEPIPE_SERVER_READ_FILE_FAILED
//
// MessageText:
//
//  The server end read from the named pipe failed.
//
#define E_KXEPIPE_SERVER_READ_FILE_FAILED ((DWORD)0xE00C000AL)

//
// MessageId: E_KXEPIPE_CLIENT_READ_FILE_FAILED
//
// MessageText:
//
//  The client end read from the named pipe failed.
//
#define E_KXEPIPE_CLIENT_READ_FILE_FAILED ((DWORD)0xE00C000BL)

//
// MessageId: E_KXEPIPE_PRODUCT_ALREADY_REGISTERED
//
// MessageText:
//
//  The product has been already registered.
//
#define E_KXEPIPE_PRODUCT_ALREADY_REGISTERED ((DWORD)0xE00C000CL)

//
// MessageId: E_KXEPIPE_INCORRECT_PARAMETER
//
// MessageText:
//
//  Incorrect parameter(s) passed by caller or delivered in the named pipe.
//
#define E_KXEPIPE_INCORRECT_PARAMETER    ((DWORD)0xE00C000DL)

//
// MessageId: E_KXEPIPE_THREAD_COMPELLED_TO_STOP
//
// MessageText:
//
//  Thread is compelled to stop because the user has stopped the pipe.
//
#define E_KXEPIPE_THREAD_COMPELLED_TO_STOP ((DWORD)0xE00C000EL)

//
// MessageId: E_KXEPIPE_WAIT_FOR_THREAD_EXIT_FAILED
//
// MessageText:
//
//  Wait for all threads created by the pipe failed.
//
#define E_KXEPIPE_WAIT_FOR_THREAD_EXIT_FAILED ((DWORD)0xE00C000FL)

//
// MessageId: E_KXEPIPE_BAD_RETURN_DATA
//
// MessageText:
//
//  Data returned from the server end or the client end is incorrect.
//
#define E_KXEPIPE_BAD_RETURN_DATA        ((DWORD)0xE00C0010L)

//
// MessageId: E_KXEPIPE_SERVICE_PROVIDER_ALREADY_REGISTERED
//
// MessageText:
//
//  The service provider has been already registered.
//
#define E_KXEPIPE_SERVICE_PROVIDER_ALREADY_REGISTERED ((DWORD)0xE00C0011L)

//
// MessageId: E_KXEPIPE_SERVICE_PROVIDER_NOT_REGISTERED
//
// MessageText:
//
//  Service provider not registered on the subproduct end.
//
#define E_KXEPIPE_SERVICE_PROVIDER_NOT_REGISTERED ((DWORD)0xE00C0012L)

//
// MessageId: E_KXEPIPE_PRODUCT_NOT_REGISTERED
//
// MessageText:
//
//  Product ID has not been registered.
//
#define E_KXEPIPE_PRODUCT_NOT_REGISTERED ((DWORD)0xE00C0013L)

//
// MessageId: E_KXEPIPE_IMPERSONATE_CLIENT_FAILED
//
// MessageText:
//
//  The server end of the named pipe impersonate the client end failed.
//
#define E_KXEPIPE_IMPERSONATE_CLIENT_FAILED ((DWORD)0xE00C0014L)

//
// MessageId: E_KXEPIPE_REVERT_TO_SELF_FAILED
//
// MessageText:
//
//  The server end of the named pipe revert to self failed.
//
#define E_KXEPIPE_REVERT_TO_SELF_FAILED  ((DWORD)0xE00C0015L)

//
// MessageId: E_KXEPIPE_SP_PROCESS_REQUEST_FAILED
//
// MessageText:
//
//  The service provider failed to process request.
//
#define E_KXEPIPE_SP_PROCESS_REQUEST_FAILED ((DWORD)0xE00C0016L)

//
// MessageId: E_KXEPIPE_CREATE_IO_COMPLETION_PORT_FAILED
//
// MessageText:
//
//  Create I/O completion port or associate an I/O port to a pipe handle failed.
//
#define E_KXEPIPE_CREATE_IO_COMPLETION_PORT_FAILED ((DWORD)0xE00C0017L)

//
// MessageId: E_KXEPIPE_CREATE_EVENT_FAILED
//
// MessageText:
//
//  Create event failed.
//
#define E_KXEPIPE_CREATE_EVENT_FAILED    ((DWORD)0xE00C0018L)

//
// MessageId: E_KXEPIPE_NOT_ALL_THREADS_ARE_NOTIFIED_TO_STOP
//
// MessageText:
//
//  Not all of worker threads are notified to stop, this is an fatal error.
//
#define E_KXEPIPE_NOT_ALL_THREADS_ARE_NOTIFIED_TO_STOP ((DWORD)0xE00C0019L)

//
// MessageId: E_KXEPIPE_PIPE_ALREADY_STARTED
//
// MessageText:
//
//  kxepipe component has been already started.
//
#define E_KXEPIPE_PIPE_ALREADY_STARTED   ((DWORD)0xE00C001AL)

//
// MessageId: E_KXEPIPE_SET_SECURITY_INFO_FAILED
//
// MessageText:
//
//  Call API SetSecurityInfo failed after the the pipe instance been created.
//
#define E_KXEPIPE_SET_SECURITY_INFO_FAILED ((DWORD)0xE00C001BL)

//
// MessageId: E_KXEPIPE_GET_SECURITY_DESCRIPTOR_SACL_FAILED
//
// MessageText:
//
//  Call API GetSecurityDescriptorSacl failed after the the pipe instance been created.
//
#define E_KXEPIPE_GET_SECURITY_DESCRIPTOR_SACL_FAILED ((DWORD)0xE00C001CL)

//
// MessageId: E_KXEPIPE_CONVERT_STRING_TO_SECURITY_DESCRIPTOR_FAILED
//
// MessageText:
//
//  Call API ConvertStringSecurityDescriptorToSecurityDescriptorW failed after the the pipe instance been created.
//
#define E_KXEPIPE_CONVERT_STRING_TO_SECURITY_DESCRIPTOR_FAILED ((DWORD)0xE00C001DL)

//
// MessageId: E_KXEPIPE_NOT_ENOUGH_MEMORY
//
// MessageText:
//
//  Call new operator to allocate memory failed.
//
#define E_KXEPIPE_NOT_ENOUGH_MEMORY      ((DWORD)0xE00C001EL)

//
// MessageId: E_KXEPIPE_SP_PIPE_NAME_NOT_FOUND_IN_REGISTRY
//
// MessageText:
//
//  The specified Service Provider's pipe name was not found in registry.
//
#define E_KXEPIPE_SP_PIPE_NAME_NOT_FOUND_IN_REGISTRY ((DWORD)0xE00C001FL)

 // KxEIPC
//
// MessageId: E_KXEIPC_SP_NOT_FOUND
//
// MessageText:
//
//  The service provider is not fouend.
//
#define E_KXEIPC_SP_NOT_FOUND            ((DWORD)0xE00B0001L)

//
// MessageId: E_KXEIPC_MANAGE_SERVER_ALREADY_INITIALIZED
//
// MessageText:
//
//  The manage server has been already initialized.
//
#define E_KXEIPC_MANAGE_SERVER_ALREADY_INITIALIZED ((DWORD)0xE00B0002L)

//
// MessageId: E_KXEIPC_MANAGE_SERVER_NOT_INITIALIZED
//
// MessageText:
//
//  The manage server has not been initialized yet.
//
#define E_KXEIPC_MANAGE_SERVER_NOT_INITIALIZED ((DWORD)0xE00B0003L)

//
// MessageId: E_KXEIPC_MANAGE_SERVER_ALREADY_STARTED
//
// MessageText:
//
//  The manage server has been already started.
//
#define E_KXEIPC_MANAGE_SERVER_ALREADY_STARTED ((DWORD)0xE00B0004L)

//
// MessageId: E_KXEIPC_MANAGE_SERVER_NOT_STARTED
//
// MessageText:
//
//  The manage server has not been started yet.
//
#define E_KXEIPC_MANAGE_SERVER_NOT_STARTED ((DWORD)0xE00B0005L)

//
// MessageId: E_KXEIPC_MANAGE_CLIENT_ALREADY_INITIALIZED
//
// MessageText:
//
//  The manage client has been already initialized.
//
#define E_KXEIPC_MANAGE_CLIENT_ALREADY_INITIALIZED ((DWORD)0xE00B0006L)

//
// MessageId: E_KXEIPC_MANAGE_CLIENT_NOT_INITIALIZED
//
// MessageText:
//
//  The manage client has not been initialized yet.
//
#define E_KXEIPC_MANAGE_CLIENT_NOT_INITIALIZED ((DWORD)0xE00B0007L)

//
// MessageId: E_KXEIPC_MANAGE_CLIENT_ALREADY_STARTED
//
// MessageText:
//
//  The manage client has been already started.
//
#define E_KXEIPC_MANAGE_CLIENT_ALREADY_STARTED ((DWORD)0xE00B0008L)

//
// MessageId: E_KXEIPC_MANAGE_CLIENT_NOT_STARTED
//
// MessageText:
//
//  The manage client has not been started yet.
//
#define E_KXEIPC_MANAGE_CLIENT_NOT_STARTED ((DWORD)0xE00B0009L)

//
// MessageId: E_KXEIPC_NOT_ALL_SERVICE_PROVIDER_REGISTERED
//
// MessageText:
//
//  Not all of the service provider of the client reregistered to the server successfully.
//
#define E_KXEIPC_NOT_ALL_SERVICE_PROVIDER_REGISTERED ((DWORD)0xE00B000AL)

//
// MessageId: E_KXEIPC_PIPE_SERVER_DOES_NOT_EXIST
//
// MessageText:
//
//  The given named pipe server doesn't exist .
//
#define E_KXEIPC_PIPE_SERVER_DOES_NOT_EXIST ((DWORD)0xE00B000BL)

//
// MessageId: E_KXEIPC_NOT_ENOUGH_MEMORY
//
// MessageText:
//
//  Call the new operator to allocate memory failed.
//
#define E_KXEIPC_NOT_ENOUGH_MEMORY       ((DWORD)0xE00B000CL)

//
// MessageId: E_KXEIPC_CREATE_SP_PIPE_INFO_KEY_FAILED
//
// MessageText:
//
//  It failed to create the registry key for the specified SP's pipe information.
//
#define E_KXEIPC_CREATE_SP_PIPE_INFO_KEY_FAILED ((DWORD)0xE00B000DL)

//
// MessageId: E_KXEIPC_SET_SP_PIPE_NAME_KEY_FAILED
//
// MessageText:
//
//  It failed to set the specified SP's pipe name key in the registry.
//
#define E_KXEIPC_SET_SP_PIPE_NAME_KEY_FAILED ((DWORD)0xE00B000EL)

//
// MessageId: E_KXEIPC_DELETE_SP_PIPE_INFO_KEY_FAILED
//
// MessageText:
//
//  It failed to delete the specified SP's pipe information in the registry.
//
#define E_KXEIPC_DELETE_SP_PIPE_INFO_KEY_FAILED ((DWORD)0xE00B000FL)

//
// MessageId: E_KXEIPC_ILLEGAL_URL
//
// MessageText:
//
//  The request caller's URL is illegal.
//
#define E_KXEIPC_ILLEGAL_URL             ((DWORD)0xE00B0010L)

//
// MessageId: E_KXEIPC_READ_INI_FILE_FAILED
//
// MessageText:
//
//   It failed to read the .ini filewhich stores legal websites.
//
#define E_KXEIPC_READ_INI_FILE_FAILED    ((DWORD)0xE00B0011L)

 // KxEServiceMgr
//
// MessageId: E_KXESERVICEMGR_NOT_INITIALIZED
//
// MessageText:
//
//  The service manager is not initialized.
//
#define E_KXESERVICEMGR_NOT_INITIALIZED  ((DWORD)0xE00A0001L)

//
// MessageId: E_KXESERVICEMGR_INITIALIZED
//
// MessageText:
//
//  The service manager is initialized.
//
#define E_KXESERVICEMGR_INITIALIZED      ((DWORD)0xE00A0002L)

//
// MessageId: E_KXESERVICEMGR_PRODUCTMGR_NOT_INITIALIZED
//
// MessageText:
//
//  The product manager is not initialized.
//
#define E_KXESERVICEMGR_PRODUCTMGR_NOT_INITIALIZED ((DWORD)0xE00A0003L)

//
// MessageId: E_KXESERVICEMGR_PRODUCTMGR_INITIALIZED
//
// MessageText:
//
//  The product manager is initialized.
//
#define E_KXESERVICEMGR_PRODUCTMGR_INITIALIZED ((DWORD)0xE00A0004L)

//
// MessageId: E_KXESERVICEMGR_PRODUCTMGR_PRODUCT_EXIST
//
// MessageText:
//
//  The product is exist.
//
#define E_KXESERVICEMGR_PRODUCTMGR_PRODUCT_EXIST ((DWORD)0xE00A0005L)

//
// MessageId: E_KXESERVICEMGR_PRODUCTMGR_PRODUCT_NOT_EXIST
//
// MessageText:
//
//  The product is not exist.
//
#define E_KXESERVICEMGR_PRODUCTMGR_PRODUCT_NOT_EXIST ((DWORD)0xE00A0006L)

//
// MessageId: E_KXESERVICEMGR_PRODUCTMGR_CLSID_DUPLICATE
//
// MessageText:
//
//  The component's clsid is duplicate
//
#define E_KXESERVICEMGR_PRODUCTMGR_CLSID_DUPLICATE ((DWORD)0xE00A0007L)

//
// MessageId: E_KXESERVICEMGR_PRODUCTMGR_PATH_INVALID
//
// MessageText:
//
//  The product's path is invalid
//
#define E_KXESERVICEMGR_PRODUCTMGR_PATH_INVALID ((DWORD)0xE00A0008L)

//
// MessageId: E_KXESERVICEMGR_PRODUCTMGR_PLUGIN_PATH_INVALID
//
// MessageText:
//
//  The product's plugin path is invalid
//
#define E_KXESERVICEMGR_PRODUCTMGR_PLUGIN_PATH_INVALID ((DWORD)0xE00A0009L)

//
// MessageId: E_KXESERVICEMGR_PRODUCTMGR_WEB_PATH_INVALID
//
// MessageText:
//
//  The product's web path is invalid
//
#define E_KXESERVICEMGR_PRODUCTMGR_WEB_PATH_INVALID ((DWORD)0xE00A000AL)

//
// MessageId: E_KXESERVICEMGR_PRODUCTMGR_SP_NOT_COMPONENT
//
// MessageText:
//
//  The service provider file is not in component file.
//
#define E_KXESERVICEMGR_PRODUCTMGR_SP_NOT_COMPONENT ((DWORD)0xE00A000BL)

//
// MessageId: E_KXESERVICEMGR_PRODUCTMGR_SP_CLSID_NOT_COMPONENT
//
// MessageText:
//
//  The service provider clsid is not in component clsid.
//
#define E_KXESERVICEMGR_PRODUCTMGR_SP_CLSID_NOT_COMPONENT ((DWORD)0xE00A000CL)

//
// MessageId: E_KXESERVICEMGR_PRODUCTMGR_SP_CLSID_DUPLICATE
//
// MessageText:
//
//  The service provider component's clsid is duplicate
//
#define E_KXESERVICEMGR_PRODUCTMGR_SP_CLSID_DUPLICATE ((DWORD)0xE00A000DL)

//
// MessageId: E_KXESERVICEMGR_ALL_SP_FAIL_IN_PRODUCT
//
// MessageText:
//
//  The all service providers in a product is failed.
//
#define E_KXESERVICEMGR_ALL_SP_FAIL_IN_PRODUCT ((DWORD)0xE00A000EL)

//
// MessageId: E_KXESERVICEMGR_PRODUCT_NOT_RUNNING
//
// MessageText:
//
//  The product is not running.
//
#define E_KXESERVICEMGR_PRODUCT_NOT_RUNNING ((DWORD)0xE00A000FL)

//
// MessageId: E_KXESERVICEMGR_PRODUCT_RUNNING
//
// MessageText:
//
//  The product is running.
//
#define E_KXESERVICEMGR_PRODUCT_RUNNING  ((DWORD)0xE00A0010L)

//
// MessageId: E_KXESERVICEMGR_GUID_INVALID
//
// MessageText:
//
//  The guid is invalid.
//
#define E_KXESERVICEMGR_GUID_INVALID     ((DWORD)0xE00A0011L)

//
// MessageId: E_KXESERVICEMGR_FILE_PATH_INVALID
//
// MessageText:
//
//  The file path is invalid.
//
#define E_KXESERVICEMGR_FILE_PATH_INVALID ((DWORD)0xE00A0012L)

//
// MessageId: E_KXESERVICEMGR_COMPONENT_NOT_EQUAL
//
// MessageText:
//
//  The component file in config is not equal in count
//
#define E_KXESERVICEMGR_COMPONENT_NOT_EQUAL ((DWORD)0xE00A0013L)

//
// MessageId: E_KXESERVICEMGR_SP_COMPONENT_NOT_EQUAL
//
// MessageText:
//
//  The sp component file in config is not equal in count
//
#define E_KXESERVICEMGR_SP_COMPONENT_NOT_EQUAL ((DWORD)0xE00A0014L)

//
// MessageId: E_KXESERVICEMGR_CHILD_PRODUCT_NOT_EQUAL
//
// MessageText:
//
//  The child product in config is not equal in count
//
#define E_KXESERVICEMGR_CHILD_PRODUCT_NOT_EQUAL ((DWORD)0xE00A0015L)

//
// MessageId: E_KXESERVICEMGR_TASK_EXCEED_MAX
//
// MessageText:
//
//  The task is exceed the max number.
//
#define E_KXESERVICEMGR_TASK_EXCEED_MAX  ((DWORD)0xE00A0016L)

//
// MessageId: E_KXESERVICEMGR_TASK_NOT_FOUND
//
// MessageText:
//
//  The task is not found.
//
#define E_KXESERVICEMGR_TASK_NOT_FOUND   ((DWORD)0xE00A0017L)

//
// MessageId: E_KXESERVICEMGR_TASK_CAN_NOT_CLOSE
//
// MessageText:
//
//  The task can't be closed.
//
#define E_KXESERVICEMGR_TASK_CAN_NOT_CLOSE ((DWORD)0xE00A0018L)

//
// MessageId: E_KXESERVICEMGR_PROCESS_DETACHED
//
// MessageText:
//
//  The process is detached.
//
#define E_KXESERVICEMGR_PROCESS_DETACHED ((DWORD)0xE00A0019L)

 // KxEPassport
//
// MessageId: E_KXEPASSPORT_USERNAME_NOT_ASCII
//
// MessageText:
//
//  The user name is not ascii.
//
#define E_KXEPASSPORT_USERNAME_NOT_ASCII ((DWORD)0xE0090001L)

//
// MessageId: E_KXEPASSPORT_PPWIZ_IS_RUNNING
//
// MessageText:
//
//  The Ppwizard is running.
//
#define E_KXEPASSPORT_PPWIZ_IS_RUNNING   ((DWORD)0xE0090002L)

//
// MessageId: E_KXEPASSPORT_PPWIZ_NOT_UP_TO_SNUFF
//
// MessageText:
//
//  The scene is not up to snuff.
//
#define E_KXEPASSPORT_PPWIZ_NOT_UP_TO_SNUFF ((DWORD)0xE0090003L)

//
// MessageId: E_KXEPASSPORT_SP_NOT_INITIALIZED
//
// MessageText:
//
//  The Passport service provider has not been initialized.
//
#define E_KXEPASSPORT_SP_NOT_INITIALIZED ((DWORD)0xE0090004L)

//
// MessageId: E_KXEPASSPORT_NO_PPWIZ_WINDOW_TO_SHOW
//
// MessageText:
//
//  There is no ppwizard window to show.
//
#define E_KXEPASSPORT_NO_PPWIZ_WINDOW_TO_SHOW ((DWORD)0xE0090005L)

 // KxERegister
//
// MessageId: E_KXEREG_FAIL_LOAD_CONFIG
//
// MessageText:
//
//  It failed to load config file.
//
#define E_KXEREG_FAIL_LOAD_CONFIG        ((DWORD)0xE0080001L)

//
// MessageId: E_KXEREG_ZERO_FILE
//
// MessageText:
//
//  The file list is empty.
//
#define E_KXEREG_ZERO_FILE               ((DWORD)0xE0080002L)

 // KxEIdl
//
// MessageId: E_KXEIDL_XML_FAIL_TO_LOAD_FILE
//
// MessageText:
//
//  It failed to load xml file
//
#define E_KXEIDL_XML_FAIL_TO_LOAD_FILE   ((DWORD)0xE0070001L)

//
// MessageId: E_KXEIDL_XML_FAIL_TO_LOAD_TEXT
//
// MessageText:
//
//  It failed to load xml text.
//
#define E_KXEIDL_XML_FAIL_TO_LOAD_TEXT   ((DWORD)0xE0070002L)

//
// MessageId: E_KXEIDL_XML_FAIL_TO_LOAD_ROOT
//
// MessageText:
//
//  It failed to load xml root
//
#define E_KXEIDL_XML_FAIL_TO_LOAD_ROOT   ((DWORD)0xE0070003L)

//
// MessageId: E_KXEIDL_XML_FAIL_TO_SAVE_FILE
//
// MessageText:
//
//  It failed to save xml file
//
#define E_KXEIDL_XML_FAIL_TO_SAVE_FILE   ((DWORD)0xE0070004L)

//
// MessageId: E_KXEIDL_XML_FAIL_TO_GEN_TEXT
//
// MessageText:
//
//  It failed to gen text in xml.
//
#define E_KXEIDL_XML_FAIL_TO_GEN_TEXT    ((DWORD)0xE0070005L)

//
// MessageId: E_KXEIDL_XML_PARENT_NODE_IS_NULL
//
// MessageText:
//
//  The parent node is null in xml.
//
#define E_KXEIDL_XML_PARENT_NODE_IS_NULL ((DWORD)0xE0070006L)

//
// MessageId: E_KXEIDL_XML_CURRENT_NODE_IS_NULL
//
// MessageText:
//
//  The current node is null in xml.
//
#define E_KXEIDL_XML_CURRENT_NODE_IS_NULL ((DWORD)0xE0070007L)

//
// MessageId: E_KXEIDL_XML_FAIL_TO_REMOVE_NODE
//
// MessageText:
//
//  It failed to remove node in xml.
//
#define E_KXEIDL_XML_FAIL_TO_REMOVE_NODE ((DWORD)0xE0070008L)

//
// MessageId: E_KXEIDL_XML_FAIL_TO_GET_NODE_NAME
//
// MessageText:
//
//  It failed to get node name in xml.
//
#define E_KXEIDL_XML_FAIL_TO_GET_NODE_NAME ((DWORD)0xE0070009L)

//
// MessageId: E_KXEIDL_XML_FAIL_TO_GET_NODE_TEXT
//
// MessageText:
//
//  It failed to get node text in xml.
//
#define E_KXEIDL_XML_FAIL_TO_GET_NODE_TEXT ((DWORD)0xE007000AL)

//
// MessageId: E_KXEIDL_XML_FAIL_TO_SET_NODE_TEXT
//
// MessageText:
//
//  It failed to set node text in xml.
//
#define E_KXEIDL_XML_FAIL_TO_SET_NODE_TEXT ((DWORD)0xE007000BL)

//
// MessageId: E_KXEIDL_XML_VALUE_STREAM_ERROR
//
// MessageText:
//
//  The stream is error in xml.
//
#define E_KXEIDL_XML_VALUE_STREAM_ERROR  ((DWORD)0xE007000CL)

//
// MessageId: E_KXEIDL_XML_XMLMANAGER_EXPECTED
//
// MessageText:
//
//  The xmlmanger is null.
//
#define E_KXEIDL_XML_XMLMANAGER_EXPECTED ((DWORD)0xE007000DL)

//
// MessageId: E_KXEIDL_XML_SCOPE_STACK_IS_EMPTY
//
// MessageText:
//
//  The scope stack is empty in xml.
//
#define E_KXEIDL_XML_SCOPE_STACK_IS_EMPTY ((DWORD)0xE007000EL)

//
// MessageId: E_KXEIDL_XML_FAIL_TO_INIT_CURSOR
//
// MessageText:
//
//  It failed to init cursor in xml.
//
#define E_KXEIDL_XML_FAIL_TO_INIT_CURSOR ((DWORD)0xE007000FL)

//
// MessageId: E_KXEIDL_XML_FAIL_TO_LOAD_DEST_DOM_ROOT
//
// MessageText:
//
//  It failed to load dest dom's root in xml.
//
#define E_KXEIDL_XML_FAIL_TO_LOAD_DEST_DOM_ROOT ((DWORD)0xE0070010L)

//
// MessageId: E_KXEIDL_XML_FAIL_TO_SET_DEST_DOM_VALUE
//
// MessageText:
//
//  It failed to set dest dom's value in xml.
//
#define E_KXEIDL_XML_FAIL_TO_SET_DEST_DOM_VALUE ((DWORD)0xE0070011L)

//
// MessageId: E_KXEIDL_XML_FAIL_TO_COPY_NODE_TO_DEST_DOM
//
// MessageText:
//
//  It failed to copy node to dest dom in xml.
//
#define E_KXEIDL_XML_FAIL_TO_COPY_NODE_TO_DEST_DOM ((DWORD)0xE0070012L)

//
// MessageId: E_KXEIDL_XML_INVALID_CURSOR_STATE
//
// MessageText:
//
//  the cursor is in invalid state in xml.
//
#define E_KXEIDL_XML_INVALID_CURSOR_STATE ((DWORD)0xE0070013L)

//
// MessageId: E_KXEIDL_JSON_FAIL_TO_LOAD_FILE
//
// MessageText:
//
//  It failed to load json file
//
#define E_KXEIDL_JSON_FAIL_TO_LOAD_FILE  ((DWORD)0xE0070020L)

//
// MessageId: E_KXEIDL_JSON_FAIL_TO_LOAD_TEXT
//
// MessageText:
//
//  It failed to load json text.
//
#define E_KXEIDL_JSON_FAIL_TO_LOAD_TEXT  ((DWORD)0xE0070021L)

//
// MessageId: E_KXEIDL_JSON_FAIL_TO_LOAD_ROOT
//
// MessageText:
//
//  It failed to load json's root
//
#define E_KXEIDL_JSON_FAIL_TO_LOAD_ROOT  ((DWORD)0xE0070022L)

//
// MessageId: E_KXEIDL_JSON_FAIL_TO_SAVE_FILE
//
// MessageText:
//
//  It failed to save json file
//
#define E_KXEIDL_JSON_FAIL_TO_SAVE_FILE  ((DWORD)0xE0070023L)

//
// MessageId: E_KXEIDL_JSON_PARENT_NODE_IS_NULL
//
// MessageText:
//
//  The parent node is null in json.
//
#define E_KXEIDL_JSON_PARENT_NODE_IS_NULL ((DWORD)0xE0070024L)

//
// MessageId: E_KXEIDL_JSON_CURRENT_NODE_IS_NULL
//
// MessageText:
//
//  The current node is null in json.
//
#define E_KXEIDL_JSON_CURRENT_NODE_IS_NULL ((DWORD)0xE0070025L)

//
// MessageId: E_KXEIDL_JSON_ROOT_NODE_IS_NULL
//
// MessageText:
//
//  The root node is null in json.
//
#define E_KXEIDL_JSON_ROOT_NODE_IS_NULL  ((DWORD)0xE0070026L)

//
// MessageId: E_KXEIDL_JSON_EXPECT_STRING_TYPE
//
// MessageText:
//
//  It expected the string type in json.
//
#define E_KXEIDL_JSON_EXPECT_STRING_TYPE ((DWORD)0xE0070027L)

//
// MessageId: E_KXEIDL_JSON_VALUE_STREAM_ERROR
//
// MessageText:
//
//  The stream is error in json.
//
#define E_KXEIDL_JSON_VALUE_STREAM_ERROR ((DWORD)0xE0070028L)

//
// MessageId: E_KXEIDL_JSON_FAIL_TO_GET_NODE_NAME
//
// MessageText:
//
//  It failed to get node name in json.
//
#define E_KXEIDL_JSON_FAIL_TO_GET_NODE_NAME ((DWORD)0xE0070029L)

//
// MessageId: E_KXEIDL_XML_FAIL_TO_GET_ROOT_NAME
//
// MessageText:
//
//  It failed to get root name in json.
//
#define E_KXEIDL_XML_FAIL_TO_GET_ROOT_NAME ((DWORD)0xE007002AL)

//
// MessageId: E_KXEIDL_JSON_FAIL_TO_LOAD_DEST_DOM_ROOT
//
// MessageText:
//
//  It failed to load dest dom's root in json.
//
#define E_KXEIDL_JSON_FAIL_TO_LOAD_DEST_DOM_ROOT ((DWORD)0xE007002BL)

//
// MessageId: E_KXEIDL_JSON_FAIL_TO_INIT_CURSOR
//
// MessageText:
//
//  It failed to init cursor in json.
//
#define E_KXEIDL_JSON_FAIL_TO_INIT_CURSOR ((DWORD)0xE007002CL)

//
// MessageId: E_KXEIDL_JSON_FAIL_TO_COPY_NODE_TO_DEST_DOM
//
// MessageText:
//
//  It failed to copy node to dest dom in json.
//
#define E_KXEIDL_JSON_FAIL_TO_COPY_NODE_TO_DEST_DOM ((DWORD)0xE007002DL)

//
// MessageId: E_KXEIDL_JSON_SCOPE_STACK_IS_EMPTY
//
// MessageText:
//
//  The scope stack is empty in json.
//
#define E_KXEIDL_JSON_SCOPE_STACK_IS_EMPTY ((DWORD)0xE007002EL)

//
// MessageId: E_KXEIDL_KDOM_DATA_OUT_OF_RANGE
//
// MessageText:
//
//  The dom data is out of range.
//
#define E_KXEIDL_KDOM_DATA_OUT_OF_RANGE  ((DWORD)0xE0070040L)

//
// MessageId: E_KXEIDL_KDOM_DATA_TYPE_DO_NOT_SUPPORT_WRITING
//
// MessageText:
//
//  The data type is not support to write in dom.
//
#define E_KXEIDL_KDOM_DATA_TYPE_DO_NOT_SUPPORT_WRITING ((DWORD)0xE0070041L)

 // KxEBase
//
// MessageId: E_KXEBASE_PLUGIN_NOT_FOUND
//
// MessageText:
//
//  It failed to find the plugin.
//
#define E_KXEBASE_PLUGIN_NOT_FOUND       ((DWORD)0xE0000001L)

//
// MessageId: E_KXEBASE_PLUGIN_FUNCTION_NOT_FOUND
//
// MessageText:
//
//  It failed to find the function in the plugin.
//
#define E_KXEBASE_PLUGIN_FUNCTION_NOT_FOUND ((DWORD)0xE0000002L)

//
// MessageId: E_KXEBASE_CFG_NOT_INIT
//
// MessageText:
//
//  The config componenet wasn't initialied.
//
#define E_KXEBASE_CFG_NOT_INIT           ((DWORD)0xE0000003L)

//
// MessageId: E_KXEBASE_EVENT_NOT_INIT
//
// MessageText:
//
//  The event componenet wasn't initialied.
//
#define E_KXEBASE_EVENT_NOT_INIT         ((DWORD)0xE0000004L)

//
// MessageId: E_KXEBASE_HTTP_NOT_INIT
//
// MessageText:
//
//  The HTTP componenet wasn't initialied.
//
#define E_KXEBASE_HTTP_NOT_INIT          ((DWORD)0xE0000005L)

//
// MessageId: E_KXEBASE_PLUGIN_NOT_INIT
//
// MessageText:
//
//  The PLUGIN componenet wasn't initialied.
//
#define E_KXEBASE_PLUGIN_NOT_INIT        ((DWORD)0xE0000006L)

//
// MessageId: E_KXEBASE_INVALID_PARAMETER
//
// MessageText:
//
//  The parameter is invalid.
//
#define E_KXEBASE_INVALID_PARAMETER      ((DWORD)0xE0000007L)

//
// MessageId: E_KXEBASE_THREAD_NOT_INIT
//
// MessageText:
//
//  The thread component wasn't initialized.
//
#define E_KXEBASE_THREAD_NOT_INIT        ((DWORD)0xE0000008L)

//
// MessageId: E_KXEBASE_NOT_ENOUGH_MEMORY
//
// MessageText:
//
//  It failed to allocate memory.
//
#define E_KXEBASE_NOT_ENOUGH_MEMORY      ((DWORD)0xE0000009L)

//
// MessageId: E_KXEBASE_SERVICECALL_NOT_INIT
//
// MessageText:
//
//  The service call component wasn't initialized.
//
#define E_KXEBASE_SERVICECALL_NOT_INIT   ((DWORD)0xE000000AL)

//
// MessageId: E_KXEBASE_SYSTEM_NOT_SUPPORT
//
// MessageText:
//
//  It can't support the system.
//
#define E_KXEBASE_SYSTEM_NOT_SUPPORT     ((DWORD)0xE000000BL)

//
// MessageId: E_KXEBASE_PLUGIN_PATH_LOADED
//
// MessageText:
//
//  The plugin path has been loaded.
//
#define E_KXEBASE_PLUGIN_PATH_LOADED     ((DWORD)0xE000000CL)

//
// MessageId: E_KXEBASE_PLUGIN_PATH_NOT_LOADED
//
// MessageText:
//
//  The plugin path doesn't have been loaded.
//
#define E_KXEBASE_PLUGIN_PATH_NOT_LOADED ((DWORD)0xE000000DL)

//
// MessageId: E_KXEBASE_ACTIVITY_NOT_INIT
//
// MessageText:
//
//  The activity stat component is not initialized.
//
#define E_KXEBASE_ACTIVITY_NOT_INIT      ((DWORD)0xE000000EL)

//
// MessageId: E_KXEBASE_ACTIVITY_INITED
//
// MessageText:
//
//  The activity stat component is initialized.
//
#define E_KXEBASE_ACTIVITY_INITED        ((DWORD)0xE000000FL)

//
// MessageId: E_KXEBASE_FUNCTION_NOT_IMPLEMENTED
//
// MessageText:
//
//  The function is not implemented.
//
#define E_KXEBASE_FUNCTION_NOT_IMPLEMENTED ((DWORD)0xE0000010L)

//
// MessageId: E_KXEBASE_SERVICE_CAN_NOT_UNINSTALL
//
// MessageText:
//
//  The service can't be uninstalled.
//
#define E_KXEBASE_SERVICE_CAN_NOT_UNINSTALL ((DWORD)0xE0000011L)

//
// MessageId: E_KXEBASE_INITED
//
// MessageText:
//
//  The kxebase is initied.
//
#define E_KXEBASE_INITED                 ((DWORD)0xE0000012L)

//
// MessageId: E_KXEBASE_NOT_INITED
//
// MessageText:
//
//  The kxebase is not initied.
//
#define E_KXEBASE_NOT_INITED             ((DWORD)0xE0000013L)

//
// MessageId: E_KXEBASE_SERVICE_CAN_NOT_STOPPED
//
// MessageText:
//
//  The service can't be stopped.
//
#define E_KXEBASE_SERVICE_CAN_NOT_STOPPED ((DWORD)0xE0000014L)

//
// MessageId: E_KXEBASE_ALL_SP_INITIALIZE_FAILED
//
// MessageText:
//
//  The service must terminate because none of the SPs was initialized successfully.
//
#define E_KXEBASE_ALL_SP_INITIALIZE_FAILED ((DWORD)0xE0000015L)

//
// MessageId: E_KXEBASE_ALL_SP_START_FAILED
//
// MessageText:
//
//  No service provider in the provider list started successfully.
//
#define E_KXEBASE_ALL_SP_START_FAILED    ((DWORD)0xE0000016L)

//
// MessageId: E_KXEBASE_DLL_PATH_NOT_ACCQUIRED_FROM_CONFIGURATION_FILE
//
// MessageText:
//
//  It failed to accquire kxebase.dll's registry path from the configuration file 'kxecommon.dat'.
//
#define E_KXEBASE_DLL_PATH_NOT_ACCQUIRED_FROM_CONFIGURATION_FILE ((DWORD)0xE0000017L)

//
// MessageId: E_KXEBASE_START_SERVICE_STOPPED
//
// MessageText:
//
//  Service auto stop when start service.
//
#define E_KXEBASE_START_SERVICE_STOPPED  ((DWORD)0xE0000018L)

 // KxECfg
//
// MessageId: E_KXECFG_VALUE_NOT_CORRECT
//
// MessageText:
//
//  The value in registry is not correct.
//
#define E_KXECFG_VALUE_NOT_CORRECT       ((DWORD)0xE0020001L)

//
// MessageId: E_KXECFG_KXENGINE_CONFIG_PATH_NOT_ACCQUIRED_FROM_CONFIGURATION_FILE
//
// MessageText:
//
//  It failed to accquire KXENGINE's registry path from the configuration file 'kxecommon.dat'.
//
#define E_KXECFG_KXENGINE_CONFIG_PATH_NOT_ACCQUIRED_FROM_CONFIGURATION_FILE ((DWORD)0xE0020002L)

 // KxEThread
//
// MessageId: E_KXETHREAD_INVALID_HANDLE
//
// MessageText:
//
//  The job handle is invalid.
//
#define E_KXETHREAD_INVALID_HANDLE       ((DWORD)0xE0030001L)

//
// MessageId: E_KXETHREAD_JOB_FINISHED
//
// MessageText:
//
//  The target job is finished.
//
#define E_KXETHREAD_JOB_FINISHED         ((DWORD)0xE0030002L)

//
// MessageId: E_KXETHREAD_JOB_READY
//
// MessageText:
//
//  The target job is ready.
//
#define E_KXETHREAD_JOB_READY            ((DWORD)0xE0030003L)

//
// MessageId: E_KXETHREAD_JOB_RUNNING
//
// MessageText:
//
//  The job is running.
//
#define E_KXETHREAD_JOB_RUNNING          ((DWORD)0xE0030004L)

//
// MessageId: E_KXETHREAD_JOB_ERROR
//
// MessageText:
//
//  The target job is in error state.
//
#define E_KXETHREAD_JOB_ERROR            ((DWORD)0xE0030005L)

//
// MessageId: E_KXETHREAD_JOB_WAIT_NUMBER_EXCEED
//
// MessageText:
//
//  The number of job to wait is exceed 64.
//
#define E_KXETHREAD_JOB_WAIT_NUMBER_EXCEED ((DWORD)0xE0030006L)

//
// MessageId: E_KXETHREAD_JOB_CLOSED
//
// MessageText:
//
//  The job is closed.
//
#define E_KXETHREAD_JOB_CLOSED           ((DWORD)0xE0030007L)

//
// MessageId: E_KXETHREAD_JOB_NOT_FINISHED
//
// MessageText:
//
//  The target job is not finished.
//
#define E_KXETHREAD_JOB_NOT_FINISHED     ((DWORD)0xE0030008L)

//
// MessageId: E_KXETHREAD_JOB_NOT_READY
//
// MessageText:
//
//  The target job is not ready.
//
#define E_KXETHREAD_JOB_NOT_READY        ((DWORD)0xE0030009L)

//
// MessageId: E_KXETHREAD_JOB_NOT_RUNNING
//
// MessageText:
//
//  The job is not running.
//
#define E_KXETHREAD_JOB_NOT_RUNNING      ((DWORD)0xE003000AL)

//
// MessageId: E_KXETHREAD_JOB_IN_STOP_CMD
//
// MessageText:
//
//  The job has recved the stop command.
//
#define E_KXETHREAD_JOB_IN_STOP_CMD      ((DWORD)0xE003000BL)

//
// MessageId: E_KXETHREAD_JOB_IN_PAUSE_CMD
//
// MessageText:
//
//  The job has recved the pause command.
//
#define E_KXETHREAD_JOB_IN_PAUSE_CMD     ((DWORD)0xE003000CL)

//
// MessageId: E_KXETHREAD_JOB_NOT_PAUSE_CMD
//
// MessageText:
//
//  The job didn't have recved the pause command.
//
#define E_KXETHREAD_JOB_NOT_PAUSE_CMD    ((DWORD)0xE003000DL)

//
// MessageId: E_KXETHREAD_MANAGER_INITIALIZED
//
// MessageText:
//
//  The job manager is initialized.
//
#define E_KXETHREAD_MANAGER_INITIALIZED  ((DWORD)0xE003000EL)

//
// MessageId: E_KXETHREAD_MANAGER_UNINITIALIZED
//
// MessageText:
//
//  The job manager is uninitialized.
//
#define E_KXETHREAD_MANAGER_UNINITIALIZED ((DWORD)0xE003000FL)

//
// MessageId: E_KXETHREAD_MANAGER_STARTED
//
// MessageText:
//
//  The job manager is started.
//
#define E_KXETHREAD_MANAGER_STARTED      ((DWORD)0xE0030010L)

//
// MessageId: E_KXETHREAD_MANAGER_STOPPED
//
// MessageText:
//
//  The job manager is stopped.
//
#define E_KXETHREAD_MANAGER_STOPPED      ((DWORD)0xE0030011L)

//
// MessageId: E_KXETHREAD_THREAD_POOL_INITIALIZED
//
// MessageText:
//
//  The threadpool is initialized.
//
#define E_KXETHREAD_THREAD_POOL_INITIALIZED ((DWORD)0xE0030012L)

//
// MessageId: E_KXETHREAD_THREAD_POOL_UNINITIALIZED
//
// MessageText:
//
//  The threadpool is uninitialized.
//
#define E_KXETHREAD_THREAD_POOL_UNINITIALIZED ((DWORD)0xE0030013L)

//
// MessageId: E_KXETHREAD_THREAD_POOL_STARTED
//
// MessageText:
//
//  The thread pool is started.
//
#define E_KXETHREAD_THREAD_POOL_STARTED  ((DWORD)0xE0030014L)

//
// MessageId: E_KXETHREAD_THREAD_POOL_STOPPED
//
// MessageText:
//
//  The threadpool is stopped.
//
#define E_KXETHREAD_THREAD_POOL_STOPPED  ((DWORD)0xE0030015L)

 // KxEEvent

//
// MessageId: E_KXEEVENT_MANAGER_STOPPED
//
// MessageText:
//
//  The event manager is not startup.
//
#define E_KXEEVENT_MANAGER_STOPPED       ((DWORD)0xE0040001L)

//
// MessageId: E_KXEEVENT_MANAGER_STARTED
//
// MessageText:
//
//  The event manager is started.
//
#define E_KXEEVENT_MANAGER_STARTED       ((DWORD)0xE0040002L)

//
// MessageId: E_KXEEVENT_EVENT_SUBSYSTEM_REGISTER_NOT_FOUND
//
// MessageText:
//
//  It can't found the subsystem id in register.
//
#define E_KXEEVENT_EVENT_SUBSYSTEM_REGISTER_NOT_FOUND ((DWORD)0xE0040003L)

//
// MessageId: E_KXEEVENT_EVENTNOTIFY_STOPPED
//
// MessageText:
//
//  The event notify manager is stopped.
//
#define E_KXEEVENT_EVENTNOTIFY_STOPPED   ((DWORD)0xE0040004L)

//
// MessageId: E_KXEEVENT_EVENTNOTIFY_STARTED
//
// MessageText:
//
//  The event notify manager is started.
//
#define E_KXEEVENT_EVENTNOTIFY_STARTED   ((DWORD)0xE0040005L)

//
// MessageId: E_KXEEVENT_EVENT_REGISTER_ID_NOT_FOUND
//
// MessageText:
//
//  It can't found the register event id.
//
#define E_KXEEVENT_EVENT_REGISTER_ID_NOT_FOUND ((DWORD)0xE0040006L)

//
// MessageId: E_KXEEVENT_EVENT_REGISTER_NOTIFY_NOT_FOUND
//
// MessageText:
//
//  It can't found the register event notify pointer.
//
#define E_KXEEVENT_EVENT_REGISTER_NOTIFY_NOT_FOUND ((DWORD)0xE0040007L)

 // KxEHttp
//
// MessageId: E_KXEHTTP_PORT_USED
//
// MessageText:
//
//  The port has been used.
//
#define E_KXEHTTP_PORT_USED              ((DWORD)0xE0050001L)

//
// MessageId: E_KXEHTTP_URI_DISPATCH_NOT_FOUND
//
// MessageText:
//
//  The uri dispatch is not found.
//
#define E_KXEHTTP_URI_DISPATCH_NOT_FOUND ((DWORD)0xE0050002L)

//
// MessageId: E_KXEHTTP_INITIALIZED
//
// MessageText:
//
//  The http is initialized.
//
#define E_KXEHTTP_INITIALIZED            ((DWORD)0xE0050003L)

//
// MessageId: E_KXEHTTP_UNINITIALIZED
//
// MessageText:
//
//  The http is uninitialized.
//
#define E_KXEHTTP_UNINITIALIZED          ((DWORD)0xE0050004L)

//
// MessageId: E_KXEHTTP_STARTED
//
// MessageText:
//
//  The http is started.
//
#define E_KXEHTTP_STARTED                ((DWORD)0xE0050005L)

//
// MessageId: E_KXEHTTP_STOPPED
//
// MessageText:
//
//  The http is stopped.
//
#define E_KXEHTTP_STOPPED                ((DWORD)0xE0050006L)

//
// MessageId: E_KXEHTTP_REQUEST_NOT_FOUND
//
// MessageText:
//
//  The http request is not found.
//
#define E_KXEHTTP_REQUEST_NOT_FOUND      ((DWORD)0xE0050007L)

//
// MessageId: E_KXEHTTP_SERVICE_METHOD_NOT_FOUND
//
// MessageText:
//
//  It can't find the service's method.
//
#define E_KXEHTTP_SERVICE_METHOD_NOT_FOUND ((DWORD)0xE0050008L)

//
// MessageId: E_KXEHTTP_REQUEST_CONTROL_NOT_EXIST
//
// MessageText:
//
//  The request control is not exist.
//
#define E_KXEHTTP_REQUEST_CONTROL_NOT_EXIST ((DWORD)0xE0050009L)

//
// MessageId: E_KXEHTTP_AUTHORIZED_FAIL
//
// MessageText:
//
//  It failed to authorize.
//
#define E_KXEHTTP_AUTHORIZED_FAIL        ((DWORD)0xE005000AL)

//
// MessageId: E_KXEHTTP_QUERY_STRING_FAIL
//
// MessageText:
//
//  It failed to get query string in url.
//
#define E_KXEHTTP_QUERY_STRING_FAIL      ((DWORD)0xE005000BL)

//
// MessageId: E_KXEHTTP_REQUEST_METHOD_NOT_SUPPORT
//
// MessageText:
//
//  The request method is not support.
//
#define E_KXEHTTP_REQUEST_METHOD_NOT_SUPPORT ((DWORD)0xE005000CL)

//
// MessageId: E_HTTP_CLIENT_READ_RESPONSE_FAIL
//
// MessageText:
//
//  It failed on reading http response.
//
#define E_HTTP_CLIENT_READ_RESPONSE_FAIL ((DWORD)0xE005000DL)

//
// MessageId: E_HTTP_CLIENT_PARASE_STATUS_FAIL
//
// MessageText:
//
//  It failed to parase the status line..
//
#define E_HTTP_CLIENT_PARASE_STATUS_FAIL ((DWORD)0xE005000EL)

//
// MessageId: E_HTTP_CLIENT_PARASE_HEADER_FAIL
//
// MessageText:
//
//  It failed to parase the response header..
//
#define E_HTTP_CLIENT_PARASE_HEADER_FAIL ((DWORD)0xE005000FL)

//
// MessageId: E_HTTP_CLIENT_READ_SOCKET_FAIL
//
// MessageText:
//
//  It failed to read socket.
//
#define E_HTTP_CLIENT_READ_SOCKET_FAIL   ((DWORD)0xE0050010L)

//
// MessageId: E_HTTP_CLIENT_READ_BODY_FAIL
//
// MessageText:
//
//  It failed to read body.
//
#define E_HTTP_CLIENT_READ_BODY_FAIL     ((DWORD)0xE0050011L)

//
// MessageId: E_HTTP_CLIENT_READ_CHUNKEDBODY_FAIL
//
// MessageText:
//
//  failed to read a 'Transfer-Encoding: chunked' response body
//
#define E_HTTP_CLIENT_READ_CHUNKEDBODY_FAIL ((DWORD)0xE0050012L)

//
// MessageId: E_HTTP_CLIENT_NOT_READ_RESPONSE
//
// MessageText:
//
//  we haven't started reading the response.
//
#define E_HTTP_CLIENT_NOT_READ_RESPONSE  ((DWORD)0xE0050013L)

//
// MessageId: E_KXEHTTP_EXCEED_POST_DATA_SIZE
//
// MessageText:
//
//  The post data size is exceed.
//
#define E_KXEHTTP_EXCEED_POST_DATA_SIZE  ((DWORD)0xE0050014L)

//
// MessageId: E_KXEHTTP_REQUEST_STATUS_NOT_CORRECT
//
// MessageText:
//
//  The request status may be in dispatching, but recv the data.
//
#define E_KXEHTTP_REQUEST_STATUS_NOT_CORRECT ((DWORD)0xE0050015L)

//
// MessageId: E_KXEHTTP_SECURITY_CODE_NOT_APPENDED
//
// MessageText:
//
//  The security code not appended in the request data.
//
#define E_KXEHTTP_SECURITY_CODE_NOT_APPENDED ((DWORD)0xE0050016L)

//
// MessageId: E_KXEHTTP_INVALID_SECURITY_CODE
//
// MessageText:
//
//  The security code in the request data is invalid.
//
#define E_KXEHTTP_INVALID_SECURITY_CODE  ((DWORD)0xE0050017L)

//
// MessageId: E_KXEHTTP_REQUEST_GET_METHOD_WITH_DATA
//
// MessageText:
//
//  The request is get, but with data.
//
#define E_KXEHTTP_REQUEST_GET_METHOD_WITH_DATA ((DWORD)0xE0050018L)

 // KxEScanApi
 // KxEScanSystem
//
// MessageId: E_KSE_OUT_OF_RANGE
//
// MessageText:
//
//  Out of range.
//
#define E_KSE_OUT_OF_RANGE               ((DWORD)0xE0E20001L)

//
// MessageId: E_KSE_INVALID_KSE_HANDLE
//
// MessageText:
//
//  Invalid handle.
//
#define E_KSE_INVALID_KSE_HANDLE         ((DWORD)0xE0E20002L)

//
// MessageId: E_KSE_DLL_CAN_NOT_BE_UNLOAD_SAFELY
//
// MessageText:
//
//  The dll can not be unload safely.
//
#define E_KSE_DLL_CAN_NOT_BE_UNLOAD_SAFELY ((DWORD)0xE0E20003L)

//
// MessageId: E_KSE_TASK_EXIST
//
// MessageText:
//
//  Task Exit.
//
#define E_KSE_TASK_EXIST                 ((DWORD)0xE0E20100L)

//
// MessageId: E_KSE_TASK_IS_ACTIVE
//
// MessageText:
//
//  The Task is active.
//
#define E_KSE_TASK_IS_ACTIVE             ((DWORD)0xE0E20101L)

//
// MessageId: E_KSE_UNKNOWN_CREATION_DISPOSITION
//
// MessageText:
//
//  Unknown creation disposition.
//
#define E_KSE_UNKNOWN_CREATION_DISPOSITION ((DWORD)0xE0E20102L)

//
// MessageId: E_KSE_STATUS_OBJ_NOT_FOUND
//
// MessageText:
//
//  The status object can not be found.
//
#define E_KSE_STATUS_OBJ_NOT_FOUND       ((DWORD)0xE0E20200L)

//
// MessageId: E_KSE_STATUS_OBJ_NOT_CREATE
//
// MessageText:
//
//  The status object can not be created.
//
#define E_KSE_STATUS_OBJ_NOT_CREATE      ((DWORD)0xE0E20201L)

//
// MessageId: E_KSE_UNSUPPORTED_TARGET_TYPE
//
// MessageText:
//
//  The target type is unsupported.
//
#define E_KSE_UNSUPPORTED_TARGET_TYPE    ((DWORD)0xE0E20202L)

//
// MessageId: E_KSE_UNSUPPORTED_OBJ_TYPE
//
// MessageText:
//
//  The object type is unsupported.
//
#define E_KSE_UNSUPPORTED_OBJ_TYPE       ((DWORD)0xE0E20203L)

//
// MessageId: E_KSE_UNABLE_TO_GET_DATA_FROM_STAT
//
// MessageText:
//
//  Unable to get data from status.
//
#define E_KSE_UNABLE_TO_GET_DATA_FROM_STAT ((DWORD)0xE0E20204L)

//
// MessageId: E_KSE_COPY_INTERRUPTED
//
// MessageText:
//
//  This copy operation is interrupted.
//
#define E_KSE_COPY_INTERRUPTED           ((DWORD)0xE0E20205L)

//
// MessageId: E_KSE_DATA_PROPERTY_NOT_FOUND
//
// MessageText:
//
//  The data property is not found.
//
#define E_KSE_DATA_PROPERTY_NOT_FOUND    ((DWORD)0xE0E20301L)

//
// MessageId: E_KSE_COMPONENT_NOT_INIT
//
// MessageText:
//
//  The component is not initialized.
//
#define E_KSE_COMPONENT_NOT_INIT         ((DWORD)0xE0E20302L)

//
// MessageId: E_KSE_EMPTY_VIRTUAL_PATH
//
// MessageText:
//
//  The virtual path is empty.
//
#define E_KSE_EMPTY_VIRTUAL_PATH         ((DWORD)0xE0E20303L)

//
// MessageId: E_KSE_TASK_IS_STOPPED
//
// MessageText:
//
//  The task is stopped.
//
#define E_KSE_TASK_IS_STOPPED            ((DWORD)0xE0E20401L)

//
// MessageId: S_KSE_TASK_IS_STOPPED
//
// MessageText:
//
//  The task is stopped.
//
#define S_KSE_TASK_IS_STOPPED            ((DWORD)0x20E20401L)

//
// MessageId: E_KSE_ADDON_SCANNER_CONFLICT
//
// MessageText:
//
//  The scanner addons are conflict.
//
#define E_KSE_ADDON_SCANNER_CONFLICT     ((DWORD)0xE0E20501L)

//
// MessageId: E_KSE_ADDON_PROCESSOR_CONFLICT
//
// MessageText:
//
//  The processor addons are conflict.
//
#define E_KSE_ADDON_PROCESSOR_CONFLICT   ((DWORD)0xE0E20502L)

//
// MessageId: E_KSE_ADDON_SCAN_STATUS_CONFLICT
//
// MessageText:
//
//  The scan status addons are conflict.
//
#define E_KSE_ADDON_SCAN_STATUS_CONFLICT ((DWORD)0xE0E20503L)

//
// MessageId: E_KSE_SOLUTION_TARGET_CONFLICT
//
// MessageText:
//
//  The solution target is conflict.
//
#define E_KSE_SOLUTION_TARGET_CONFLICT   ((DWORD)0xE0E20504L)

//
// MessageId: E_KSE_SOLUTION_TARGET_IS_TOO_DEEP
//
// MessageText:
//
//  The solution target is too deep.
//
#define E_KSE_SOLUTION_TARGET_IS_TOO_DEEP ((DWORD)0xE0E20505L)

//
// MessageId: E_KSE_FAIL_TO_OPEN_SOLUTION_FILE
//
// MessageText:
//
//  Fail to open solution file.
//
#define E_KSE_FAIL_TO_OPEN_SOLUTION_FILE ((DWORD)0xE0E20506L)

//
// MessageId: E_KSE_CHANNEL_EXIST
//
// MessageText:
//
//  The channel exist.
//
#define E_KSE_CHANNEL_EXIST              ((DWORD)0xE0E20601L)

//
// MessageId: E_KSE_CHANNEL_DOSE_NOT_EXIST
//
// MessageText:
//
//  The channel does not exist.
//
#define E_KSE_CHANNEL_DOSE_NOT_EXIST     ((DWORD)0xE0E20602L)

//
// MessageId: E_KSE_CHANNEL_IS_DISABLED
//
// MessageText:
//
//  The channel is disabled.
//
#define E_KSE_CHANNEL_IS_DISABLED        ((DWORD)0xE0E20603L)

//
// MessageId: E_KSE_ATTACHED_DATA_ITEM_NOT_EXIST
//
// MessageText:
//
//  The attached data item does not exist.
//
#define E_KSE_ATTACHED_DATA_ITEM_NOT_EXIST ((DWORD)0xE0E20701L)

//
// MessageId: E_KSE_BAD_TARGET_TYPE
//
// MessageText:
//
//  The target type is bad.
//
#define E_KSE_BAD_TARGET_TYPE            ((DWORD)0xE0E40001L)

//
// MessageId: E_KSE_NOT_SUPPORT_TARGET
//
// MessageText:
//
//  The target is unsupported.
//
#define E_KSE_NOT_SUPPORT_TARGET         ((DWORD)0xE0E40002L)

//
// MessageId: E_KSE_DODISTRIBUTE
//
// MessageText:
//
//  The DoDistribute function failed.
//
#define E_KSE_DODISTRIBUTE               ((DWORD)0xE0E40003L)

//
// MessageId: E_KSE_INVALID_TARGET_ID
//
// MessageText:
//
//  The target ID is invalid.
//
#define E_KSE_INVALID_TARGET_ID          ((DWORD)0xE0E40004L)

//
// MessageId: E_KSE_AUTORUN_DLL_FAIL
//
// MessageText:
//
//  The kaeautorunex.dll error.
//
#define E_KSE_AUTORUN_DLL_FAIL           ((DWORD)0xE0E40005L)

//
// MessageId: E_KSE_NOT_SUPPORTED_EVN_VAR
//
// MessageText:
//
//  This environment variable is not supported.
//
#define E_KSE_NOT_SUPPORTED_EVN_VAR      ((DWORD)0xE0E40006L)

//
// MessageId: E_KSE_NOT_IMPLEMENT_GETEVVBYSELF_FUNC
//
// MessageText:
//
//  This environment variable used a not implemented GetEVVBySelf function.
//
#define E_KSE_NOT_IMPLEMENT_GETEVVBYSELF_FUNC ((DWORD)0xE0E40007L)

//
// MessageId: E_KSE_FILE_NOT_EXIST
//
// MessageText:
//
//  The file does not exist.
//
#define E_KSE_FILE_NOT_EXIST             ((DWORD)0xE0E60001L)

//
// MessageId: E_KSE_CAN_NOT_OPEN_FILE
//
// MessageText:
//
//  The file can not be opened.
//
#define E_KSE_CAN_NOT_OPEN_FILE          ((DWORD)0xE0E60002L)

//
// MessageId: E_KSE_CAN_NOT_DELETE_FILE
//
// MessageText:
//
//  The file can not be deleted.
//
#define E_KSE_CAN_NOT_DELETE_FILE        ((DWORD)0xE0E60003L)

//
// MessageId: E_KSE_TEMP_FILE_OUT_OF_BUFF_SIZE
//
// MessageText:
//
//  The temp file buffer size is too small.
//
#define E_KSE_TEMP_FILE_OUT_OF_BUFF_SIZE ((DWORD)0xE0E60004L)

//
// MessageId: E_KSE_GET_FILE_TYPE
//
// MessageText:
//
//  GetFileType function failed.
//
#define E_KSE_GET_FILE_TYPE              ((DWORD)0xE0E70001L)

//
// MessageId: E_KSE_BWM_SCAN
//
// MessageText:
//
//  BWM scan failed.
//
#define E_KSE_BWM_SCAN                   ((DWORD)0xE0E70002L)

//
// MessageId: E_KSE_BWM_ENDLESS_RESCAN
//
// MessageText:
//
//  Maybe BWM had a bug, that lead endless rescaning.
//
#define E_KSE_BWM_ENDLESS_RESCAN         ((DWORD)0xE0E70003L)

//
// MessageId: E_KSE_BWMCALLER_INIT_FAILED
//
// MessageText:
//
//  KSEBWMCaller Init function failed.
//
#define E_KSE_BWMCALLER_INIT_FAILED      ((DWORD)0xE0E70004L)

//
// MessageId: E_KSE_EXTRACT_GET_ARC_TYPE
//
// MessageText:
//
//  GetArcType function failed.
//
#define E_KSE_EXTRACT_GET_ARC_TYPE       ((DWORD)0xE0E70101L)

//
// MessageId: E_KSE_EXTRACT_DO_EXTRACT
//
// MessageText:
//
//  DoExtract function failed.
//
#define E_KSE_EXTRACT_DO_EXTRACT         ((DWORD)0xE0E70102L)

//
// MessageId: E_KSE_EXTRACT_HEAD_FAIL
//
// MessageText:
//
//  Extract head failed.
//
#define E_KSE_EXTRACT_HEAD_FAIL          ((DWORD)0xE0E70103L)

//
// MessageId: E_KSE_EXTRACT_DATA_FAIL
//
// MessageText:
//
//  Extract data failed.
//
#define E_KSE_EXTRACT_DATA_FAIL          ((DWORD)0xE0E70104L)

//
// MessageId: E_KSE_UNPACK_GET_SHELL_TYPE
//
// MessageText:
//
//  GetShellType function failed.
//
#define E_KSE_UNPACK_GET_SHELL_TYPE      ((DWORD)0xE0E70201L)

//
// MessageId: E_KSE_UNPACK_DO_UNPACK
//
// MessageText:
//
//  DoUnpack function failed.
//
#define E_KSE_UNPACK_DO_UNPACK           ((DWORD)0xE0E70202L)

//
// MessageId: E_KSE_SDK_BAD_POINTER
//
// MessageText:
//
//  A bad pointer, like NULL, be passed to function.
//
#define E_KSE_SDK_BAD_POINTER            ((DWORD)0xE0E80001L)

//
// MessageId: E_KSE_SDK_NOT_INITIALIZE
//
// MessageText:
//
//  The SDK module has not been initialized.
//
#define E_KSE_SDK_NOT_INITIALIZE         ((DWORD)0xE0E80002L)

//
// MessageId: E_KSE_SDK_BAD_PARAMETER
//
// MessageText:
//
//  The function be passed a bad parameter.
//
#define E_KSE_SDK_BAD_PARAMETER          ((DWORD)0xE0E80003L)

//
// MessageId: E_KSE_SDK_INVALID_VERSION
//
// MessageText:
//
//  The version is not invalid.
//
#define E_KSE_SDK_INVALID_VERSION        ((DWORD)0xE0E80004L)

//
// MessageId: E_KSE_NET_DET_FAILED
//
// MessageText:
//
//  The net detecting failed.
//
#define E_KSE_NET_DET_FAILED             ((DWORD)0xE0E90001L)

 //	KWFS=0xF1:FACILITY_WFS
//
// MessageId: E_KW_NEW_OBJECT_FAILED
//
// MessageText:
//
//  It's failed to new object.
//
#define E_KW_NEW_OBJECT_FAILED           ((DWORD)0xE0F10001L)

//
// MessageId: E_KW_FDB_INITIALIZE_ERROR
//
// MessageText:
//
//  It's failed to initialize fdb.
//
#define E_KW_FDB_INITIALIZE_ERROR        ((DWORD)0xE0F10002L)

//
// MessageId: E_KW_FV_INITIALIZE_ERROR
//
// MessageText:
//
//  It's failed to initialize fv.
//
#define E_KW_FV_INITIALIZE_ERROR         ((DWORD)0xE0F10003L)

//
// MessageId: E_KW_CALL_FM_FAILE
//
// MessageText:
//
//  It's failed to call FM SDK.
//
#define E_KW_CALL_FM_FAILE               ((DWORD)0xE0F10004L)

//
// MessageId: E_KW_CREATE_DBFILE_ERROR
//
// MessageText:
//
//  It's failed to create DB file.
//
#define E_KW_CREATE_DBFILE_ERROR         ((DWORD)0xE0F10005L)

//
// MessageId: E_KWFV_START_THREAD_FAILED
//
// MessageText:
//
//  It's failed to start thread.
//
#define E_KWFV_START_THREAD_FAILED       ((DWORD)0xE0F10006L)

//
// MessageId: E_KW_NOT_INIT
//
// MessageText:
//
//  Need initialize first.
//
#define E_KW_NOT_INIT                    ((DWORD)0xE0F10007L)

//
// MessageId: E_KWFV_VERIFY_FILE_FAILED
//
// MessageText:
//
//  It's failed to add file to verify.
//
#define E_KWFV_VERIFY_FILE_FAILED        ((DWORD)0xE0F10008L)

//
// MessageId: E_KW_UNSUCCESS
//
// MessageText:
//
//  failed,unsuccess
//
#define E_KW_UNSUCCESS                   ((DWORD)0xE0F10009L)

//
// MessageId: E_KW_NOT_STOP
//
// MessageText:
//
//  Need stop first
//
#define E_KW_NOT_STOP                    ((DWORD)0xE0F1000AL)

 //	KWFDB=0xF2:FACILITY_WFS_FDB
//
// MessageId: E_KWFDB_INVALID_PARAMETER
//
// MessageText:
//
//  The parameter is invalid
//
#define E_KWFDB_INVALID_PARAMETER        ((DWORD)0xE0F20001L)

//
// MessageId: E_KWFDB_FILE_ALREADY_EXISTS
//
// MessageText:
//
//  The file already exists.
//
#define E_KWFDB_FILE_ALREADY_EXISTS      ((DWORD)0xE0F20002L)

//
// MessageId: E_KWFDB_GET_FILE_INFO
//
// MessageText:
//
//  It's failed to get file information.
//
#define E_KWFDB_GET_FILE_INFO            ((DWORD)0xE0F20003L)

//
// MessageId: E_KWFDB_FILE_NOT_FOUND
//
// MessageText:
//
//  The white file system connot find the file specified.
//
#define E_KWFDB_FILE_NOT_FOUND           ((DWORD)0xE0F20004L)

//
// MessageId: E_KWFDB_FILE_TOO_LARGE
//
// MessageText:
//
//  The file size exceeds the limit allowed and cannot be saved.
//
#define E_KWFDB_FILE_TOO_LARGE           ((DWORD)0xE0F20005L)

//
// MessageId: E_KWFDB_ADD_FILE_FAILED
//
// MessageText:
//
//  It's failed to add the file information into file DB.
//
#define E_KWFDB_ADD_FILE_FAILED          ((DWORD)0xE0F20006L)

//
// MessageId: E_KWFDB_FIND_FILE_FAILED
//
// MessageText:
//
//  It's failed to find the file information in file DB.
//
#define E_KWFDB_FIND_FILE_FAILED         ((DWORD)0xE0F20007L)

//
// MessageId: E_KWFDB_DELETE_FILE_FAILED
//
// MessageText:
//
//  it's failed to delete file information.
//
#define E_KWFDB_DELETE_FILE_FAILED       ((DWORD)0xE0F20008L)

//
// MessageId: E_KWFDB_UPDATE_FILE_FAILED
//
// MessageText:
//
//  it's failed to update file information.
//
#define E_KWFDB_UPDATE_FILE_FAILED       ((DWORD)0xE0F20009L)

//
// MessageId: E_KWFDB_CALL_DB_FAILED
//
// MessageText:
//
//  It's failed to call DB.
//
#define E_KWFDB_CALL_DB_FAILED           ((DWORD)0xE0F2000AL)

//
// MessageId: E_KWFDB_DB_NOT_RETURN_DATA
//
// MessageText:
//
//  The calling of DB is return none data.
//
#define E_KWFDB_DB_NOT_RETURN_DATA       ((DWORD)0xE0F2000BL)

//
// MessageId: E_KWFDB_DB_EMPTY
//
// MessageText:
//
//  The file information DB is empty.
//
#define E_KWFDB_DB_EMPTY                 ((DWORD)0xE0F2000CL)

//
// MessageId: E_KWFDB_FILE_INFO_EXPIRE
//
// MessageText:
//
//  The file information in DB is out of time.
//
#define E_KWFDB_FILE_INFO_EXPIRE         ((DWORD)0xE0F2000DL)

//
// MessageId: E_KWFV_OPEN_FILE_FAILED
//
// MessageText:
//
//  It is failed to open file.
//
#define E_KWFV_OPEN_FILE_FAILED          ((DWORD)0xE0F2000EL)

//
// MessageId: E_KWFDB_END_OF_DB
//
// MessageText:
//
//  It is end of DB
//
#define E_KWFDB_END_OF_DB                ((DWORD)0xE0F2000FL)

//
// MessageId: E_KWFDB_FILE_NEED_VERIFY
//
// MessageText:
//
//  Please verify the file first
//
#define E_KWFDB_FILE_NEED_VERIFY         ((DWORD)0xE0F20010L)

//
// MessageId: E_KWFDB_NEED_START
//
// MessageText:
//
//  Please start the thread first
//
#define E_KWFDB_NEED_START               ((DWORD)0xE0F20011L)

//
// MessageId: E_KWFDB_COMPRESS_FAILED
//
// MessageText:
//
//  Compress out of bounds
//
#define E_KWFDB_COMPRESS_FAILED          ((DWORD)0xE0F20012L)

//	KWFDB=0xF3:FACILITY_WFS_SP
//
// MessageId: E_KWSP_INITIALIZE_FAILED
//
// MessageText:
//
//  It's failed to initialize .
//
#define E_KWSP_INITIALIZE_FAILED         ((DWORD)0xE0F30001L)

//
// MessageId: E_KWSP_START_FAILED
//
// MessageText:
//
//  It's failed to start .
//
#define E_KWSP_START_FAILED              ((DWORD)0xE0F30002L)

//KxEFM=0xA1:FACILITY_FILEMON
//
// MessageId: E_KXEFILEMON_LOADDRIVER_FAILED
//
// MessageText:
//
//  It's failed to load driver.
//
#define E_KXEFILEMON_LOADDRIVER_FAILED   ((DWORD)0xE0A10001L)

//
// MessageId: E_KXEFILEMON_GETSHAREMEM_FAILED
//
// MessageText:
//
//  It's failed to get share memory.
//
#define E_KXEFILEMON_GETSHAREMEM_FAILED  ((DWORD)0xE0A10002L)

//
// MessageId: E_KXEFILEMON_INITSCANENGINE_FAILED
//
// MessageText:
//
//  It's failed to init scan engine.
//
#define E_KXEFILEMON_INITSCANENGINE_FAILED ((DWORD)0xE0A10003L)

//
// MessageId: E_KXEFILEMON_ERROR_INVALID_PARAMETER
//
// MessageText:
//
//  Parameter is invlided.
//
#define E_KXEFILEMON_ERROR_INVALID_PARAMETER ((DWORD)0xE0A10004L)

//
// MessageId: E_KXEFILEMON_ERROR_INITIALIZE_FAILED
//
// MessageText:
//
//  It's failed to initialize.
//
#define E_KXEFILEMON_ERROR_INITIALIZE_FAILED ((DWORD)0xE0A10005L)

//
// MessageId: E_KXEFILEMON_ERROR_UNINITIALIZE_FAILED
//
// MessageText:
//
//  It's failed to uninitialize.
//
#define E_KXEFILEMON_ERROR_UNINITIALIZE_FAILED ((DWORD)0xE0A10006L)

//
// MessageId: E_KXEFILEMON_STARTFILEMON_FAILED
//
// MessageText:
//
//  It's failed to startfilemon.
//
#define E_KXEFILEMON_STARTFILEMON_FAILED ((DWORD)0xE0A10007L)

//
// MessageId: E_KXEFILEMON_IS_ALREADY_STOP
//
// MessageText:
//
//  It's already stop.
//
#define E_KXEFILEMON_IS_ALREADY_STOP     ((DWORD)0xE0A10008L)

//
// MessageId: E_KXEFILEMON_IS_NOT_INIT
//
// MessageText:
//
//  It's not init.
//
#define E_KXEFILEMON_IS_NOT_INIT         ((DWORD)0xE0A10009L)

//
// MessageId: E_KXEFILEMON_ADDFILTERPROCESSID_ERROR
//
// MessageText:
//
//  It's failed to add filter processid.
//
#define E_KXEFILEMON_ADDFILTERPROCESSID_ERROR ((DWORD)0xE0A1000AL)

//
// MessageId: E_KXEFILEMON_REMOVFILTERPROCESSID_ERROR
//
// MessageText:
//
//  It's failed to remove filter processid.
//
#define E_KXEFILEMON_REMOVFILTERPROCESSID_ERROR ((DWORD)0xE0A1000BL)

//
// MessageId: E_KXEFILEMON_WRITETEMPWHITELIST_ERROR
//
// MessageText:
//
//  It's failed to write temp whitelist.
//
#define E_KXEFILEMON_WRITETEMPWHITELIST_ERROR ((DWORD)0xE0A1000CL)

//
// MessageId: E_KXEFILEMON_FREEMD5TABLE_ERROR
//
// MessageText:
//
//  It's failed to free md5table.
//
#define E_KXEFILEMON_FREEMD5TABLE_ERROR  ((DWORD)0xE0A1000DL)

//KxEFMSP=0xA2:FACILITY_FILEMON_SP
//
// MessageId: E_KXEFILEMONSP_INITIALIZESERVICE_FAILED
//
// MessageText:
//
//  It's failed to initializeservice.
//
#define E_KXEFILEMONSP_INITIALIZESERVICE_FAILED ((DWORD)0xE0A20001L)

//
// MessageId: E_KXEFILEMONSP_STARTFILEMON_FAILED
//
// MessageText:
//
//  It's failed to startfilemon.
//
#define E_KXEFILEMONSP_STARTFILEMON_FAILED ((DWORD)0xE0A20002L)

//
// MessageId: E_KXEFILEMONSP_STOPFILEMON_FAILED
//
// MessageText:
//
//  It's failed to stopfilemon.
//
#define E_KXEFILEMONSP_STOPFILEMON_FAILED ((DWORD)0xE0A20003L)

//
// MessageId: E_KXEFILEMONSP_FILEMONINTERFACE_ISNULL
//
// MessageText:
//
//  The FileMonInterface is null.
//
#define E_KXEFILEMONSP_FILEMONINTERFACE_ISNULL ((DWORD)0xE0A20004L)

//
// MessageId: E_KXEFILEMONSP_QUERYFILEMON_ERROR
//
// MessageText:
//
//  It's failed to queryfilemon.
//
#define E_KXEFILEMONSP_QUERYFILEMON_ERROR ((DWORD)0xE0A20005L)

//
// MessageId: E_KXEFILEMONSP_SETFILEMONSETTING_ERROR
//
// MessageText:
//
//  It's failed to setfilemon.
//
#define E_KXEFILEMONSP_SETFILEMONSETTING_ERROR ((DWORD)0xE0A20006L)

//
// MessageId: E_KXEFILEMONSP_ADDFILTERPROCESSID_ERROR
//
// MessageText:
//
//  It's failed to addfilterprocessid.
//
#define E_KXEFILEMONSP_ADDFILTERPROCESSID_ERROR ((DWORD)0xE0A20007L)

//
// MessageId: E_KXEFILEMONSP_REMOVEFILTERPROCESSID_ERROR
//
// MessageText:
//
//  It's failed to removefilterprocessid.
//
#define E_KXEFILEMONSP_REMOVEFILTERPROCESSID_ERROR ((DWORD)0xE0A20008L)

//
// MessageId: E_KXEFILEMONSP_QUERYFILEMONSETTING_ERROR
//
// MessageText:
//
//  It's failed to queryfilemonsetting.
//
#define E_KXEFILEMONSP_QUERYFILEMONSETTING_ERROR ((DWORD)0xE0A20009L)

//
// MessageId: E_KXEFILEMONSP_QUERYFILEMONSCANINFO_ERROR
//
// MessageText:
//
//  It's failed to QueryFileMonScanInfo.
//
#define E_KXEFILEMONSP_QUERYFILEMONSCANINFO_ERROR ((DWORD)0xE0A2000AL)

//
// MessageId: E_KXEFILEMONSP_NOTIFYENGINEUPDATED_ERROR
//
// MessageText:
//
//  It's failed to NotifyEngineUpdated
//
#define E_KXEFILEMONSP_NOTIFYENGINEUPDATED_ERROR ((DWORD)0xE0A2000BL)

//
// MessageId: E_KXEFILEMONSP_CONFIG_TRUST_PATH_ERROR
//
// MessageText:
//
//  It's failed to ConfigTrustPath
//
#define E_KXEFILEMONSP_CONFIG_TRUST_PATH_ERROR ((DWORD)0xE0A2000CL)

//
// MessageId: E_KXEFILEMONSP_CONFIG_TRUST_FILE_TYPE_ERROR
//
// MessageText:
//
//  It's failed to ConfigTrustFileType
//
#define E_KXEFILEMONSP_CONFIG_TRUST_FILE_TYPE_ERROR ((DWORD)0xE0A2000DL)

//
// MessageId: E_KXEFILEMON_SET_WORK_MODE_ERROR
//
// MessageText:
//
//  It's failed to SetWorkMode
//
#define E_KXEFILEMON_SET_WORK_MODE_ERROR ((DWORD)0xE0A2000EL)

//
// MessageId: E_KXEFILEMON_SWITCH_WORK_MODE_ERROR
//
// MessageText:
//
//  It's failed to SwitchWorkMode
//
#define E_KXEFILEMON_SWITCH_WORK_MODE_ERROR ((DWORD)0xE0A2000FL)

 // KxESCSP
//
// MessageId: E_KXE_SCSP_LOAD_XML_FAILED
//
// MessageText:
//
//  It's failed to load the xml file.
//
#define E_KXE_SCSP_LOAD_XML_FAILED       ((DWORD)0xE0B10001L)

//
// MessageId: E_KXE_SCSP_BAD_XML_FILE
//
// MessageText:
//
//  The xml file is bad.
//
#define E_KXE_SCSP_BAD_XML_FILE          ((DWORD)0xE0B10002L)

//
// MessageId: E_KXE_SCSP_NOT_LOAD_CFG_FILE
//
// MessageText:
//
//  The config file has not been loaded.
//
#define E_KXE_SCSP_NOT_LOAD_CFG_FILE     ((DWORD)0xE0B10003L)

//
// MessageId: E_KXE_SCSP_NOT_LOAD_REG_INFO
//
// MessageText:
//
//  The registered product info has not been loaded.
//
#define E_KXE_SCSP_NOT_LOAD_REG_INFO     ((DWORD)0xE0B10004L)

//
// MessageId: E_KXE_SCSP_UNKNOWN_PRODUCT_ID
//
// MessageText:
//
//  The product id is unknowned.
//
#define E_KXE_SCSP_UNKNOWN_PRODUCT_ID    ((DWORD)0xE0B10005L)

//
// MessageId: E_KXE_SCSP_UNKNOWN_VIP_FUNC_ID
//
// MessageText:
//
//  The vip function id is unknowned.
//
#define E_KXE_SCSP_UNKNOWN_VIP_FUNC_ID   ((DWORD)0xE0B10006L)

//
// MessageId: E_KXE_SCSP_INSTALL_PROCESS_EXIST
//
// MessageText:
//
//  The installing process already exist.
//
#define E_KXE_SCSP_INSTALL_PROCESS_EXIST ((DWORD)0xE0B10007L)

//
// MessageId: E_KXE_SCSP_CONFLICT_CFG_FILES
//
// MessageText:
//
//  The config file of the product is conflict with the config file of the Security Center.
//
#define E_KXE_SCSP_CONFLICT_CFG_FILES    ((DWORD)0xE0B10008L)

//
// MessageId: E_KXE_SCSP_BAD_PARAMETER
//
// MessageText:
//
//  The parameter is invalid.
//
#define E_KXE_SCSP_BAD_PARAMETER         ((DWORD)0xE0B10009L)

//
// MessageId: E_KXE_SCSP_CFG_NOT_INSTALLED_PRODUCT
//
// MessageText:
//
//  The product which be configuring, has not been installed.
//
#define E_KXE_SCSP_CFG_NOT_INSTALLED_PRODUCT ((DWORD)0xE0B1000AL)

 // KxESRSP
//
// MessageId: E_KXESRSP_UUID_CONFLICT
//
// MessageText:
//
//  The UUID has been used.
//
#define E_KXESRSP_UUID_CONFLICT          ((DWORD)0xE0A40001L)

//
// MessageId: E_KXESRSP_NOT_INITED
//
// MessageText:
//
//  The object has not been inited;
//
#define E_KXESRSP_NOT_INITED             ((DWORD)0xE0A40002L)

//
// MessageId: E_KXESRSP_INDEX_OUT_OF_RANGE
//
// MessageText:
//
//  The index is out of range.
//
#define E_KXESRSP_INDEX_OUT_OF_RANGE     ((DWORD)0xE0A40003L)

//
// MessageId: E_KXESRSP_USER_STOP
//
// MessageText:
//
//  Stop by user.
//
#define E_KXESRSP_USER_STOP              ((DWORD)0xE0A40004L)

//
// MessageId: E_KXESRSP_SESSION_NUM_MAX_LIMIT
//
// MessageText:
//
//  The count of sessions has to be the max limit.
//
#define E_KXESRSP_SESSION_NUM_MAX_LIMIT  ((DWORD)0xE0A40005L)

//
// MessageId: E_KXEBASIC_SERVICE_PROVIDER_STARTED
//
// MessageText:
//
//  The service provider is started.
//
#define E_KXEBASIC_SERVICE_PROVIDER_STARTED ((DWORD)0xE0A50001L)

