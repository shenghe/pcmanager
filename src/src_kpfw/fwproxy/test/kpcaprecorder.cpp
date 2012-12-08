#include "StdAfx.h"
#include "kpcaprecorder.h"

VOID WriteDateToFile( TCHAR *szFileName, PVOID pBuffer, DWORD dwLength );

//////////////////////////////////////////////////////////////////////////

KPcapFileRecorder::KPcapFileRecorder(void):m_hFile(INVALID_HANDLE_VALUE)
{
}

//////////////////////////////////////////////////////////////////////////

KPcapFileRecorder::~KPcapFileRecorder(void)
{
}

//////////////////////////////////////////////////////////////////////////

BOOL KPcapFileRecorder::Record( 
	TCHAR *szFileName,
	PROTO_TYPE pt,
	PACKET_DIR dir,
	ULONG uLocalIp,
	USHORT uLocalPort,
	ULONG uRemoteIP,
	USHORT uRemotePort,
	PVOID pData, 
	ULONG uDataLen )
{
	BOOL bResult = FALSE;
	PVOID pBuffer = NULL;
	ULONG uPackSize = 0;
	
	switch( pt )
	{
	case enumPT_TCP:
		{
			uPackSize = MakeTcpPacket( dir, uLocalIp, uLocalPort, uRemoteIP, uRemotePort, pData, uDataLen, &pBuffer );
		}
		break;
	case enumPT_UDP:
		{
			uPackSize = MakeUdpPacket( dir, uLocalIp, uLocalPort, uRemoteIP, uRemotePort, pData, uDataLen, &pBuffer );
		}
		break;
	}
	
	if( uPackSize )
	{
		bResult = WritePacket( szFileName, pBuffer, uPackSize );
		free( pBuffer );
		pBuffer = NULL;
	}
	return bResult;
}

//////////////////////////////////////////////////////////////////////////

ULONG KPcapFileRecorder::MakeUdpPacket(
	PACKET_DIR dir,
	ULONG uLocalIp,
	USHORT uLocalPort,
	ULONG uRemoteIP,
	USHORT uRemotePort,
	PVOID pData, 
	ULONG uDataLen,
	PVOID *pOutBuffer)
{
	UDP_PACKET *udpPacket = (UDP_PACKET *)malloc( sizeof(UDP_PACKET) + uDataLen );
	if( udpPacket == NULL )
		return 0;
	ZeroMemory( (char*)udpPacket, sizeof(UDP_PACKET) + uDataLen );
	
	udpPacket->ethHdr.EthType = ETHER_IP;

	udpPacket->ipHdr.byHeaderLen = 5;
	udpPacket->ipHdr.byIPVersion = 4;
	udpPacket->ipHdr.byProtocolType = IPHEADER_UDP;
	udpPacket->ipHdr.uSourceAddress = dir == enumPD_Send ? uLocalIp : uRemoteIP;
	udpPacket->ipHdr.uDestinationAddress = dir == enumPD_Send ? uRemoteIP : uLocalIp ;
	udpPacket->ipHdr.uTotalLen = htons( (USHORT)sizeof(IPHEADER) + (USHORT)sizeof(UDPHEADER) + (USHORT)uDataLen );

	udpPacket->udpHdr.uCheckSum = 0;
	udpPacket->udpHdr.uSourcePort = htons( dir == enumPD_Send ? uLocalPort : uRemotePort );
	udpPacket->udpHdr.uDestinationPort = htons( dir == enumPD_Send ? uRemotePort : uLocalPort );
	udpPacket->udpHdr.uTotalLength = htons( (USHORT)sizeof(UDPHEADER) + (USHORT)uDataLen );
	memcpy( (PCHAR)udpPacket + sizeof(UDP_PACKET), pData, uDataLen );
	
	*pOutBuffer = (PVOID)udpPacket;
	return sizeof(UDP_PACKET) + uDataLen;
}

//////////////////////////////////////////////////////////////////////////

