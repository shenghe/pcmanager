///////////////////////////////////////////////////////////////	
//	
// FileName  :  templatebuilder_public.h
// Creator   :  yangzhenhui@kingsoft.com
// Date    	 :  2009-08-04  11:13
// Comment   :  
//
///////////////////////////////////////////////////////////////	
#pragma once


extern const __declspec(selectany) GUID CLSID_IAdvFltXmlEditorImpl = 
{ 0x8620b4f2, 0xc2b6, 0x437f, { 0xac, 0xad, 0x22, 0x7d, 0x24, 0x20, 0x92, 0x96 } };

[
	uuid({8620b4f2-c2b6-437f-acad-227d24209296})
]
interface IAdvFltXmlEditor : public IUnknown
{
	virtual bool AddNode( ULONG uOffset, const char* pHexData, ULONG uDataLen, char* pStrNodeDesc ) = 0;
	virtual bool SetProtoType( ULONG pt ) = 0;		//PROTO_TYPE
	virtual bool SetPacketSize( ULONG uSize ) = 0;
	virtual bool SetDesc( const char* szDesc ) = 0;
	virtual bool SetDirection( ULONG pd ) = 0;		//PACKET_DIR
	virtual bool SetOperation( int nOp ) = 0;
	virtual bool SetSrcPort( short sPort ) = 0;
	virtual bool SetDesPort( short sPort ) = 0;
	virtual bool SetGuid( GUID guid ) = 0;
	virtual int GetXml( char* szXml, int nMaxSize ) = 0;
	virtual bool SetXml( const char* szXml ) = 0;
	virtual PBYTE GetTemplateBuffer() = 0;
	virtual ULONG GetTemplateSize() = 0;
	virtual void GetGuid( GUID *guid ) = 0;
	virtual int GetDesc( char *szDesc, ULONG uSize ) = 0;
	virtual bool MatchPackage( const char* pRawPackage, DWORD dwRawPackLength ) = 0;
	virtual void Clear() = 0;
	virtual IAdvFltXmlEditor* Next() = 0;
};


//////////////////////////////////////////////////////////////////////////


extern const __declspec(selectany) GUID CLSID_ITemplateBuilderImpl = 
{ 0x97435990, 0x9f89, 0x4f1b, { 0x9b, 0x38, 0x53, 0xe2, 0x97, 0x4d, 0xb1, 0x22 } };

[
	uuid({97435990-9f89-4f1b-9b38-53e2974db122})
]
interface ITemplateBuilder : public IUnknown
{
	/**
	* @brief	xml进, IAdvFltXmlEditor 出
	* @remark	使用IAdvFltXmlEditor::Next遍历其它项, 在调用完之后使用 ITemplateBuilder::release 释放
	* @param 	
	* @return	S_OK 成功
	**/
	virtual STDMETHODIMP BuildTemplate( __in const char* szXml, __out IAdvFltXmlEditor **Iterator ) = 0;

};


//////////////////////////////////////////////////////////////////////////
