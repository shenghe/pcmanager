////////////////////////////////////////////////////////////////////////////////
//      
//      common include File for kpfw
//      
//      File      : kpfw_dpt_hlp.cpp
//      Version   : 1.0
//      Comment   : 定义网镖服务和界面之间的转换辅助类
//      
//      Create at : 2009-07-15
//      Create by : chenguicheng
//      
////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "kpfw_dpt_hlp.h"

namespace Dpt_hlp
{
	void ToMd5( const Kxe_MD5_Param* md5Param, BYTE md5[16] )
	{
		if( md5Param && md5 )
			memcpy( md5, md5Param, sizeof(BYTE)*16 );
	}

	void FromMd5( Kxe_MD5_Param* md5Param, const BYTE md5[16] )
	{
		if ( md5Param && md5 )
			memcpy( md5Param, md5, sizeof(BYTE)*16 );
	}
	
	void ToMAC( const Kxe_MAC_Param* macParam, BYTE mac[6] )
	{
		if( macParam && mac)
			memcpy( mac, macParam, sizeof(BYTE)*6  );
	}

	void FromMAC( Kxe_MAC_Param* macParam, const BYTE mac[6] )
	{
		if( macParam && mac )
			memcpy( macParam, mac, sizeof(BYTE)* 6 );
	}

	void ToGUID( const Kxe_GUID_Param* pGuidParam, GUID* pGuid )
	{
		if( pGuidParam && pGuid)
			memcpy( pGuid, pGuidParam, sizeof(GUID) );
	}

	void FromGUID( Kxe_GUID_Param* pGuidParam, const GUID* pGuid )
	{
		if( pGuidParam && pGuid)
			memcpy( pGuidParam, pGuid, sizeof(GUID) );
	}

	void ToIpFilterAddr( const Kxe_FilterIPAddr_Param& filterParam, FILTER_IP_ADDR& filter )
	{
		filter.eMode = (IP_ADDR_MODE)filterParam.eMode;
		filter.ipAddr = filterParam.IpAddr.ipAddr;
		filter.ipRange = filterParam.IpAddr.ipRange;
	}

	void FromIpFilterAddr( Kxe_FilterIPAddr_Param& filterParam, const FILTER_IP_ADDR& filter )
	{
		filterParam.eMode = (int)(filter.eMode);
		filterParam.IpAddr.ipAddr = filter.ipAddr;
		filterParam.IpAddr.ipRange = filter.ipRange;
	}

	void ToFilterPort( const Kxe_FilterPort_Param& filerParam, FILTER_PORT& filter )
	{
		filter.eMode = (PORT_MODE)filerParam.eMode;
		filter.port = filerParam.port.port;
		filter.portRange = filerParam.port.portRange;
	}
	void FromFilterPort( Kxe_FilterPort_Param& filerParam, const FILTER_PORT& filter )
	{
		filerParam.eMode = (int)(filter.eMode);
		filerParam.port.port = filter.port;
		filerParam.port.portRange = filter.portRange;
	}

	void ToTCP_DATA( const Kxe_TCPData_Param& tcpParam, TCP_EXTRA_DATA& tcp)
	{
		ToFilterPort( tcpParam.LocalPort, tcp.LocalPort );
		ToFilterPort( tcpParam.RemotePort, tcp.RemotePort );
		tcp.byTcpFlags = tcpParam.byTcpFlags;
	}

	void FromTCP_DATA( Kxe_TCPData_Param& tcpParam, const TCP_EXTRA_DATA& tcp)
	{
		FromFilterPort( tcpParam.LocalPort, tcp.LocalPort );
		FromFilterPort( tcpParam.RemotePort, tcp.RemotePort );
		tcpParam.byTcpFlags = tcp.byTcpFlags;
	}

	void ToUDP_DATA( const Kxe_UDPData_Param& udpParam, UDP_EXTRA_DATA& udp )
	{
		ToFilterPort( udpParam.LocalPort, udp.LocalPort );
		ToFilterPort( udpParam.RemotePort, udp.RemotePort );
	}

