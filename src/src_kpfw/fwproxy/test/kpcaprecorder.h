#pragma once

#define	PCAP_MAGIC			0xa1b2c3d4
#define	PCAP_SWAPPED_MAGIC		0xd4c3b2a1
#define	PCAP_NSEC_MAGIC			0xa1b23c4d
#define	PCAP_SWAPPED_NSEC_MAGIC		0x4d3cb2a1

#define DLT_NULL	0	/* no link-layer encapsulation */
#define DLT_EN10MB	1	/* Ethernet (10Mb) */
#define DLT_EN3MB	2	/* Experimental Ethernet (3Mb) */
#define DLT_AX25	3	/* Amateur Radio AX.25 */
#define DLT_PRONET	4	/* Proteon ProNET Token Ring */
#define DLT_CHAOS	5	/* Chaos */
#define DLT_IEEE802	6	/* IEEE 802 Networks */
#define DLT_ARCNET	7	/* ARCNET, with BSD-style header */
#define DLT_SLIP	8	/* Serial Line IP */
#define DLT_PPP		9	/* Point-to-point Protocol */
#define DLT_FDDI	10	/* FDDI */

typedef unsigned int guint32 ;
typedef bool gboolean;
typedef unsigned short guint16;
typedef int gint32;
typedef unsigned int bpf_u_int32;


struct pcap_pkthdr {
	struct timeval ts;	/* time stamp */
	bpf_u_int32 caplen;	/* length of portion present */
	bpf_u_int32 len;	/* length this packet (off wire) */
};

/* "libpcap" file header. */
struct pcap_hdr {
	guint32 magic;		/* magic number */
	guint16	version_major;	/* major version number */
	guint16	version_minor;	/* minor version number */
	gint32	thiszone;	/* GMT to local correction */
	guint32	sigfigs;	/* accuracy of timestamps */
	guint32	snaplen;	/* max length of captured packets, in octets */
	guint32	network;	/* data link type */
};

/* "libpcap" record header. */
struct pcaprec_hdr {
	guint32	ts_sec;		/* timestamp seconds */
	guint32	ts_usec;	/* timestamp microseconds (nsecs for PCAP_NSEC_MAGIC) */
	guint32	incl_len;	/* number of octets of packet saved in file */
	guint32	orig_len;	/* actual length of packet */
};


class KPcapFileRecorder
{
public:
	KPcapFileRecorder(void);
	~KPcapFileRecorder(void);

	BOOL Record( TCHAR *szFileName,
		PROTO_TYPE pt,
		PACKET_DIR dir,
		ULONG uLocalIp,
		USHORT uLocalPort,
		ULONG uRemoteIP,
		USHORT uRemotePort,
		PVOID pData, 
		ULONG uDataLen
		);

private:
	ULONG MakeUdpPacket(
		PACKET_DIR dir,
		ULONG uLocalIp,
		USHORT uLocalPort,
		ULONG uRemoteIP,
		USHORT uRemotePort,
		PVOID pData, 
		ULONG uDataLen,
		PVOID *pOutBuffer);

	ULONG MakeTcpPacket(
		PACKET_DIR dir,
		ULONG uLocalIp,
		USHORT uLocalPort,
		ULONG uRemoteIP,
		USHORT uRemotePort,
		PVOID pData, 
		ULONG uDataLen,
		PVOID *pOutBuffer);
	
	BOOL WritePacket( TCHAR *szFileName, PVOID pData, ULONG uDataLen );
	BOOL WritePcapHdr();
	HANDLE m_hFile;
};
