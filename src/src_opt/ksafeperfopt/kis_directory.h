////////////////////////////////////////////////////////////////////////////////
//      
//      Kingsoft File for kiscomm
//      
//      File      : kis_directory.h
//      Version   : 1.0
//      Comment   : 文件夹操作类
//      
//      Create at : 2008-3-5
//      Create by : liupeng
//      
////////////////////////////////////////////////////////////////////////////////

#pragma once 
#include <vector>
using namespace std;


namespace kis 
{

// 文件枚举器
class KFileIterator
{
private:
	ATL::CString			m_strSearch;
	ATL::CString			m_strPath;
	BOOL			m_bSearchRecursive;
	BOOL			m_bFileOnly;
	BOOL			m_bSubDirFirst;
	HANDLE			m_hCurDir;

	HANDLE			m_hDirSearch;
	KFileIterator*	m_pCurDirIter;
	WIN32_FIND_DATA	m_findData;

public:
	KFileIterator(LPCTSTR pstrSearch, ATL::CString strPath, BOOL bSearchRecursive, BOOL bFileOnly = TRUE,
				BOOL bSubDirFirst = FALSE) :
			m_strSearch(pstrSearch),
			m_strPath(strPath),
			m_bSearchRecursive(bSearchRecursive),
			m_bFileOnly(bFileOnly),
			m_bSubDirFirst(bSubDirFirst),
			m_hCurDir(NULL),
			m_hDirSearch(NULL),
			m_pCurDirIter(NULL)
	{
		if (m_strPath[m_strPath.GetLength() - 1] != L'\\')
			m_strPath += L"\\";

		ClearFindData();
	}

	~KFileIterator()
	{
		if (m_pCurDirIter)
		{
			delete m_pCurDirIter;
			m_pCurDirIter = NULL;
		}
		if (m_hCurDir && (m_hCurDir != INVALID_HANDLE_VALUE))
		{
			::FindClose(m_hCurDir);
			m_hCurDir = NULL;
		}
		if (m_hDirSearch && m_hDirSearch != INVALID_HANDLE_VALUE)
		{
			::FindClose(m_hDirSearch);
			m_hDirSearch = NULL;
		}
	}

	BOOL	Next()
	{
		if (m_bSubDirFirst)
		{
			//ASSERT(m_bSearchRecursive);

			if (SearchSubDirs())
				return TRUE;

			return SearchCurDir();
		}
		else
		{
			if (SearchCurDir())
				return TRUE;

			if (!m_bSearchRecursive)
				return FALSE;

			return SearchSubDirs();
		}
	}

	ATL::CString GetFileName()
	{
		if (m_hCurDir && m_hCurDir != INVALID_HANDLE_VALUE)
			return m_strPath + ATL::CString(m_findData.cFileName);

		if (!m_bSearchRecursive || !m_pCurDirIter)
		{
			// ASSERT(!"已经没有文件可以获取了!");
			return ATL::CString(_T(""));
		}
		return m_pCurDirIter->GetFileName();
	}

	// 当前是文件还是文件夹
	BOOL	IsDirectory()
	{
		if (m_hCurDir && m_hCurDir != INVALID_HANDLE_VALUE)
			return m_findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
		
		if (!m_bSearchRecursive || !m_pCurDirIter)
		{
			// ASSERT(!"已经没有文件可以获取了!");
			return FALSE;
		}
		return m_pCurDirIter->IsDirectory();
	}

private:
	// 搜索当前的文件夹，不是子文件夹
	BOOL	SearchCurDir()
	{
		// 本文件夹下面搜索
		if (m_hCurDir == NULL)
		{
			m_hCurDir = ::FindFirstFile(m_strPath + m_strSearch, &m_findData);
			if (m_hCurDir != INVALID_HANDLE_VALUE)
			{
				if (IsFindAttr(&m_findData))
					return TRUE;
				// 如果第一个找到的是文件夹名，那么还需要继续查找
				if (FindFile())
					return TRUE;
			}
		}

		if (m_hCurDir != INVALID_HANDLE_VALUE)
		{
			if (FindFile())
				return TRUE;
		}

		return FALSE;
	}

	// 搜索子文件夹
	BOOL	SearchSubDirs()
	{
		if (m_hDirSearch == NULL)
		{
			m_hDirSearch = ::FindFirstFile(m_strPath + _T("*"), &m_findData);
			if (m_hDirSearch != INVALID_HANDLE_VALUE)
			{
				if (IsDir(&m_findData))
				{
					if (SearchNewSubDir(m_strPath + ATL::CString(m_findData.cFileName) + L"\\"))
						return TRUE;
				}
				while (FindDir())
				{
					if (SearchNewSubDir(m_strPath + ATL::CString(m_findData.cFileName) + L"\\"))
						return TRUE;
				}
			}
		}

		// 某个子文件夹正在搜索中
		if (m_hDirSearch != INVALID_HANDLE_VALUE)
		{
			// 当前子文件夹找到
			if (SubDirNext())
				return TRUE;

			// 当前子文件夹没有，继续搜索其他子文件夹
			while (FindDir())
			{
				if (SearchNewSubDir(ATL::CString(m_findData.cFileName)))
					return TRUE;
			} 
		}

		return FALSE;
	}