	void FromUDP_DATA( Kxe_UDPData_Param& udpParam, const UDP_EXTRA_DATA& udp )
	{
		FromFilterPort( udpParam.LocalPort, udp.LocalPort );
		FromFilterPort( udpParam.RemotePort, udp.RemotePort );
	}

	void ToICMP_DATA( const Kxe_ICMPData_Param& icmpParam, ICMP_EXTRA_DATA& icmp )
	{
		icmp.byType = icmpParam.byType;
		icmp.byCode = icmpParam.byCode;
	}

	void FromICMP_DATA( Kxe_ICMPData_Param& icmpParam, const ICMP_EXTRA_DATA& icmp )
	{
		icmpParam.byType = icmp.byType;
		icmpParam.byCode = icmp.byCode;
	}

	void ToIGMP_DATA( const Kxe_IGMPData_Param& igmpParam, IGMP_EXTRA_DATA& igmp )
	{
		igmp.byType = igmpParam.byType;
		igmp.byTypeOfGroup = igmpParam.byTypeOfGroup;
		igmp.uMulticastAddr = igmpParam.uMulticastAddr;
	}

	void FromIGMP_DATA( Kxe_IGMPData_Param& igmpParam, const IGMP_EXTRA_DATA& igmp )
	{
		igmpParam.byType = igmp.byType;
		igmpParam.byTypeOfGroup = igmp.byTypeOfGroup;
		igmpParam.uMulticastAddr = igmp.uMulticastAddr;
	}

	void ToDataUnion( const DataUnion& dataUnion, FILTERINFO_EXTRADATA& data )
	{
		ToTCP_DATA( dataUnion.TCP_DATA, data.TCP_DATA );
		ToUDP_DATA( dataUnion.UDP_DATA, data.UDP_DATA );
		ToICMP_DATA( dataUnion.ICMP_DATA, data.ICMP_DATA );
		ToIGMP_DATA( dataUnion.IGMP_DATA, data.IGMP_DATA );
	}

	void FromDataUnion( DataUnion& dataUnion, const FILTERINFO_EXTRADATA& data )
	{
		
		FromTCP_DATA( dataUnion.TCP_DATA, data.TCP_DATA );
		FromUDP_DATA( dataUnion.UDP_DATA, data.UDP_DATA );
		FromICMP_DATA( dataUnion.ICMP_DATA, data.ICMP_DATA );
		FromIGMP_DATA( dataUnion.IGMP_DATA, data.IGMP_DATA );
	}

	void ToFilterInfo( const Kxe_FilterInfo_Param& filterInfoParam, FILTER_INFO& filterInfo )
	{
		filterInfo.cbSize		= filterInfoParam.cbSize;
		filterInfo.nVer			= filterInfoParam.nVer;
		filterInfo.ProtocolType = (PROTO_TYPE)filterInfoParam.ProtocolType;
		filterInfo.Direction	= (PACKET_DIR)filterInfoParam.Direction;
		filterInfo.Operation	= filterInfoParam.Operation;

		ToIpFilterAddr( filterInfoParam.LocalAddr, filterInfo.LocalAddr );
		ToIpFilterAddr( filterInfoParam.RemoteAddr, filterInfo.RemoteAddr );

		ToDataUnion( filterInfoParam.Data, filterInfo.FilterInfo_ExtraData );
	}

	void FromFilterInfo( Kxe_FilterInfo_Param& filterInfoParam, const FILTER_INFO& filterInfo )
	{
		filterInfoParam.cbSize			= filterInfo.cbSize;
		filterInfoParam.nVer			= filterInfo.nVer;
		filterInfoParam.ProtocolType	= (int)filterInfo.ProtocolType;
		filterInfoParam.Direction		= (int)filterInfo.Direction;
		filterInfoParam.Operation		= filterInfo.Operation;

		FromIpFilterAddr( filterInfoParam.LocalAddr, filterInfo.LocalAddr );
		FromIpFilterAddr( filterInfoParam.RemoteAddr, filterInfo.RemoteAddr );

		FromDataUnion( filterInfoParam.Data, filterInfo.FilterInfo_ExtraData );
	}

