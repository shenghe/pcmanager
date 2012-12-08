/********************************************************************
	created:	2010/06/25 
	filename: 	automemmanage.h
	author:		Xinxing Zhao
	
	purpose:	ÄÚ´æ
*********************************************************************/
#pragma once
class CMem
{
public:
    CMem(DWORD size = 0 )  
    { 
        m_pBuffer = NULL; 

        if (size > 0)
        {
            m_pBuffer = (LPBYTE)malloc(size) ; 
        }
    }

    ~CMem()  
    {  
        if (m_pBuffer)
        {
            free(m_pBuffer);  
        }
    }

    LPBYTE allocBuf(DWORD size) 
    { 
        if (size > 0) 
        {
            m_pBuffer = (LPBYTE)malloc(size) ;
        }

        return m_pBuffer; 
    }

    LPBYTE getData() 
    { 
        return m_pBuffer; 
    }

private:

    LPBYTE m_pBuffer;
};