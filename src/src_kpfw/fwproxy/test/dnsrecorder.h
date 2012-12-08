#pragma once

//Type field of Query and Answer
#define T_A		    1		/* host address */
#define T_NS		2		/* authoritative server */
#define T_CNAME		5		/* canonical name */
#define T_SOA		6		/* start of authority zone */
#define T_PTR		12		/* domain name pointer */
#define T_MX		15		/* mail routing information */

//DNS header structure
struct DNS_HEADER
{
	unsigned	short id;		    // identification number

	unsigned	char rd     :1;		// recursion desired
	unsigned	char tc     :1;		// truncated message
	unsigned	char aa     :1;		// authoritive answer
	unsigned	char opcode :4;	    // purpose of message
	unsigned	char qr     :1;		// query/response flag

	unsigned	char rcode  :4;	    // response code
	unsigned	char cd     :1;	    // checking disabled
	unsigned	char ad     :1;	    // authenticated data
	unsigned	char z      :1;		// its z! reserved
	unsigned	char ra     :1;		// recursion available

	unsigned    short q_count;	    // number of question entries
	unsigned	short ans_count;	// number of answer entries
	unsigned	short auth_count;	// number of authority entries
	unsigned	short add_count;	// number of resource entries
};


//Constant sized fields of query structure
struct QUESTION
{
	unsigned short qtype;
	unsigned short qclass;
};


//Constant sized fields of the resource record structure
#pragma pack(push, 1)
struct  R_DATA
{
	unsigned char name_type;
	unsigned char name_offset;
	unsigned short type;
	unsigned short _class;
	unsigned int   ttl;
	unsigned short data_len;
};
#pragma pack(pop)


//Pointers to resource record contents
struct RES_RECORD
{
	unsigned char  *name;
	struct R_DATA  *resource;
	unsigned char  *rdata;
};

//Structure of a Query
typedef struct
{
	unsigned char *name;
	struct QUESTION      *ques;
} QUERY;


class KDnsRecorder
{
public:
	KDnsRecorder(void);
	~KDnsRecorder(void);	
	
	BOOL Record( 
		PROTO_TYPE pt,
		PACKET_DIR dir,
		ULONG uLocalIp,
		USHORT uLocalPort,
		ULONG uRemoteIP,
		USHORT uRemotePort,
		PVOID pData, 
		ULONG uDataLen );

private:

	BOOL RecordSend( 
		PROTO_TYPE pt,
		DWORD uLocalIp,
		USHORT uLocalPort,
		DWORD uRemoteIP,
		USHORT uRemotePort,
		PVOID pData, 
		DWORD uDataLen );

	BOOL RecordRecv( 
		PROTO_TYPE pt,
		DWORD uLocalIp,
		USHORT uLocalPort,
		DWORD uRemoteIP,
		USHORT uRemotePort,
		PVOID pData, 
		DWORD uDataLen );
	VOID Output();
	BOOL IsValidRequest( DNS_HEADER* pDnsHeader );
	BOOL IsValidResponse( DNS_HEADER* pDnsHeader );
	int GetHost( PVOID pData, DWORD uDataLen, string &strHostName );
	BOOL GetHostAddr( PVOID pData, DWORD uDataLen, DWORD nAnsPos, DWORD nAnsCount, list<DWORD> &listAddr );
	map<string, list<DWORD> > m_hostIpList;
	set<unsigned short> m_sendDnsTansID;
};