	void ToIpFilterInfo( const Kxe_IPFilterInfo_Param& infoParam, KIP_FILTER_INFO& info )
	{
		ToGUID( &(infoParam.guid), &(info.id) );
		ToFilterInfo( infoParam.filterInfo, info.filter );
	}

	void FromIpFilterInfo( Kxe_IPFilterInfo_Param& infoParam, const KIP_FILTER_INFO& info )
	{
		FromGUID( &(infoParam.guid), &(info.id) );
		FromFilterInfo( infoParam.filterInfo, info.filter );
	}

	void ToIpRule( Kxe_IpRule_Param& ipRuleParam, KIpRule& ipRule )
	{
		ipRule.SetCreateDate( ipRuleParam.timeCreate );
		ipRule.SetCreateReason( ATL::CString(ipRuleParam.strReason.c_str() ) );
		ipRule.SetCreator( ATL::CString(ipRuleParam.strCreator.c_str()) );
		ipRule.SetDesc( ATL::CString(ipRuleParam.strDesc.c_str()) );

		ipRule.SetExternData( (LPVOID)(ipRuleParam.pExternData) );
		ipRule.SetName( ATL::CString(ipRuleParam.strName.c_str()) );
		ipRule.SetLastModifyer( ATL::CString(ipRuleParam.strModifyer.c_str()) );
		ipRule.SetLastModifyDate( ipRuleParam.timeModifyer );

		ipRule.SetValid(ipRuleParam.bValid );
		ipRule.SetModify(ipRuleParam.bModify );

		ToIpFilterInfo( ipRuleParam.IpInfo, *(ipRule.GetFilterInfo()) );

	}

	void FromIpRule( Kxe_IpRule_Param& ipRuleParam,  KIpRule& ipRule )
	{
		ipRuleParam.timeCreate = *(ipRule.GetCreateDate());
		ipRuleParam.strReason = ipRule.GetCreateReason();
		ipRuleParam.strCreator = ipRule.GetCreator();
		ipRuleParam.strDesc = ipRule.GetDesc();

		ipRuleParam.pExternData = (DWORD)ipRule.GetExternData();
		ipRuleParam.strName = ipRule.GetName();
		ipRuleParam.strModifyer = ipRule.GetLastModifyer();
		ipRuleParam.timeModifyer = *(ipRule.GetLastModifyDate());

		ipRuleParam.bValid = ipRule.GetValid();
		ipRuleParam.bModify = ipRule.GetModify();

		FromIpFilterInfo( ipRuleParam.IpInfo, *(ipRule.GetFilterInfo()) );
	}

	void ToIpRuleVec( Kxe_IpRuleVec_Param& ruleParam, KIpRuleVec& rule )
	{	
		KIpRule* pRule = NULL;

		rule.Clear();
		rule.SetTheSameCnt( ruleParam.m_nTheSameRuleCnt );

		for( int i = 0; i < (int)ruleParam.nRec.size(); i++ )
		{
			pRule = new KIpRule;
			if ( pRule )
			{
				ToIpRule( ruleParam.nRec[i], *pRule );
				rule.AddRule( pRule );
			}
		}
	}

	void FromIpRuleVec( Kxe_IpRuleVec_Param& ruleParam, KIpRuleVec& rule )
	{
		ruleParam.nRec.clear();

		ruleParam.m_nTheSameRuleCnt = rule.GetTheSameCnt();
		for( int i = 0; i < rule.GetRuleCnt(); i++ )
		{
			Kxe_IpRule_Param ipRuleParam;
			FromIpRule( ipRuleParam, *(rule.GetRule(i)) );
			ruleParam.nRec.push_back( ipRuleParam );
		}
	}


	void ToIpRuleVec( Kxe_IpRuleVec_Ret& ruleParam, KIpRuleVec& rule )
	{	
		KIpRule* pRule = NULL;

		rule.Clear();
		rule.SetTheSameCnt( ruleParam.m_nTheSameRuleCnt );

		for( int i = 0; i < (int)ruleParam.nRec.size(); i++ )
		{
			pRule = new KIpRule;
			if ( pRule )
			{
				ToIpRule( ruleParam.nRec[i], *pRule );
				rule.AddRule( pRule );
			}
		}
	}

