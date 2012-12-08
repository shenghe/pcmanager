//////////////////////////////////////////////////////////////////////////
// 
// Class Name   : CKBitmapMapMFC
// Author       : Conan Zhang
// Create Date  : 2006.12.30
// Version      : 1.0
// Description  : 用于自绘控件的Bitmap资源收集器(MFC版本)
// Tags         : 
// 
//////////////////////////////////////////////////////////////////////////

#pragma once

class CKBitmapMapMFC
{
public:
    CKBitmapMapMFC(void)
    {
    }
    virtual ~CKBitmapMapMFC(void)
    {
        if (this == GetInstance())
            _ReleaseMap();
    }

    HBITMAP operator[](const UINT uResID)
    {
        HBITMAP hRetVal = NULL;
        BOOL bRet = GetInstance()->m_ResourceMap.Lookup(uResID, (void *&)hRetVal);

        if (!bRet)
        {
            hRetVal = ::LoadBitmap(::AfxGetResourceHandle(), MAKEINTRESOURCE(uResID));

            if (hRetVal)
                GetInstance()->m_ResourceMap.SetAt(uResID, hRetVal);
        }

        return hRetVal;
    }

protected:

    static CKBitmapMapMFC* GetInstance()
    {
        static CKBitmapMapMFC s_Instance;
        return &s_Instance;
    }

    //CMapStringToPtr m_FileMap;
    CMapWordToPtr m_ResourceMap;

    void _ReleaseMap(void)
    {
        POSITION pos = NULL;
        HBITMAP hBitmap = NULL;
        WORD wKey = 0;

        for( pos = m_ResourceMap.GetStartPosition(); pos != NULL;)
        {
            m_ResourceMap.GetNextAssoc( pos, wKey, (void *&)hBitmap);
            ::DeleteObject(hBitmap);
        }

        m_ResourceMap.RemoveAll();
    }
};
