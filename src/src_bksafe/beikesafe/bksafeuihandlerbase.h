
#pragma once

// enum STATE_CLOSE_CHECK
// {
// 	CLOSE_CHECK_FORCE_CLOSE = 0,		// 直接强制退出
// 	CLOSE_CHECK_CANCEL_CLOSE,			// 取消退出动作
// 	CLOSE_CHECK_CONTINUE,
// };//hub

class CBkSafeUIHandlerBase
{
public:
	virtual STATE_CLOSE_CHECK	CloseCheck() = 0;						// 关闭之前的询问
	virtual VOID				CloseSuccess(BOOL bSucClose){return;}	// 询问之后的可以进行动作的机会
};

class CBkSafeUIHandlerMgr
{
public:
	CBkSafeUIHandlerMgr()
	{

	}
	virtual ~CBkSafeUIHandlerMgr()
	{

	}

	VOID AddUIHandler(CBkSafeUIHandlerBase* ptr)
	{
		m_ptrArrayCheck.Add(ptr);
	}

	VOID CloseSuccess()
	{
		for ( int i=0; i < m_ptrArrayCheck.GetSize(); i++)
		{
			m_ptrArrayCheck[i]->CloseSuccess(TRUE);
		}
	}

	BOOL GetCloseCheckRet()
	{
		for ( int i=0; i < m_ptrArrayCheck.GetSize(); i++)
		{
			CBkSafeUIHandlerBase*	ptr = m_ptrArrayCheck[i];
			STATE_CLOSE_CHECK		st	= ptr->CloseCheck();

			if (st==CLOSE_CHECK_FORCE_CLOSE)
			{
				return TRUE;
			}
			else if (st==CLOSE_CHECK_CANCEL_CLOSE)
			{
				ptr->CloseSuccess(FALSE);
				return FALSE;
			}
		}
		return TRUE;
	}

protected:
	CSimpleArray<CBkSafeUIHandlerBase*>	m_ptrArrayCheck;
};