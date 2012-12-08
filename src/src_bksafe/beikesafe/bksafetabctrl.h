
#pragma once

#define TAB_MAX_VALUE_NUM					2

#define TAB_SWITCH_NEW_IEFIX				0
		#define TAB_SWITCH_NEW_IEFIX_FROM_EXAM		1
		#define TAB_SWITCH_NEW_IEFIX_FROM_CLICK		2

#define TAB_SWITCH_SOFTMGR					1
		#define TAB_SWITCH_SOFTMGR_FROM_DESKTOP		1
		#define TAB_SWITCH_SOFTMGR_FROM_CLICK		2


class CBkSafeTabChangeCtrl
{
public:
	CBkSafeTabChangeCtrl()
	{
		CSimpleArray<DWORD>	nXX;

		for ( int i=0; i < TAB_MAX_VALUE_NUM; i++)
		{
			m_valueArray.Add(nXX);
		}
		
	}
	virtual ~CBkSafeTabChangeCtrl(){}

public:
	static CBkSafeTabChangeCtrl& Instance()
	{
		static CBkSafeTabChangeCtrl x;
		return x;
	}

	VOID Push( DWORD nType, DWORD nValue)
	{
		if ( nType >= TAB_MAX_VALUE_NUM )
			return;

		m_valueArray[nType].Add(nValue);
	}

	DWORD Pop( DWORD nType, DWORD nDefvalue)
	{
		if ( nType >= TAB_MAX_VALUE_NUM )
			return nDefvalue;

		DWORD	nSize = m_valueArray[nType].GetSize();
		if ( nSize == 0 )
			return nDefvalue;

		nDefvalue = m_valueArray[nType][nSize-1];
		m_valueArray[nType].RemoveAt(nSize-1);

		return nDefvalue;
	}


protected:
	CSimpleArray<CSimpleArray<DWORD>>	m_valueArray;
};