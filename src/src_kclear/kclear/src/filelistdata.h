// FileListData.h: interface for the CFileListData class.
//
//////////////////////////////////////////////////////////////////////

#pragma once


#include "DeleteFileTraverse.h"
#include "filestruct.h"
#include "trashdefine.h"

typedef struct tag_TRASHFILEITEM
{
    CString   strFileName;
    ULONGLONG ulFileSize;
    bool      bIsClear;
    bool      bIsVedio;//是否完整视频
    HTREEITEM hItem;
    int ParentDirId;

    tag_TRASHFILEITEM() : ulFileSize(0)
    {}
    
}TRASHFILEITEM;

typedef struct tag_TRASHFILELIST {
    
    std::vector<TRASHFILEITEM> itemArray;

    ULONGLONG ulAllFileSize;

	CString strfileList;

    tag_TRASHFILELIST() : ulAllFileSize(0)
    {
		strfileList = L"";
	}

    tag_TRASHFILELIST(const tag_TRASHFILELIST& cpy)
    {
       
        std::vector<tag_TRASHFILEITEM>::const_iterator iter;
        for (iter=cpy.itemArray.begin();iter!=cpy.itemArray.end();iter++)
            itemArray.push_back(*iter);
        ulAllFileSize = cpy.ulAllFileSize;
		strfileList = cpy.strfileList;
    }

    const tag_TRASHFILELIST& operator = (const tag_TRASHFILELIST& cpy)
    {
       
        std::vector<tag_TRASHFILEITEM>::const_iterator iter;
        for (iter=cpy.itemArray.begin();iter!=cpy.itemArray.end();iter++)
            itemArray.push_back(*iter);
        ulAllFileSize = cpy.ulAllFileSize;
		strfileList = cpy.strfileList;
        return *this;
    }

}TRASHFILELIST;

class CFileListData  
{
public:

	CFileListData();

	virtual ~CFileListData();

    __int64 AddTrashFile(int nIndex, LPFINDFILEDATA pFileData);
    
public:
    std::vector<TRASHFILELIST> m_itemArray;
    __int64 m_nTotalsize;
};