	void FromIpRuleVec( Kxe_IpRuleVec_Ret& ruleParam, KIpRuleVec& rule )
	{
		ruleParam.nRec.clear();

		ruleParam.m_nTheSameRuleCnt = rule.GetTheSameCnt();
		for( int i = 0; i < rule.GetRuleCnt(); i++ )
		{
			Kxe_IpRule_Param ipRuleParam;
			FromIpRule( ipRuleParam, *(rule.GetRule(i)) );
			ruleParam.nRec.push_back( ipRuleParam );
		}
	}

	void ToArea( Kxe_Area_Param& areaParam, KArea& area )
	{
		area.SetName( ATL::CString(areaParam.strName.c_str()) );
		area.SetType( areaParam.nAreaType );

		BYTE byteMac[6]  = {0};
		ToMAC( &(areaParam.vecGateMac), byteMac );
		area.SetGateMac( byteMac );

		area.SetCreateTime( &(areaParam.ftCreate) );
		area.SetGateIp( areaParam.uIp );
		area.SetWireless( areaParam.bWireless );
		area.SetLastChangeTime( areaParam.nLastChangeTime );
	}


	void FromArea( Kxe_Area_Param& areaParam, KArea& area )
	{
		areaParam.strName			= area.GetName();
		areaParam.nAreaType			= area.GetType();

		BYTE byteMac[6] = {0};
		area.GetGateMac( byteMac );
		FromMAC( &(areaParam.vecGateMac), byteMac );

		areaParam.ftCreate			= *(area.GetCreateTime());
		areaParam.uIp				= area.GetGateIp();
		areaParam.bWireless			= area.GetWireless();
		areaParam.nLastChangeTime	= area.GetLastChangeTime();
	}

	void ToAreaVec( Kxe_AreaVec_Ret& areaParam, KAreaVec& areaVec )
	{
		areaVec.Clear();

		areaVec.SetOwn( areaParam.bOwn );

		KArea* pArea = NULL;
		for ( int i = 0; i < (int)areaParam.vecArea.size(); i ++ )
		{
			pArea = new KArea;
			ToArea( areaParam.vecArea[i], *pArea );
			areaVec.AddArea( pArea );
		}
	}

	void FromAreaVec( Kxe_AreaVec_Ret& areavecParam, KAreaVec& areaVec )
	{
		areavecParam.vecArea.clear();
		KArea* pArea = NULL;

		areavecParam.bOwn = areaVec.GetOwn();

		for ( int i = 0; i < areaVec.GetAreaCnt(); i++ )
		{
			pArea = areaVec.GetArea(i);
			if ( pArea )
			{
				Kxe_Area_Param areaParam;
				FromArea( areaParam, *pArea );
				areavecParam.vecArea.push_back( areaParam );
			}
		}
	}

	void ToAttackInfo( const Kxe_AttackInfo_Param& attackInfoParam, AttackInfo& attackInfo )
	{
		attackInfo.attackAddr = attackInfoParam.attackAddr;
		ToGUID( &(attackInfoParam.nReason), &(attackInfo.nReason) );
		attackInfo.nAttackTimes = attackInfoParam.nAttackTimes;
		attackInfo.nTime = attackInfoParam.nTime;
	}

	void FromAttackInfo( Kxe_AttackInfo_Param& attackInfoParam, const AttackInfo& attackInfo )
	{
		attackInfoParam.attackAddr		= attackInfo.attackAddr;
		FromGUID( &(attackInfoParam.nReason), &(attackInfo.nReason) );
		attackInfoParam.nAttackTimes	= attackInfo.nAttackTimes;
		attackInfoParam.nTime			= attackInfo.nTime;
	}

	void ToAttackList( const Kxe_AttackList_Ret& attackListParam, vector<AttackInfo>& vecAttack )
	{
		vecAttack.clear();

		for ( int i = 0; i < (int)attackListParam.attackList.size(); i ++ )
		{
			AttackInfo attackInfo;
			ToAttackInfo( attackListParam.attackList[i], attackInfo );
			vecAttack.push_back( attackInfo );
		}
	}

