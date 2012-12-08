#include "StdAfx.h"
#include "dnsrecorder.h"


KDnsRecorder::KDnsRecorder(void)
{
}

//////////////////////////////////////////////////////////////////////////

KDnsRecorder::~KDnsRecorder(void)
{
}

//////////////////////////////////////////////////////////////////////////
BOOL KDnsRecorder::Record( 
	PROTO_TYPE pt,
	PACKET_DIR dir,
	ULONG uLocalIp,
	USHORT uLocalPort,
	ULONG uRemoteIP,
	USHORT uRemotePort,
	PVOID pData, 
	ULONG uDataLen )
{
	if( dir == enumPD_Recieve )
	{
		if( RecordRecv( (PROTO_TYPE)pt,
			uLocalIp, uLocalPort,
			uRemoteIP, uRemotePort,
			pData,
			uDataLen ) )
		{
			Output();			
		}
		return TRUE;
	}
	
	return RecordSend( (PROTO_TYPE)pt,
		uLocalIp, uLocalPort,
		uRemoteIP, uRemotePort,
		pData,
		uDataLen );
}

//////////////////////////////////////////////////////////////////////////

BOOL KDnsRecorder::RecordSend( 
	PROTO_TYPE pt,
	DWORD uLocalIp,
	USHORT uLocalPort,
	DWORD uRemoteIP,
	USHORT uRemotePort,
	PVOID pData, 
	DWORD uDataLen )
{
	DNS_HEADER* pDns = NULL;

	do 
	{
		if( uDataLen <= sizeof(DNS_HEADER) )
			break;
		
		if( uRemotePort != 53 )
			break;

		pDns = (DNS_HEADER*)pData;
		if( !IsValidRequest(pDns) )
			break;
		
		//只有本机向外发送过DNS请求,才解析. 
		//TODO: 多线程加锁
		m_sendDnsTansID.insert( pDns->id );

	} while (FALSE);

	return FALSE;
}

//////////////////////////////////////////////////////////////////////////

BOOL KDnsRecorder::RecordRecv(
	PROTO_TYPE pt,
	DWORD uLocalIp,
	USHORT uLocalPort,
	DWORD uRemoteIP,
	USHORT uRemotePort,
	PVOID pData, 
	DWORD uDataLen )
{
	DNS_HEADER* pDns = NULL;
	string strHostName;
	list<DWORD> uAddrList;
	BOOL bResult = FALSE;
	DWORD nAnsPos = 0;
	DWORD nAnsCount = 0;

	do 
	{
		if( uDataLen <= sizeof(DNS_HEADER) )
			break;
		
		if( uRemotePort != 53 )
			break;

		pDns = (DNS_HEADER*)pData;
		if( !IsValidResponse(pDns) )
			break;

		if( m_sendDnsTansID.find( pDns->id ) == m_sendDnsTansID.end() )
			break;
		m_sendDnsTansID.erase( pDns->id );

		nAnsPos = GetHost( pData, uDataLen, strHostName );
		if( nAnsPos == 0 || nAnsPos >= uDataLen )
			break;
		
		nAnsCount = ntohs(pDns->ans_count);

		if( !GetHostAddr( pData, uDataLen, nAnsPos, nAnsCount, uAddrList ) )
			break;

		m_hostIpList[strHostName] = uAddrList;
		

		//remove
		{
			struct sockaddr_in a;
			cout << strHostName <<endl;
			list<DWORD> &ipList = uAddrList;
			cout << "-------------------------------"<<endl;
			for( list<DWORD>::iterator ipItem = ipList.begin(); ipItem != ipList.end(); ipItem++ )
			{
				a.sin_addr.s_addr=(*ipItem);    //working without ntohl
				cout << "\t"<< inet_ntoa(a.sin_addr) << endl;
			}
		}


		bResult = TRUE;

	} while (FALSE);

	return bResult;
}


//////////////////////////////////////////////////////////////////////////


VOID KDnsRecorder::Output()
{
	struct sockaddr_in a;
	for(map<string, list<DWORD> >::iterator item = m_hostIpList.begin(); item != m_hostIpList.end(); item ++ )
	{
		cout << item->first <<endl;
		list<DWORD> &ipList = item->second;
		for( list<DWORD>::iterator ipItem = ipList.begin(); ipItem != ipList.end(); ipItem++ )
		{
			a.sin_addr.s_addr=(*ipItem);    //working without ntohl
			cout << "\t"<< inet_ntoa(a.sin_addr) << endl;
		}
		cout << "------------------------"<<endl;
	}
}

