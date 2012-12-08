////////////////////////////////////////////////////////////////////////////////
//      
//      common include File for kpfw
//      
//      File      : filecomm.h
//      Version   : 1.0
//      Comment   : 定义网镖何种数据文件的数据结构
//      
//      Create at : 2008-4-7
//      Create by : liupeng
//      
////////////////////////////////////////////////////////////////////////////////

#pragma once 

#pragma pack(push, 1)
// 数据结构体头部定义
typedef struct _tgRecordHeader
{
	unsigned int		cbSize;								// 整个结构体大小
	unsigned char		bCompRecord;						// 是否是复合结构体
	unsigned char		nVer;								// 结构体版本
	unsigned short		nType;								// 结构体类型
} RecordHeader;

// 属性结构项
typedef struct _tgAttribEntry
{
	unsigned short		nAttribID;							// 属性类型
	long				nValue;								// 属性值
} AttribEntry;

// 区域描述定义
typedef struct _tgZoonEntry
{
	unsigned char		MacAddr[6];							// mac地址
	unsigned char		bTrusted;							// 是否可信
} ZoonEntry;

#pragma pack(pop)

//////////////////////////////////////////////////////////////////////////
// 结构体类型定义

// 结构体区间类型分类

enum
{
	Rct_Class_Comm			= 0,							// 公共结构区间
	Rct_Class_AppRule		= 0x1000,						// 应用程序规则文件使用
	Rct_Class_IpRule		= 0x2000,						// ip规则文件使用
	Rct_Class_Area			= 0x3000,						// 区域文件
};

// 公共结构体类型
enum
{
	Rct_Comm_AttrList		= Rct_Class_Comm | 1,			// 属性列表
	Rct_Comm_DataBuf		= Rct_Class_Comm | 2,			// 原始数据

	Rct_PortRangeList		= Rct_Class_Comm | 3,			// 端口范围列表
	Rct_PortList			= Rct_Class_Comm | 4,			// 端口列表

	Rct_IPRangeList			= Rct_Class_Comm | 5,			// ip范围列表
	Rct_IPList				= Rct_Class_Comm | 6,			// ip列表
};

// 应用程序规则文件中结构的类型
enum
{
	AppRule_Root			= Rct_Class_AppRule | 0x0101,	// 应用程序规则开始
	AppRule_RuleList		= Rct_Class_AppRule | 0x0102,	// 应用程序规则列表
	AppRule_Rule			= Rct_Class_AppRule | 0x0103,	// 应用程序规则项
	AppRule_RuleID			= Rct_Class_AppRule | 0x0104,	// 规则id
	AppRule_ExePath			= Rct_Class_AppRule | 0x0105,	// 程序路径
	AppRule_UserMode		= Rct_Class_AppRule | 0x0106,	// 用户选择的模式
	AppRule_TrustMode		= Rct_Class_AppRule | 0x0107,	// 自动判断的模式
	AppRule_CheckSum		= Rct_Class_AppRule | 0x0108,	// 校验和
	AppRule_LastFileTime	= Rct_Class_AppRule | 0x010a,	// 上次文件改变的时间

	AppRule_Creator			= Rct_Class_AppRule | 0x0201,	// 创建人
	AppRule_CreateDate		= Rct_Class_AppRule | 0x0202,	// 创建日期
	AppRule_CreateReason	= Rct_Class_AppRule | 0x0203,	// 创建原因
	AppRule_LastModifyDate	= Rct_Class_AppRule | 0x0204,	// 最近修改日期
	AppRule_LastModifyer	= Rct_Class_AppRule | 0x0205,	// 最近修改人

	AppRule_DenyPortList	= Rct_Class_AppRule | 0x0301,	// 拒绝端口列表
	AppRule_TcpRemotePortList	= Rct_Class_AppRule | 0x0302,	// tcp 远程端口列表
	AppRule_TcpLocalPortList	= Rct_Class_AppRule | 0x0303,	// tcp 本地端口列表
	AppRule_UdpRemotePortList	= Rct_Class_AppRule | 0x0304,	// udp 远程端口列表
	AppRule_UdpLocalPortList	= Rct_Class_AppRule | 0x0305,	// udp 本地端口列表
};

// ip规则文件结构类型定义
enum
{
	IpRule_Root				= Rct_Class_IpRule	| 0x0101,	// ip规则数据开始
	IpRule_RuleList			= Rct_Class_IpRule	| 0x0102,	// ip规则列表
	IpRule_Rule				= Rct_Class_IpRule	| 0x0103,	// 单个ip规则

