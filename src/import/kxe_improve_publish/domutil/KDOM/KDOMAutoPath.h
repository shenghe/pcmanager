/********************************************************************
* CreatedOn: 2006-12-11   14:53
* FileName:  KDOMAutoPath.h
* CreatedBy: zhangrui <zhangrui@kingsoft.net>
* $LastChangedDate$
* $LastChangedRevision$
* $LastChangedBy$
* $HeadURL$
* Purpose:
*********************************************************************/

#ifndef __KDOMAutoPath_H_
#define __KDOMAutoPath_H_

#include "KDOMBasic.h"

_KAN_DOM_BEGIN

template<class _KDOMCursor>
class KDOMAutoPathEx
{
public:
    KDOMAutoPathEx(const char* pcszPath, _KDOMCursor* pDOMCursor, bool bWrite):
        m_pDOMCursor(pDOMCursor),
        m_bWrite(bWrite)
    {
        assert(pDOMCursor);
        if (m_pDOMCursor)
        {
            long lRet = m_pDOMCursor->PhrasePath(pcszPath, m_pathItemList);
            if (SUCCEEDED(lRet))
            {
                for (size_t i = 0; i < m_pathItemList.size(); ++i)
                {
                    m_pDOMCursor->BeginChild(m_pathItemList[i].c_str(), m_bWrite);
                }
            }
        }
    }

    ~KDOMAutoPathEx()
    {
        if (m_pDOMCursor)
        {
            while (!m_pathItemList.empty())
            {
                m_pDOMCursor->EndChild();
                m_pathItemList.pop_back();
            }
        }
    }


private:
    KDOMAutoPathEx(const KDOMAutoPathEx&);
    KDOMAutoPathEx& operator=(const KDOMAutoPathEx&);



    bool                        m_bWrite;
    _KDOMCursor*                m_pDOMCursor;
    std::vector<std::string>    m_pathItemList;
};

_KAN_DOM_END


#endif//__KDOMAutoPath_H_
