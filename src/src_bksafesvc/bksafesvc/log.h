#ifndef __LOG_H__
#define __LOG_H__


class CScanInfoLog
{
public:
	static HRESULT SaveScanInfo( BK_SCAN_INFO& ScanInfo);
	static HRESULT LoadScanInfo( BK_SCAN_INFO& ScanInfo );
private:
	static BOOL _GetLogFilePath( CString& FilePath, DWORD ScanMode  );
};


#endif //__LOG_H__