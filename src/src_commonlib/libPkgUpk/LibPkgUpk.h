#pragma once
#include <vector>
#include <string>

typedef std::vector<CString> Files;
enum ENUM_ExtractErrorCode {
	EE_SUCCESS,
	EE_EXE,
	EE_MSP,
	EE_CAB,
	EE_PATH,
	EE_NOFILE,
};

ENUM_ExtractErrorCode	ExtractOfficePackage(LPCTSTR szPkgFilename, LPCTSTR szDestPath, Files &files);
BOOL					IsLibPkgUpkValid();