	void FromAttackList( Kxe_AttackList_Ret& attackListParam, const vector<AttackInfo>& vecAttack )
	{
		attackListParam.attackList.clear();

		for ( int i = 0; i < (int)vecAttack.size(); i++ )
		{
			Kxe_AttackInfo_Param attackInfo;
			FromAttackInfo( attackInfo, vecAttack[i] );
			attackListParam.attackList.push_back( attackInfo );
		}
	}

	void ToKpfwLog( const Kxe_KpfwLog_Param& logParam, KpfwLog& log )
	{
		log.nID = logParam.nID;
		log.nType = logParam.nType;
		log.nSize = logParam.nSize;
		log.strData.clear();
		
		for ( int i = 0; i < logParam.vecData.size(); i++ )
		{
			log.strData.append( 1, logParam.vecData[i] );
		}
	}

	void FromKpfwLog( Kxe_KpfwLog_Param& logParam, const KpfwLog& log )
	{
		logParam.nID = log.nID;
		logParam.nType  =log.nType;
		logParam.nSize = log.nSize;
		logParam.vecData.clear();
		int nLen = log.strData.length();
		for ( int i = 0; i < nLen; i++ )
		{
			logParam.vecData.push_back( log.strData[i] );
		}	
	}

	void ToEventInfo( Kxe_EventInfo_Ret& infoParam, KNetEventInfo& info )
	{
		info.SetMax( infoParam.nMax );
		
		KpfwLog* pLog = NULL;
		for( int i = 0; i < (int)infoParam.Logs.size() ; i++ )
		{
			pLog = new KpfwLog;
			if ( pLog )
			{
				ToKpfwLog( infoParam.Logs[i], *pLog );
				info.AddEvent( pLog );
			}
		}
	}

	void FromEventInfo( Kxe_EventInfo_Ret& infoParam, KNetEventInfo& info )
	{
		infoParam.Logs.clear();

		infoParam.nMax =  info.GetMax();

		for( int i = 0; i < info.GetCount() ; i++ )
		{
			Kxe_KpfwLog_Param logParam;
			FromKpfwLog( logParam, *(info.GetLog(i)) );
			infoParam.Logs.push_back( logParam );
		}
	}

	void ToAppRule( Kxe_AppRule_Param& ruleParam, KAppRule& rule )
	{
		ToGUID( &(ruleParam.id), &(rule.m_id) );

		rule.SetType( (AppRuleType)(ruleParam.nRuleType) );
		rule.SetExePath( (LPWSTR)(ruleParam.strExePath.c_str()) );

		ToMd5( &(ruleParam.checkSum), rule.m_checkSum );

		rule.SetLastFileTime( ruleParam.ftLast );
		rule.SetLastTrustRepTime( ruleParam.ftLastTrustRep );
		rule.SetCreator( (LPWSTR)(ruleParam.strCreator.c_str()) );
		rule.SetCreateDate( ruleParam.ftCreate );
		rule.SetCreateReason( (LPWSTR)(ruleParam.strReason.c_str()) );
		rule.SetLastModifyDate( ruleParam.ftLastModify );
		rule.SetLastModifyer( (LPWSTR)(ruleParam.m_strModifyer.c_str()) );

		rule.m_tcpRemoteDenyRange = ruleParam.tcpRemoteDenyRange;
		rule.m_tcpLocalDenyRange = ruleParam.tcpLocalDenyRange;
		rule.m_udpRemoteDenyRange = ruleParam.udpRemoteDenyRange;
		rule.m_udpLocalDenyRange = ruleParam.udpLocalDenyRange;

		rule.m_userMode = ruleParam.nUserMode;
		rule.m_trustMode = ruleParam.nTrustMode;
		rule.m_pathhashCode = ruleParam.lPathhashCode;
		rule.m_pExternData = (LPVOID)(ruleParam.pExternData);
	}

