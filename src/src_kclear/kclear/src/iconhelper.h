#pragma once

//////////////////////////////////////////////////////////////////////////

class CIconCache
{
public:
    CIconCache()
    {
        m_nLength = 0;
    }

    ~CIconCache()
    {
    }

    class IconInfo 
    {
    public:
        IconInfo(CString strExt, HICON hIcon)
        {
            m_strExt = strExt;
            m_hIcon = DuplicateIcon(NULL, hIcon);
        }

        IconInfo(const IconInfo& iconInfo)
            : m_strExt(iconInfo.m_strExt)
        {
            m_hIcon = DuplicateIcon(NULL, iconInfo.m_hIcon);
        }

        ~IconInfo()
        {
            DestroyIcon(m_hIcon);
        }

        CString m_strExt;
        HICON m_hIcon;
    };

    HICON GetIcon(const CString& strFilePath)
    {
        CString strExt;
        int nFlag1, nFlag2;

        nFlag1 = strFilePath.ReverseFind(_T('\\'));
        nFlag2 = strFilePath.ReverseFind(_T('.'));
        if (nFlag1 != -1 && nFlag2 != -1)
        {
            if (nFlag1 < nFlag2)
                strExt = strFilePath.Right(strFilePath.GetLength() - nFlag2);
        }

        if (-1 == nFlag1 && nFlag1 != -1)
        {
            strExt = strFilePath.Right(strFilePath.GetLength() - nFlag2);
        }

        return GetIcon(strFilePath, strExt);
    }

    HICON GetIcon(const CString& strPath, const CString& strExt)
    {
        HICON hIcon = NULL;
        HICON hOldIcon = NULL;

        if (strExt.CompareNoCase(_T(".exe")) == 0 ||
            strExt.CompareNoCase(_T(".scr")) == 0 ||
            strExt.CompareNoCase(_T(".lnk")) == 0)
        {
            goto clean0;
        }
        
        GetIcon(strExt, hIcon);
        if (hIcon)
            goto clean0;
        
        hOldIcon = GetIconFromFilePath(strPath);
        if (!hOldIcon)
        {
            hIcon = KAppRes::Instance().GetIcon("IDI_UNKNOWN_SMALL");
            goto clean0;
        }
        else
        {
            SaveIcon(strExt, hOldIcon);
            GetIcon(strExt, hIcon);
        }

clean0:
        if (hOldIcon)
        {
            ::DestroyIcon(hOldIcon);
            hOldIcon = NULL;
        }

        return hIcon;
    }

    void SaveIcon(const CString& strExt, HICON hIcon)
    {
        IconInfo iconInfo(strExt, hIcon);
        m_listIcons.push_front(iconInfo);
        if (++m_nLength > 30)
        {
            m_listIcons.pop_back();
            --m_nLength;
        }
    }

    void GetIcon(const CString& strExt, HICON& hIcon)
    {
        std::deque<IconInfo>::iterator i;

        for (i = m_listIcons.begin(); i != m_listIcons.end(); ++i)
        {
            if (i->m_strExt.CompareNoCase(strExt) == 0)
            {
                hIcon = i->m_hIcon;
                break;
            }
        }
    }

    void Clean()
    {
        m_listIcons.clear();
    }

    HICON GetIconFromFilePath(const CString& strFilePath)
    {
        SHFILEINFO fileInfo = { 0 };

        ::SHGetFileInfo(strFilePath, 0, &fileInfo, sizeof(fileInfo), SHGFI_ICON | SHGFI_SMALLICON);

        return fileInfo.hIcon;
    }

private:
    std::deque<IconInfo> m_listIcons;
    int m_nLength;
};

//////////////////////////////////////////////////////////////////////////

