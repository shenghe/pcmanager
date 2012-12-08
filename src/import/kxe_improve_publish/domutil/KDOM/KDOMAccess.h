/********************************************************************
* CreatedOn: 2006-12-11   14:50
* FileName:  KDOMAccess.h
* CreatedBy: zhangrui <zhangrui@kingsoft.net>
* $LastChangedDate$
* $LastChangedRevision$
* $LastChangedBy$
* $HeadURL$
* Purpose:
*********************************************************************/

#ifndef __KDOMAccess_H_
#define __KDOMAccess_H_

#include "../KMPCommon/KConv.h"

#include "KDOMBasic.h"
#include "KDOMAutoPath.h"

_KAN_DOM_BEGIN

template<class _KDOMCursor>
class KDOMAccess
{
public:
    KDOMAccess() : m_bInitFlag(false)
	{
	}

    ~KDOMAccess()
	{

	}

    // 载入一个DOM文件
    long LoadFile(const TKChar* pcszFileName)
	{
		assert(pcszFileName);
		int nRetCode = m_domCursor.LoadFile(pcszFileName);
		
		m_bInitFlag = SUCCEEDED(nRetCode);
		
		return nRetCode;
	}

    // 载入一个DOM字符串
    long LoadString(const char* pcszInput)
	{
		assert(pcszInput);
		int nRetCode = m_domCursor.LoadString(pcszInput);
		
		m_bInitFlag = SUCCEEDED(nRetCode);
		
		return nRetCode;
	}

    // 载入一个字符串作为DOM对象的根结点名字
    long LoadDOMName(const char* pcszName)
	{
		assert(pcszName);
		int nRetCode = m_domCursor.LoadDOMName(pcszName);
		
		m_bInitFlag = SUCCEEDED(nRetCode);
		
		return nRetCode;
	}

    // 载入另一个DOM对象
    template<class _KOtherDOMCursor>
    long LoadDOM(KDOMAccess<_KOtherDOMCursor>& otherDomAccess)
	{
		// 避免自复制
		if ((void*)this == (void*)&otherDomAccess)
			return S_OK;
		
		int nRetCode = otherDomAccess.m_domCursor.ToOtherDOM(m_domCursor);
		
		m_bInitFlag = SUCCEEDED(nRetCode);
		
		return nRetCode;
	}

    // 将DOM对象保存到文件
    long SaveFile(const TKChar* pcszFileName)
	{
		assert(pcszFileName);
		return m_domCursor.SaveFile(pcszFileName);
	}

    // 输出DOM字符串(不保证是否带格式)
    long ToString(std::string* pstrOutput)
	{
		assert(m_bInitFlag);
		assert(pstrOutput);
		return m_domCursor.ToString(pstrOutput);
	}

    // 输出不带格式的DOM字符串()
    long ToCompactString(std::string* pstrRet)
	{
		assert(m_bInitFlag);
		assert(pstrRet);
		return m_domCursor.ToCompactString(pstrRet);
	}

    // 获取数据的名字,通常为数据的根结点名
    long GetDataName(std::string* pstrDataName)
	{
		assert(m_bInitFlag);
		assert(pstrDataName);
		
		return m_domCursor.GetObjName(*pstrDataName);
	}

    long PopPathDump(std::vector<std::string>& dumpPath)
	{
		assert(m_bInitFlag);
		
		m_domCursor.PopPathDump(dumpPath);
		
		return 0;
	}

    _KDOMCursor& Cursor()
	{
		return m_domCursor;
	}


    //////////////////////////////////////////////////////////////////////////
    // 数据存取接口,在以下接口调用前,必须保证至少调用过一次某个Load方法
    // pcszPath是以'\'为分隔符号的路径表示
    // 格式为"child\grandchild\...\objectname"
    // 注意,路径中不包含根结点的名字,也就是通过LoadDOMName设置的名字
    template<class T>
    long WriteData(const char* pcszPath, T& value)
	{  
		assert(m_bInitFlag);
		assert(pcszPath);

		m_domCursor.PrepareForWrite();
		{
			KDOMAutoPath autoPath(pcszPath, &m_domCursor, true);

			return m_domCursor.ObjDataExchange(true, value);
		}
	}

    template<class T>
    long ReadData(const char* pcszPath, T& value)
	{
		assert(m_bInitFlag);
		assert(pcszPath);
		
		m_domCursor.PrepareForRead();
		
		{
			KDOMAutoPath autoPath(pcszPath, &m_domCursor, false);
			
			return m_domCursor.ObjDataExchange(false, value);
		}
	}

    template<class T>
    long ReadData(const char* pcszPath, T& value, const T& defValue)
	{
		assert(m_bInitFlag);
		assert(pcszPath);
		
		int nRetCode = ReadData(pcszPath, value);
		if (FAILED(nRetCode))
		{
			value = defValue;
		}
		
		return S_OK;
	}


private:
    KDOMAccess(const KDOMAccess&);
    KDOMAccess& operator=(const KDOMAccess&);

    typedef KDOMAutoPathEx<_KDOMCursor>  KDOMAutoPath;

    _KDOMCursor m_domCursor;

    bool m_bInitFlag;

#if _MSC_VER >= 1400
	template<class _KDOMCursor>
		friend class KDOMAccess;
#else
	friend class KDOMAccess<_KDOMCursor>;
#endif

};

_KAN_DOM_END

//#include "KDOMAccessImp.h"

#endif//__KDOMAccess_H_
