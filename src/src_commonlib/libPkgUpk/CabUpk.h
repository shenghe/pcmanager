#pragma once
#include "include/fdi.h"
#include "Utils.h"

LPCSTR return_fdi_error_string(int err);
class CCabExtract 
{
public:
	BOOL Extract(LPCTSTR szCabFilename, LPCTSTR szDestPath);
	BOOL _Extract( char * cabinet_fullpath );
	INT _NotifyCallBack(FDINOTIFICATIONTYPE fdint, PFDINOTIFICATION pfdin);

public:
	Files m_files;
	
protected:
	CStringA m_strDestPath;
};