#pragma once
#include <vector>
#include "scan/quran/ikavrecycle.h"

typedef HRESULT (__stdcall *CREATE_RECYCLE)(REFIID riid, void** ppv);

struct  QUAR_FILE_INFO
{
	DWORD dwCheck;
	SYSTEMTIME stSTime;
	WCHAR szQuarFileName[MAX_PATH * 2];
	WCHAR szSurFileName[MAX_PATH * 2];
	WCHAR szVirusName[MAX_PATH * 2];
	ULONG uIndex;

	QUAR_FILE_INFO(const QUAR_FILE_INFO& st)
	{
		dwCheck = st.dwCheck;
		stSTime = st.stSTime;
		uIndex	= st.uIndex;
		::wcsncpy(szQuarFileName, st.szQuarFileName, MAX_PATH * 2);
		::wcsncpy(szSurFileName, st.szSurFileName, MAX_PATH * 2);
		::wcsncpy(szVirusName, st.szVirusName, MAX_PATH * 2);
	}

	QUAR_FILE_INFO& operator =(const QUAR_FILE_INFO &st)
	{
		dwCheck = st.dwCheck;
		stSTime = st.stSTime;
		uIndex	= st.uIndex;
		::wcsncpy(szQuarFileName, st.szQuarFileName, MAX_PATH * 2);
		::wcsncpy(szSurFileName, st.szSurFileName, MAX_PATH * 2);
		::wcsncpy(szVirusName, st.szVirusName, MAX_PATH * 2);

		return *this;
	}

	QUAR_FILE_INFO()
	{
		dwCheck = 0;
		uIndex	= 0;
		ZeroMemory(&stSTime, sizeof(stSTime));
		ZeroMemory(szQuarFileName, sizeof(szQuarFileName));
		ZeroMemory(szSurFileName, sizeof(szSurFileName));
		ZeroMemory(szVirusName, sizeof(szVirusName));
	}
};

typedef std::vector<QUAR_FILE_INFO> VEC_QUAR;

class KaquaraLoader
{
public:
	KaquaraLoader()
		:m_hQuara(NULL)
		, m_pKavRecycle(NULL)
	{

	}

	~KaquaraLoader()
	{
		UnInit();
	}

	HRESULT Init()
	{
		BOOL bRet = _DoLoadQuaran();
		return bRet == TRUE ? S_OK: E_FAIL;
	}

	HRESULT UnInit()
	{
		if (m_pKavRecycle)
		{
			m_pKavRecycle->Release();
			m_pKavRecycle = NULL;
		}
		if (m_pKaFile)
		{
			m_pKaFile->Release();
			m_pKaFile = NULL;
		}

		if (m_hQuara)
		{
			::FreeLibrary(m_hQuara);
			m_hQuara = NULL;
		}

		return S_OK;
	}

	void DeleteQuarantineFile( std::vector<std::wstring>& vecFile )
	{
		if ( m_pKavRecycle )
		{
			for( int i = 0; i < vecFile.size(); i++ )
			{
				m_pKavRecycle->DeleteQuarantineFile( vecFile[i].c_str() );
			}
		}
	}

	HRESULT RestoreQuarantineFile( std::wstring& strSurName, std::wstring& strQuaraName )
	{
		HRESULT hr = E_FAIL;
		if( m_pKavRecycle )
		{
			hr = m_pKavRecycle->RestoreQuarantineFile( strQuaraName.c_str(), strSurName.c_str() );
			if( SUCCEEDED(hr) )
			{
				m_pKavRecycle->DeleteQuarantineFile( strQuaraName.c_str() );
			}
		}

		return hr;
	}

	void LoadQuara( VEC_QUAR& vecFile )
	{
		if (!m_pKaFile)
		{
			return;
		}

		ULONG ulIndex = 0;
		m_pKaFile->getQurantFileCount(&ulIndex);
		for (int i = 0; i< ulIndex; i++)
		{
			QUAR_FILE_INFO stQuarInfo;
			DWORD dwLen = MAX_PATH * 2 - 1;
			m_pKaFile->getCreateTime(i, &stQuarInfo.stSTime);
			m_pKaFile->getQurantFileName(i, stQuarInfo.szQuarFileName, dwLen);
			m_pKaFile->getVirusName(i, stQuarInfo.szVirusName, dwLen);
			m_pKaFile->getOrignFileName(i, stQuarInfo.szSurFileName, dwLen);
			stQuarInfo.uIndex = i;

			vecFile.push_back( stQuarInfo );
		}

		return;
	}

	int GetQurantFileCount()
	{
		ULONG ulIndex = 0;
		if (m_pKaFile)
		{
			m_pKaFile->getQurantFileCount(&ulIndex);
		}

		return ulIndex;
	}

protected:

	BOOL _DoLoadQuaran()
	{
		if (!m_hQuara)
		{
			WCHAR szQuarPath[MAX_PATH * 2] = {0};
			::GetModuleFileName(NULL, szQuarPath, MAX_PATH*2 - 1);
			::PathRemoveFileSpecW(szQuarPath);
			::PathAppend(szQuarPath, L"kse\\kavquara.dll");

			m_hQuara = ::LoadLibrary(szQuarPath);

			HRESULT hr = E_FAIL;
			if ( m_hQuara )
			{
				CREATE_RECYCLE pCreateRecycle = NULL;
				pCreateRecycle = (CREATE_RECYCLE)GetProcAddress( m_hQuara, "CreateRecycleObject");
				if (pCreateRecycle != NULL)
				{
					hr = pCreateRecycle(IID_KAVRECYCLE, (LPVOID*)&m_pKavRecycle);
				}
				if (SUCCEEDED(hr) && m_pKavRecycle )
				{
					hr = m_pKavRecycle->GetQuarantineInfo(&m_pKaFile);
					return m_pKaFile != NULL;
				}
			}
		}

		return FALSE;
	}

protected:

	HMODULE			m_hQuara;
	IKAVRecycle*	m_pKavRecycle;
	IKAQFileInfo*	m_pKaFile;
};