	void FromAppRule( Kxe_AppRule_Param& ruleParam, KAppRule& rule )
	{
		FromGUID( &(ruleParam.id), &(rule.m_id) );

		ruleParam.nRuleType = (int)( rule.GetType() );
		ruleParam.strExePath = rule.GetExePath();

		FromMd5( &(ruleParam.checkSum), rule.m_checkSum );

		ruleParam.ftLast = *(rule.GetLastFileTime());
		ruleParam.ftLastTrustRep = *(rule.GetLastTrustRepTime());
		ruleParam.strCreator = rule.GetCreator();
		ruleParam.ftCreate = *(rule.GetCreateDate());
		ruleParam.strReason = rule.GetCreateReason();
		ruleParam.ftLastModify = *(rule.GetLastModifyDate());
		ruleParam.m_strModifyer = rule.GetLastModifyer();

		ruleParam.tcpRemoteDenyRange = rule.m_tcpRemoteDenyRange;
		ruleParam.tcpLocalDenyRange = rule.m_tcpLocalDenyRange;
		ruleParam.udpRemoteDenyRange = rule.m_udpRemoteDenyRange;
		ruleParam.udpLocalDenyRange = rule.m_udpLocalDenyRange;

		ruleParam.nUserMode = rule.m_userMode;
		ruleParam.nTrustMode = rule.m_trustMode;
		ruleParam.lPathhashCode = rule.m_pathhashCode;
		ruleParam.pExternData = (DWORD)(rule.m_pExternData);
	}


	void ToAppRuleVec( Kxe_AppRuleVec_Param& vecParam, KAppRuleVec& vec )
	{
		vec.Clear();

		KAppRule* pAppRule = NULL;
		for ( int i = 0; i < (int)vecParam.vecAppRules.size(); i++ )
		{
			pAppRule = new KAppRule;
			if ( pAppRule )
			{
				ToAppRule( vecParam.vecAppRules[i], *pAppRule );
				vec.AddRule( pAppRule );
			}
		}
	}


	void FromAppRuleVec(Kxe_AppRuleVec_Param&  vecParam, KAppRuleVec& rule )
	{
		vecParam.vecAppRules.clear();

		for ( int i = 0; i < rule.GetRuleCnt() ; i++ )
		{
			Kxe_AppRule_Param ruleParam;
			FromAppRule( ruleParam, *(rule.GetRule(i)) );
			vecParam.vecAppRules.push_back( ruleParam );
		}
	}

	void ToAppRuleVec( Kxe_AppRuleVec_Ret& vecParam, KAppRuleVec& vec )
	{
		vec.Clear();

		KAppRule* pAppRule = NULL;
		for ( int i = 0; i < (int)vecParam.vecAppRules.size(); i++ )
		{
			pAppRule = new KAppRule;
			if ( pAppRule )
			{
				ToAppRule( vecParam.vecAppRules[i], *pAppRule );
				vec.AddRule( pAppRule );
			}
		}
	}


	void FromAppRuleVec(Kxe_AppRuleVec_Ret&  vecParam, KAppRuleVec& rule )
	{
		vecParam.vecAppRules.clear();

		for ( int i = 0; i < rule.GetRuleCnt() ; i++ )
		{
			Kxe_AppRule_Param ruleParam;
			FromAppRule( ruleParam, *(rule.GetRule(i)) );
			vecParam.vecAppRules.push_back( ruleParam );
		}
	}


	void ToMacIp( const Kxe_MacIp_Param& macipParam, mac_ip& macip )
	{
		ToMAC( &(macipParam.mac), macip.mac );
		macip.ip = macipParam.ip;
	}
	
	void FromMacIp( Kxe_MacIp_Param& macipParam, const mac_ip& macip )
	{
		FromMAC( &(macipParam.mac), macip.mac );
		macipParam.ip = macip.ip;
	}


	void ToMacIp( const Kxe_MacIp_Ret& macipParam, mac_ip& macip )
	{
		ToMAC( &(macipParam.macIp.mac), macip.mac );
		macip.ip = macipParam.macIp.ip;
	}

	void FromMacIp( Kxe_MacIp_Ret& macipParam, const mac_ip& macip )
	{
		FromMAC( &(macipParam.macIp.mac), macip.mac );
		macipParam.macIp.ip = macip.ip;
	}


