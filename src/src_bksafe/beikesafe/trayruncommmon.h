#pragma once

class CTrayRunCommon
{
public:
	CTrayRunCommon(){
		m_bChanged = FALSE;
	}
	virtual ~CTrayRunCommon(){}

	static CTrayRunCommon* GetPtr()
	{
		static CTrayRunCommon	sx;
		return &sx;
	}

public:
	void SetChange()
	{
		m_bChanged = TRUE;
	}

	BOOL GetChange()
	{
		if ( m_bChanged )
		{
			m_bChanged = FALSE;
			return TRUE;
		}
		return FALSE;
	}

protected:
	BOOL	m_bChanged;
private:
};