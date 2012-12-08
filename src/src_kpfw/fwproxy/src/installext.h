/********************************************************************
* CreatedOn: 2007-10-22   15:25
* FileName: InstallExt.h
* CreatedBy: lidengwang <lidengwang@kingsoft.net>
* $LastChangedDate$
* $LastChangedRevision$
* $LastChangedBy$
* $HeadURL:  $
* Purpose:
*********************************************************************/

#pragma once


enum{
    em_none,
    em_install,
    em_uninstall
};

class InstallExt
{
public:
    
    InstallExt();

    HRESULT RunInsatll(LPTSTR pszCmdLine, LPTSTR pszCurPath);
    HRESULT DoInstall();
    HRESULT DoUninstall();
    void WaitEXEExit();

private:
    
    //BOOL  m_bExist;
    //DWORD m_dwCurUserPolicy;
    //BYTE  m_bufferLocMPolicy[1];
    //int   m_nWaitDo;
    //BOOL  m_bShowWarning;

};