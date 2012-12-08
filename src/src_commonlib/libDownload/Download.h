#pragma once
#include "Defines.h"
#include "pubconfig/pubconfig.h"

struct TDownStat : public IDownStat
{
public:
	TDownStat() 
	{
		Reset();
	}
	virtual int64 Downloaded() 
	{
		return m_iDownloaded;
	}

	virtual int64 Speed()
	{
		return m_nSpeed;
	}

	virtual int64 AverageSpeed()
	{
		if(!m_bBegined)
			return 0;
		if(m_bEnded)
			return (m_iDownloaded)/(m_dwTmEnd-m_dwTmBegin+1);
		DWORD dwTmNow = GetTickCount();
		ATLASSERT(dwTmNow>=m_dwTmBegin);
		return (m_iDownloaded*1000)/(dwTmNow-m_dwTmBegin+1);
	}

	virtual DWORD TimeUsed()
	{
		return m_dwTmEnd - m_dwTmBegin;
	}

	void Reset()
	{
		m_nSpeed = 0;
		m_iDownloaded = 0;
		m_bBegined = m_bEnded = FALSE;
		m_dwTmBegin = m_dwTmEnd = m_dwTmLastSlot = m_dwTmLastDownloadProgess = 0;
	}

	void OnDownBegin()
	{
		m_bBegined = TRUE;
		m_iDownloaded = 0;
		m_dwTmBegin = m_dwTmEnd = m_dwTmLastSlot = m_dwTmLastDownloadProgess = GetTickCount();
		m_nSpeed = m_nSpeedSlot = 0;
	}

	void OnDownEnd()
	{
		ATLASSERT(m_bBegined);
		m_bEnded = TRUE;
		m_dwTmEnd = GetTickCount();
	}
		
	void OnDownData(DWORD dwTimeNow, int64 iDownloaded)
	{
		if((dwTimeNow-m_dwTmLastSlot)>DOWNLOAD_SPEED_TEST_INTERVAL)
		{
			m_nSpeed = m_nSpeedSlot*(dwTimeNow-m_dwTmLastSlot)/(DOWNLOAD_SPEED_TEST_INTERVAL);
			m_nSpeedSlot = 0;
			m_dwTmLastSlot = dwTimeNow;
		}
		m_nSpeedSlot += iDownloaded;
	}

	void OnDownData(int64 iDownloaded)
	{
		//OnDownData(GetTickCount(), iDownloaded);
		m_iDownloaded += iDownloaded;
		m_nSpeedSlot += iDownloaded;
	}
	
public:
	int64 m_iDownloaded;
	BOOL m_bBegined, m_bEnded;
	
	// speed 
	int64 m_nSpeed, m_nSpeedSlot;
	DWORD m_dwTmBegin, m_dwTmEnd, m_dwTmLastSlot;

	// 
	DWORD m_dwTmLastDownloadProgess;
};