//////////////////////////////////////////////////////////////////////////

BOOL KDnsRecorder::IsValidRequest( DNS_HEADER* pDnsHeader )
{
	//return ( pDnsHeader->rd == 1 
	//	&& pDnsHeader->tc == 0
	//	&& pDnsHeader->aa == 0
	//	&& pDnsHeader->opcode == 0
	//	&& pDnsHeader->qr == 0
	//	&& pDnsHeader->rcode == 0
	//	&& pDnsHeader->cd == 0
	//	&& pDnsHeader->ad == 0
	//	&& pDnsHeader->z == 0
	//	&& pDnsHeader->ra == 0 
	//	&& ntohs(pDnsHeader->q_count) == 1
	//	&& pDnsHeader->ans_count == 0
	//	&& pDnsHeader->auth_count == 0
	//	&& pDnsHeader->add_count == 0);

	const BYTE pStandardQuery[] = {0x01,0x00, 0x00, 0x01, 0, 0, 0, 0, 0, 0 };
	return memcmp( (BYTE*)pDnsHeader + sizeof(short), pStandardQuery, sizeof(pStandardQuery) ) == 0;
}

//////////////////////////////////////////////////////////////////////////

BOOL KDnsRecorder::IsValidResponse( DNS_HEADER* pDnsHeader )
{
	return ( pDnsHeader->rd == 1 
		&& pDnsHeader->tc == 0
		&& pDnsHeader->aa == 0
		&& pDnsHeader->opcode == 0
		&& pDnsHeader->qr == 1
		&& pDnsHeader->rcode == 0
		&& pDnsHeader->cd == 0
		&& pDnsHeader->ad == 0
		&& pDnsHeader->z == 0
		&& pDnsHeader->ra == 1 
		&& pDnsHeader->q_count == ntohs(1) 
		&& ntohs(pDnsHeader->ans_count) >= 1 );
}

//////////////////////////////////////////////////////////////////////////

BOOL KDnsRecorder::GetHostAddr( PVOID pData, DWORD uDataLen, DWORD nAnsPos, DWORD nAnsCount, list<DWORD> &listAddr )
{
	BOOL bResult = FALSE;
	R_DATA *pAnsData = NULL;

	for( DWORD i=0; i<nAnsCount; i++ )
	{
		if( (nAnsPos + sizeof(R_DATA)) > uDataLen )
			break;
		pAnsData = (R_DATA *)((PBYTE)pData + nAnsPos);

		nAnsPos += ntohs( pAnsData->data_len ) + sizeof(R_DATA);
		if( (DWORD)nAnsPos > uDataLen )
			break;

		//IPv4 address
		if( ntohs(pAnsData->type) != 1		
			|| ntohs(pAnsData->data_len) != 4 )
			continue;
		
		listAddr.push_back( *((PDWORD)((PBYTE)pAnsData + sizeof(R_DATA))) );
		
	}

	return listAddr.size() != 0;
}

//////////////////////////////////////////////////////////////////////////

int KDnsRecorder::GetHost( PVOID pData, DWORD uDataLen, string &strHostName )
{
	PBYTE pAns = (PBYTE)pData + sizeof( DNS_HEADER );
	int nMaxLen = uDataLen - sizeof(DNS_HEADER);
	int nNameLen = 0;
	int i = 0, j = 0;
	int p = 0;
	for( nNameLen=0; nNameLen<nMaxLen; nNameLen++ )
	{
		if( pAns[nNameLen] == '\0' )
			break;
	}

	if( nNameLen == nMaxLen )
		return FALSE;

	//now convert 3www6google3com0 to www.google.com
	for( i=0; i<nNameLen; i++ )
	{
		p = pAns[i];
		for( j=0; j<p; j++ )
		{
			strHostName.push_back( pAns[++i] );
			if( i == nNameLen )
			{
				goto _end;
			}
		}
		strHostName.push_back( '.' );
	}

	//remove the last dot
	if( strHostName.size() )
	{
		strHostName.erase( strHostName.end() - 1 );
		return nNameLen + 1 + sizeof(QUESTION) + sizeof(DNS_HEADER);
	}

_end:
	return 0;
}