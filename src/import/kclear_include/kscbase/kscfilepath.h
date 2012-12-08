/********************************************************************
	created:	2010/03/06
	created:	6:3:2010   10:28
	filename: 	kscfilepath.h
	author:		Jiang Fengbing
	
	purpose:	
*********************************************************************/

#ifndef KSCFILEPATH_INC_
#define KSCFILEPATH_INC_

//////////////////////////////////////////////////////////////////////////

#include "kscbase.h"

//////////////////////////////////////////////////////////////////////////

// TODO: 暂时不提供
// typedef enum tagKPathType {
// 	FILE_EXE,			// 当前进程的exe的路径
// 	FILE_MODULE,		// 获得指定模块的路径
// 	DIR_WINDOWS,		// 获得Windows的路径，通常为"c:\windows"
// 	DIR_SYSTEM,			// 获得system的路径，通常为"c:\windows\system32"
// 	DIR_PROGRAM_FILES,	// 获得Program的路径，通常为"c:\program files"
// 	DIR_APP_DATA,		// 个人的Application Data目录
// } KPathType;

//////////////////////////////////////////////////////////////////////////

class KFilePath 
{
public:
	KFilePath() {}
	KFilePath(const KFilePath& path) : m_strPath(path.m_strPath) {}
	explicit KFilePath(const std::wstring& strPath) : m_strPath(strPath) {}

	KFilePath& operator=(const KFilePath& path) 
	{
		m_strPath = path.m_strPath;
		return *this;
	}

	KFilePath& operator=(const std::wstring& strPath) 
	{
		m_strPath = strPath;
		return *this;
	}

	static const wchar_t m_cSeparators[];
	static const wchar_t m_cCurrentDirectory[];
	static const wchar_t m_cParentDirectory[];
	static const wchar_t m_cExtensionSeparator;

	// 获得Windows目录
	static KFilePath GetWindowsPath();

	// 获得System32目录
	static KFilePath GetSystemPath();

	// 获得指定的系统目录
	static KFilePath GetFolderPath(int nFolder);

	// 字符串比较 ==
	bool operator==(const KFilePath& path) const;

	// 字符串比较 !=
	bool operator!=(const KFilePath& path) const;

	// 为STL容器准备的less
	bool operator<(const KFilePath& path) const;

	// 获得路径字符串
	const std::wstring& value() const;

	// 获得路径字符串的hash值
	size_t hash_value() const;

	// 获得路径字符数组
	operator const wchar_t* () const;

	// 是否为空
	bool empty() const;

	// 分割路径
	void GetComponents(std::vector<std::wstring>& vComponents) const;

	
	bool IsParent(const KFilePath& child) const;

	// 获得目录的路径
	KFilePath DirName() const;

	// 获得文件名的路径
	KFilePath BaseName() const;

	// 获得扩展名(包含.)
	std::wstring GetExtension() const;

	// 添加扩展名
	void AddExtension(const std::wstring& strExtension);

	// 移除扩展名
	void RemoveExtension();

	// 替换扩展名
	void ReplaceExtension(const std::wstring& extension);

	// 扩展名是否一致(包含.)
	bool MatchesExtension(const std::wstring& extension) const;

	// 是否为目录
	bool IsDirectory() const;

	// 追加字符串
	void Append(const std::wstring& strComponent);
	// 追加路径
	void Append(const KFilePath& component);

	// 移除文件名
	void RemoveFileSpec();

	// 判断是否存在
	bool FileExists();

	// 是否为绝对目录
	bool IsAbsolute() const;

	// 在尾部添加反斜杠
	void AddBackslash();

	// 在尾部删除反斜杠
	void RemoveBackslash();

	// 删除头部反斜杠
	void RemoveHeadBackslash();

	// 去除引号
	void RemoveQuotes();

	void Attach(std::wstring strFileName);
	std::wstring Detach();

	// 从HMODULE获得路径
	static KFilePath GetFilePath(HMODULE hModule);

	
	static bool IsSeparator(wchar_t character);
	static bool IsDot(wchar_t character);

	KFilePath StripTrailingSeparators() const;

private:
	void StripTrailingSeparatorsInternal();

	std::wstring m_strPath;
};

//////////////////////////////////////////////////////////////////////////
// 为了可以把FilePath放到哈希表里面

namespace stdext 
{
	inline size_t hash_value(const KFilePath& f) 
	{
		return f.hash_value();
	}

}  // namespace stdext

//////////////////////////////////////////////////////////////////////////

#endif	// KSCFILEPATH_INC_