	void ToArpHostInfo( const Kxe_ArpHostInfo_Ret& infoParam, KArpHostInfo& info )
	{
		info.m_nHostIpCnt = infoParam.hostInfo.nHostIpCnt;

		for( int i = 0; i < (int)infoParam.hostInfo.vecHostMacIp.size() && i < max_hostip_count; i++ )
		{
			ToMacIp( infoParam.hostInfo.vecHostMacIp[i], info.m_HostIp[i] );
		}

		info.m_nGateWayCnt = infoParam.hostInfo.nGateWayCnt;

		for( int i = 0; i < (int)infoParam.hostInfo.vecGetWay.size() && i < max_hostip_count; i++ )
		{
			ToMacIp( infoParam.hostInfo.vecGetWay[i], info.m_GateWay[i] );
		}
	}

	void FromArpHostInfo( Kxe_ArpHostInfo_Ret& infoParam, const KArpHostInfo& info )
	{
		infoParam.hostInfo.nHostIpCnt = info.m_nHostIpCnt;

		for( int i = 0; i < infoParam.hostInfo.nHostIpCnt && i < max_hostip_count; i++ )
		{
			Kxe_MacIp_Param macIp;
			FromMacIp( macIp, info.m_HostIp[i] );
			infoParam.hostInfo.vecHostMacIp.push_back( macIp );
		}

		infoParam.hostInfo.nGateWayCnt = info.m_nGateWayCnt;

		for( int i = 0; i < infoParam.hostInfo.nGateWayCnt && i < max_hostip_count; i++ )
		{
			Kxe_MacIp_Param macIp;
			FromMacIp( macIp, info.m_GateWay[i] );
			infoParam.hostInfo.vecGetWay.push_back( macIp );
		}
	}

	void ToAdapterInfo( const Kxe_KAdapterInfo_Param& infoParam, KAdapterInfo& info )
	{
		info.m_ConnGuidId =  infoParam.ConnGuidId.c_str();
		info.m_ConnName =  infoParam.ConnName.c_str();
		info.m_ConnStatus =  (NETCON_STATUS) infoParam.ConnStatus;
		info.m_ConnMedia =  (NETCON_MEDIATYPE)infoParam.ConnMedia;
		info.m_DeviceName =  infoParam.DeviceName.c_str();
		info.m_uHostIp =  infoParam.uHostIp;
		info.m_uGatewayIp =  infoParam.uGatewayIp;

		ToMAC( &(infoParam.vecHostMac), info.m_HostMac );
		ToMAC( &(infoParam.vecGatewayMac), info.m_GatewayMac );

		info.m_strHostName =  infoParam.strHostName.c_str();
		info.m_strGateName =  infoParam.strGateName.c_str();
		info.m_bWireless =  infoParam.bWireless;
		info.m_bAttackGate =  infoParam.bAttackGate;
		info.m_bAttackInter =  infoParam.bAttackInter;

		info.m_adapterFlux = MAKEINT64( infoParam.adapterFlux.dwLow, infoParam.adapterFlux.dwHigh );
	}

	void FromAdapterInfo( Kxe_KAdapterInfo_Param& infoParam, const KAdapterInfo& info )
	{
		infoParam.ConnGuidId = info.m_ConnGuidId;
		infoParam.ConnName = info.m_ConnName;
		infoParam.ConnStatus = (int)info.m_ConnStatus;
		infoParam.ConnMedia = (int)info.m_ConnMedia;
		infoParam.DeviceName = info.m_DeviceName;
		infoParam.uHostIp = info.m_uHostIp;
		infoParam.uGatewayIp = info.m_uGatewayIp;

		FromMAC( &(infoParam.vecHostMac), info.m_HostMac );
		FromMAC( &(infoParam.vecGatewayMac), info.m_GatewayMac );

		infoParam.strHostName  = info.m_strHostName;
		infoParam.strGateName = info.m_strGateName;
		infoParam.bWireless = info.m_bWireless;
		infoParam.bAttackGate = info.m_bAttackGate;
		infoParam.bAttackInter = info.m_bAttackInter;

		infoParam.adapterFlux.dwLow = LOINT64( info.m_adapterFlux );
		infoParam.adapterFlux.dwHigh = HIINT64( info.m_adapterFlux );
	}
	
}// Dpt_hlp