ULONG KPcapFileRecorder::MakeTcpPacket(
	PACKET_DIR dir,
	ULONG uLocalIp,
	USHORT uLocalPort,
	ULONG uRemoteIP,
	USHORT uRemotePort,
	PVOID pData, 
	ULONG uDataLen,
	PVOID *pOutBuffer)
{
	TCP_PACKET *tcpPacket = (TCP_PACKET *)malloc( sizeof(TCP_PACKET) + uDataLen );
	if( tcpPacket == NULL )
		return 0;
	ZeroMemory( (char*)tcpPacket, sizeof(TCP_PACKET) + uDataLen );

	tcpPacket->ethHdr.EthType = ETHER_IP;

	tcpPacket->ipHdr.byHeaderLen = 5;
	tcpPacket->ipHdr.byIPVersion = 4;
	tcpPacket->ipHdr.byProtocolType = IPHEADER_TCP;
	tcpPacket->ipHdr.uSourceAddress =  dir == enumPD_Send ? uLocalIp : uRemoteIP;
	tcpPacket->ipHdr.uDestinationAddress = dir == enumPD_Send ? uRemoteIP : uLocalIp;
	tcpPacket->ipHdr.uTotalLen = htons( (USHORT)sizeof(IPHEADER) + (USHORT)sizeof(TCPHEADER) + (USHORT)uDataLen );

	tcpPacket->tcpHdr.uCheckSum = 0;
	tcpPacket->tcpHdr.uSourcePort = htons( dir == enumPD_Send ? uLocalPort : uRemotePort );
	tcpPacket->tcpHdr.uDestinationPort = htons( dir == enumPD_Send ? uRemotePort : uLocalPort );
	tcpPacket->tcpHdr.HeaderLen_And_Flags.Details.byHeaderLen = 5;
	tcpPacket->tcpHdr.HeaderLen_And_Flags.Details.byACK = 1;
	tcpPacket->tcpHdr.HeaderLen_And_Flags.Details.byPSH = 1;
	
	memcpy( (PCHAR)tcpPacket + sizeof(TCP_PACKET), pData, uDataLen );

	*pOutBuffer = (PVOID)tcpPacket;
	return sizeof(TCP_PACKET) + uDataLen;

}					

//////////////////////////////////////////////////////////////////////////

BOOL KPcapFileRecorder::WritePacket( 
	TCHAR *szFileName, 
	PVOID pData, 
	ULONG uDataLen )
{
	DWORD dwWritten = 0;
	struct pcaprec_hdr rec_hdr;
	
	time( (time_t*)&rec_hdr.ts_sec );
	rec_hdr.incl_len = uDataLen;
	rec_hdr.orig_len = uDataLen;

	if( m_hFile == INVALID_HANDLE_VALUE )
	{
		//TODO:·Åµ½ Init()
		m_hFile = CreateFile( szFileName, GENERIC_READ | GENERIC_WRITE, 
					FILE_SHARE_WRITE | FILE_SHARE_READ, 
					0, 
					OPEN_ALWAYS,
					FILE_ATTRIBUTE_NORMAL,
					NULL);
		if( m_hFile == INVALID_HANDLE_VALUE )
			return FALSE;

		if( ERROR_ALREADY_EXISTS != GetLastError() )
		{
			WritePcapHdr();
		}
	}

	if( m_hFile == INVALID_HANDLE_VALUE )
		return FALSE;
	
	if( !WriteFile( m_hFile, (char*)&rec_hdr, sizeof(rec_hdr), &dwWritten, 0 ) )
		return FALSE;

	if( !WriteFile( m_hFile, (char*)pData, uDataLen, &dwWritten, 0 ) )
		return FALSE;

	return FALSE;
}

//////////////////////////////////////////////////////////////////////////

BOOL KPcapFileRecorder::WritePcapHdr()
{
	struct pcap_hdr file_hdr;
	DWORD dwWritten;

	file_hdr.magic = PCAP_MAGIC;
	file_hdr.version_major = 2;
	file_hdr.version_minor = 4;
	file_hdr.thiszone = 0;	/* XXX - current offset? */
	file_hdr.sigfigs = 0;	/* unknown, but also apparently unused */
	file_hdr.snaplen = 0;
	file_hdr.network = DLT_EN10MB;
	
	return WriteFile( m_hFile, (char*)&file_hdr, sizeof(file_hdr), &dwWritten, 0 );
}

//////////////////////////////////////////////////////////////////////////

