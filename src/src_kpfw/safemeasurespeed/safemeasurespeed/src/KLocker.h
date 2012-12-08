#pragma once
/********************************************************************
//
//	FileName	:	KLocker.h
//	Version		:	1.0
//	Author		:	liyu
//	Date		:	2009-12-3	09:01	--	Created.
//	Comment		:	锁，可以使用多种锁，具体根据情况定。
//	eg：	
//			class UserLocker
//			{
//			public:
//				UserLocker() { m_pLockerCS = new KLockerCS; }
//				void User(void) 
//				{
//					KLocker(m_pLickerCS);
//					//代码行
//				}
//			private:
//				ILockerMode* m_pLockerCS;
//			}
*********************************************************************/

class ILockerMode
{
public:
	virtual void Lock(void) = 0;
	virtual void Ulock(void) = 0;
};


class KLockerCS : public ILockerMode
{
public:
	KLockerCS() { ::InitializeCriticalSection(&m_CS); }
	~KLockerCS() { ::DeleteCriticalSection(&m_CS); }

	void Lock(void) 
	{ 
		::EnterCriticalSection(&m_CS); 
	}

	void Ulock(void) 
	{ 
		::LeaveCriticalSection(&m_CS); 
	}

private:
	CRITICAL_SECTION m_CS;
};

class KLockerEvent : public ILockerMode
{
public:
	KLockerEvent(LPCWSTR lpName) { m_hEvnet = ::CreateEvent(NULL, FALSE, TRUE, lpName); }
	~KLockerEvent() { ::CloseHandle(m_hEvnet); }

	void Lock(void) { ::WaitForSingleObject(m_hEvnet, INFINITE); }
	void Ulock(void) { ::SetEvent(m_hEvnet); }

private:
	HANDLE m_hEvnet;
};

class KLocker
{
public:
	KLocker(ILockerMode* pLock)
	{
		m_pLock = pLock;
		if (m_pLock != NULL)		m_pLock->Lock(); 
	}

	~KLocker(void)
	{
		if (m_pLock != NULL)		m_pLock->Ulock();	
	}

private:
	ILockerMode* m_pLock;
};