	BOOL	SearchNewSubDir(ATL::CString strDir)
	{
		//ASSERT(m_pCurDirIter == NULL);
		m_pCurDirIter = new KFileIterator(m_strSearch, strDir, m_bSearchRecursive, m_bFileOnly);
		return SubDirNext();
	}

	void	ClearFindData()
	{
		memset(&m_findData, 0, sizeof(m_findData));
	}

	// 是否是需要找的文件名
	BOOL	IsFindAttr(WIN32_FIND_DATA *pFindData)
	{
		if (pFindData == NULL)
			return FALSE;

		if (_wcsicmp(pFindData->cFileName, L".") == 0 || _wcsicmp(pFindData->cFileName, L"..") == 0)
			return FALSE;

		if (!m_bFileOnly)
			return TRUE;

		return !(pFindData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
	}

	// 查找文件
	BOOL	FindFile()
	{
		BOOL bFind = FALSE;
		while (!bFind)
		{
			ClearFindData();
			if (::FindNextFile(m_hCurDir, &m_findData))
			{
				if (IsFindAttr(&m_findData))
					return TRUE;
			}
			else
			{
				::FindClose(m_hCurDir);
				m_hCurDir = INVALID_HANDLE_VALUE;
				break;
			}
		}
		return FALSE;
	}

	// 是否是文件夹
	BOOL	IsDir(WIN32_FIND_DATA *pFindData)
	{
		if (pFindData == NULL)
			return FALSE;

		if (_wcsicmp(pFindData->cFileName, L".") == 0 || _wcsicmp(pFindData->cFileName, L"..") == 0)
			return FALSE;

		return pFindData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
	}

	// 查找文件夹
	BOOL	FindDir()
	{
		BOOL bFind = FALSE;
		while (!bFind)
		{
			ClearFindData();
			if (::FindNextFile(m_hDirSearch, &m_findData))
			{
				if (IsDir(&m_findData))
					return TRUE;
			}
			else
			{
				::FindClose(m_hDirSearch);
				m_hDirSearch = INVALID_HANDLE_VALUE;
				break;
			}
		}
		return FALSE;
	}

	// 搜索当前子文件夹
	BOOL	SubDirNext()
	{
		//ASSERT(m_pCurDirIter);
		if (m_pCurDirIter->Next())
			return TRUE;

		delete m_pCurDirIter;
		m_pCurDirIter = NULL;
		return FALSE;
	}

};

/*
// 文件夹操作器
class KDirectory
{
public:
	KDirectory()		{}

	// 创建一个目录，如果在给定的路径中某些dir不存在，则自动创建它们
	static BOOL	Create(ATL::CString strPath)
	{
		KPath path(strPath);
		if (path.Exists())
			return TRUE;

		path.RemoveBackslash();
		if (!Create(CString(path.GetPath())))
			return FALSE;

		::CreateDirectory(path, NULL);
		return TRUE;
	}

	// 删除一个目录
	static BOOL	Delete(ATL::CString strPath)
	{
		BOOL bRet = TRUE;
		// 先删除所有的file
		KFileIterator iter(CString(_T("*")), strPath, TRUE, TRUE, TRUE);
		while (iter.Next())
		{
			CString strFileName = iter.GetFileName();
			if (!DeleteFile(strFileName))
			{
				bRet = FALSE;
			}
		}
		if (!bRet)
			return bRet;

		// 删除所有的dir
		KFileIterator iter_dir(CString(_T("*")), strPath, TRUE, FALSE, TRUE);
		while (iter.Next())
		{
			CString strFileName = iter.GetFileName();
			if (!RemoveDirectory(strFileName))
				bRet = FALSE;
		}
		return bRet;
	}

	// 拷贝一个目录，及目录下的文件
	static BOOL Copy(ATL::CString strFrom, ATL::CString strTo, BOOL bOverwriteExist)
	{
		KFileIterator iter(ATL::CString(_T("*")), strFrom, TRUE, FALSE, FALSE);
		while (iter.Next())
		{
			KPath path(iter.GetFileName());
			path.MakeRelative(KPath(strFrom));

			KPath pathFileTo(strTo);
			pathFileTo.Append(path);

			if (iter.IsDirectory())
			{
				if (!Create(pathFileTo.GetStr()))
				{
					//ASSERT(!"创建文件夹失败!");
					return FALSE;
				}
			}
			else
			{
				if (pathFileTo.Exists() && !bOverwriteExist)
					continue;
				if (!CopyFile(iter.GetFileName(), pathFileTo, FALSE))
				{
					//ASSERT(!"拷贝文件失败!");
					return FALSE;
				}
			}
		}
		return TRUE;
	}
};
*/
} // namespace kis 