	IpRule_RuleName			= Rct_Class_IpRule	| 0x0105,	// ip规则名字
	IpRule_RuleDesc			= Rct_Class_IpRule	| 0x0106,	// ip规则描述
	IpRule_Valid			= Rct_Class_IpRule	| 0x0107,	// ip规则是否起作用

	IpRule_Creator			= Rct_Class_IpRule	| 0x0201,	// 创建人
	IpRule_CreateDate		= Rct_Class_IpRule	| 0x0202,	// 创建日期
	IpRule_CreateReason		= Rct_Class_IpRule	| 0x0203,	// 创建原因
	IpRule_LastModifyDate	= Rct_Class_IpRule	| 0x0204,	// 最近修改日期
	IpRule_LastModifyer		= Rct_Class_IpRule	| 0x0205,	// 最近修改人

	IpRule_FilterInfo		= Rct_Class_IpRule	| 0x0301,	// 规则信息
};

// 区域文件信息
enum
{
	AreaInfo_Root			= Rct_Class_Area	| 0x0101,	// 区域数据开始
	AreaInfo_AreaList		= Rct_Class_Area	| 0x0102,	// 已知区域表
	AreaInfo_Area			= Rct_Class_Area	| 0x0103,	// 一个区域信息
	
	AreaInfo_Name			= Rct_Class_Area	| 0x0201,	// 区域名
	AreaInfo_CreateDate		= Rct_Class_Area	| 0x0202,	// 区域创建时间
	AreaInfo_Type			= Rct_Class_Area	| 0x0203,	// 区域类型
	AreaInfo_Gate			= Rct_Class_Area	| 0x0204,	// 区域网关mac地址
	AreaInfo_GateIp			= Rct_Class_Area	| 0x0205,	// 区域网关IP地址
};

//////////////////////////////////////////////////////////////////////////
// 结构体读取辅助类
class KRecord
{
private:
	BYTE*				m_pBuf;								// 结构开始

public:
	KRecord(BYTE* pBuf): m_pBuf(pBuf)
	{
	}

	BOOL	Ok()
	{
		return GetSize() >= sizeof(RecordHeader);
	}

	INT		GetSize()
	{
		return ((RecordHeader*)m_pBuf)->cbSize;
	}

	BOOL	IsComposed()
	{
		return ((RecordHeader*)m_pBuf)->bCompRecord;
	}

	INT		GetVersion()
	{
		return ((RecordHeader*)m_pBuf)->nVer;
	}

	INT		GetType()
	{
		return ((RecordHeader*)m_pBuf)->nType;
	}

	BYTE*	GetData()
	{
		return m_pBuf + sizeof(RecordHeader);
	}

	INT		GetDataSize()
	{
		return GetSize() - sizeof(RecordHeader);
	}

	KRecord	NextRecord()
	{
		return KRecord( m_pBuf + GetSize() );
	}

	KRecord	FirstSubRecord()
	{
		ASSERT(IsComposed());
		return KRecord( m_pBuf + sizeof(RecordHeader) );
	}

	BOOL	IsSubRecord(KRecord& rcd)
	{
		if (rcd.m_pBuf < GetData())
			return FALSE;
		if (rcd.m_pBuf > ( GetData() + GetDataSize() - sizeof(RecordHeader)))
			return FALSE;
		return TRUE;
	}
};

//////////////////////////////////////////////////////////////////////////
// 结构体写出辅助接口
interface IWriteProxy
{
	virtual BYTE*	GetBuf(INT nPos)		= 0;
	virtual INT		GetCurPos()				= 0;
	virtual BYTE*	Next(INT nBytes)		= 0;
};

#define	INIT_MEM_BUF_SIZE		1024*256

class KMemWriteProxy: public IWriteProxy
{
private:
	BYTE*			m_pBuf;
	BYTE*			m_pCur;
	INT				m_nBufSize;
public:
	KMemWriteProxy()			
	{
		m_nBufSize = INIT_MEM_BUF_SIZE;
		m_pBuf = new BYTE[m_nBufSize];
		m_pCur = m_pBuf;
	}

	~KMemWriteProxy()
	{
		delete [] m_pBuf;
		m_nBufSize = 0;
		m_pCur = 0;
	}

	virtual BYTE*	GetBuf(INT nPos)
	{
		return m_pBuf + nPos;
	}

	virtual INT		GetCurPos()
	{
		return (INT)(m_pCur - m_pBuf);
	}

