/**
* @brief			INI文件编辑类
* @author		yaojianfeng@kingsoft.com
*/
#pragma once
#include <string>
#include <windows.h>
using std::string;
using std::wstring;

/**
* @brief			   该类负责进行INI文件操作处理
* @detail				其中strSec参数指代“段”，即由"[]"包含起来的区域，strKey参数指代“键”，即
*						赋值操作的左值，strVal指代右值
*						对于任何写入操作，非特别说明，如果指定文件不存在，就会创建文件并写入
*						对于Incr*操作，如果对应文件和键不存在，则会创建文件和键并执行递增操作，创建键后初始化键值为0，
*						Incr结果为1
* @warning				对任何获得段值时限定值字符个数为65535（含NULL），如果超过该个数则直接返回空串
*/

class IniEditor
{
public :

    IniEditor( const wstring& wstrFilePath ) : m_wstrFile( wstrFilePath ) {}

	IniEditor() {}

	~IniEditor() {}

	const wstring& GetFile() const { return m_wstrFile; }

	BOOL SetFile(const wstring& wstrFilePath) { m_wstrFile = wstrFilePath; return TRUE;}

	wstring ReadString(
		const wstring& strSec, 
		const wstring& strKey ) const;

    BOOL WriteString(
		const wstring& strSec, 
		const wstring& strKey, 
		const wstring& strVal );


	BOOL ReadDWORD( 
		const wstring& strSec, 
		const wstring& strKey, 
		int& dwVal) const;

	BOOL WriteDWORD(
		const wstring& strSec, 
		const wstring& strKey, 
		DWORD dwVal);

	BOOL IncrDWORD(const wstring& strSec, const wstring& strKey);

	BOOL Del( const wstring& strSec, const wstring& strKey );
	
	BOOL DelFile();

private :

    wstring m_wstrFile;
};
