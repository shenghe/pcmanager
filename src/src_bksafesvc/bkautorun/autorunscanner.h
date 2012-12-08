/**
* @file    autorunscanner.h
* @brief   ...
* @author  zhangrui
* @date    2009-04-24  15:44
*/

#ifndef AUTORUNSCANNER_H
#define AUTORUNSCANNER_H

#include "skylark2\skylarkbase.h"
#include "autoruncontext.h"



class CAutorunScanner//: public CAutorunContext
{
public:

    CAutorunScanner();
    ~CAutorunScanner();


    HRESULT Initialize();
    HRESULT Uninitialize();
    void    Reset();

    void    EnableFeedDebug(BOOL bEnable);

    BOOL    FindFirstEntryItem(DWORD dwEntryID);
    BOOL    FindNextEntryItem();
    BOOL    IsEndOfEntryItemFind() const;

    HRESULT ExportTo(CAutorunScanner& target);



    DWORD   GetCurrentEntryID() const;
    DWORD   GetCurrentEntryType() const;

	HRESULT CleanCurrentEntry() const;


    LPCWSTR GetInformation() const;
    DWORD   GetFileCount() const;
    LPCWSTR GetFilePathAt(DWORD dwIndex) const;



protected:


    DWORD                   m_dwFindEntryID;    ///< 要查找的启动项id
    CAutorunContext         m_atrContext;       ///< 启动项目枚举的句柄


    const SLATR_ENTRY_DATA* m_pCurrentEntry;    ///< 当前扫描到的启动项
};



#endif//AUTORUNSCANNER_H