	virtual BYTE*	Next(INT nBytes)
	{
		if (GetCurPos() + nBytes > m_nBufSize)
			Resize( GetCurPos() + nBytes );

		BYTE* pCur = m_pCur;
		m_pCur += nBytes;
		return pCur;
	}

private:
	INT		Resize(INT nSize)
	{
		INT nNewSize = m_nBufSize;
		while (nNewSize < nSize)
			nNewSize *= 2;

		if (m_pBuf)
		{
			INT nOldPos = (INT)(m_pCur - m_pBuf);

			BYTE*	pBuf = new BYTE[nNewSize];
			memcpy(pBuf, m_pBuf, m_nBufSize);
			delete [] m_pBuf;

			m_nBufSize = nNewSize;
			m_pBuf = pBuf;
			m_pCur = m_pBuf + nOldPos;
		}
		return nNewSize;
	}
};

//////////////////////////////////////////////////////////////////////////
// 结构体写出辅助类
class KRecordWriter: public IWriteProxy
{
private:
	IWriteProxy*		m_pProxy;							// 写出代理
	INT					m_pRecordPos;						// 当前Record的指针
	INT					m_nSize;							// 当前record的大小
	BOOL				m_bEnd;								// 是否已经结束
public:
	KRecordWriter(IWriteProxy* pProxy, unsigned char bComp, unsigned char nVersion, unsigned short nType)
	{
		m_pProxy = pProxy;
		m_pRecordPos = m_pProxy->GetCurPos();
		m_nSize = 0;
		m_bEnd = FALSE;

		RecordHeader* pRecord = (RecordHeader*) AddSize( sizeof(RecordHeader) );

		pRecord->bCompRecord = bComp;
		pRecord->nType = nType;
		pRecord->nVer = nVersion;
	}

	virtual BYTE*	GetBuf(INT nPos)
	{
		return m_pProxy->GetBuf(nPos);
	}

	virtual INT		GetCurPos()
	{
		return m_pProxy->GetCurPos();
	}

	virtual BYTE*	Next(INT nBytes)
	{
		return AddSize(nBytes);
	}

	~KRecordWriter()
	{
		ASSERT(m_bEnd);
	}

	RecordHeader* GetRecord()
	{
		return (RecordHeader*) m_pProxy->GetBuf(m_pRecordPos);
	}

	KRecordWriter*	StartRecord(unsigned short nType, int nVer = 0)
	{
		return new KRecordWriter(this, 0, nVer, nType);
	}

	KRecordWriter*	StartCompRecord(unsigned short nType, int nVer = 0)
	{
		return new KRecordWriter(this, 1, nVer, nType);
	}

	BOOL	EndRecord()
	{
		GetRecord()->cbSize = m_nSize;
		m_bEnd = TRUE;
		return TRUE;
	}

	template< typename T>
	BOOL	Write(T& Value)
	{
		ASSERT( !GetRecord()->bCompRecord && "复合记录不能用这个函数" );
		T* pBuf = (T*) AddSize( sizeof(T) );
		*pBuf = Value;
		return TRUE;
	}

	template< typename T>
	BOOL	Write(T* pValue, int nSize = 1)
	{
		ASSERT( !GetRecord()->bCompRecord && "复合记录不能用这个函数" );
		BYTE* pBuf = AddSize( sizeof(T) * nSize );
		memcpy(pBuf, pValue, sizeof(T) * nSize);
		return TRUE;
	}

private:
	BYTE*	AddSize(INT nSize)
	{
		m_nSize += nSize;
		return m_pProxy->Next(nSize);
	}
};

//////////////////////////////////////////////////////////////////////////
// 常用方法
inline HRESULT SaveString(LPCWSTR str, KRecordWriter* pWriter, unsigned short nType, int nVer = 0)
{
	KRecordWriter* pStr = pWriter->StartRecord(nType, nVer);
	pStr->Write(str, (int)(wcslen(str) + 1));

	pStr->EndRecord();
	delete pStr;
	return S_OK;
}

template< typename T>
inline HRESULT SaveStruct(T* pValue, KRecordWriter* pWriter, unsigned short nType, int nVer = 0)
{
	KRecordWriter* pStructWriter = pWriter->StartRecord(nType);
	pStructWriter->Write(pValue);

	pStructWriter->EndRecord();
	delete pStructWriter;
	return S_OK;
}

template< typename T>
inline HRESULT SaveArray(T* pValue, INT nSize, KRecordWriter* pWriter, unsigned short nType, int nVer = 0)
{
	KRecordWriter* pStructWriter = pWriter->StartRecord(nType);
	pStructWriter->Write(pValue, sizeof(T) * nSize);

	pStructWriter->EndRecord();
	delete pStructWriter;
	return S_OK;
}