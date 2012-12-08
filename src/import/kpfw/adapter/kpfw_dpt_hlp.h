////////////////////////////////////////////////////////////////////////////////
//      
//      common include File for kpfw
//      
//      File      : kpfw_dpt_hlp.h
//      Version   : 1.0
//      Comment   : 定义网镖服务和界面之间的转换辅助类
//      
//      Create at : 2009-07-15
//      Create by : chenguicheng
//      
////////////////////////////////////////////////////////////////////////////////

#include "kpfw/kpfw_dpt_def.h"

namespace Dpt_hlp
{

	void ToGUID( const Kxe_GUID_Param* guidParam, GUID* guid );
	void FromGUID( Kxe_GUID_Param* pGuiParam, const GUID* pGuid );

	void ToIpFilterAddr( const Kxe_FilterIPAddr_Param& filterParam, FILTER_IP_ADDR& filter );
	void FromFilterAddr( Kxe_FilterIPAddr_Param& filterParam, const FILTER_IP_ADDR& filter );

	void ToFilterPort( const Kxe_FilterPort_Param& filerParam, FILTER_PORT& filter );
	void FromFilterPort( Kxe_FilterPort_Param& filerParam, const FILTER_PORT& filter );

	void ToTCP_DATA( const Kxe_TCPData_Param& tcpParam, TCP_EXTRA_DATA& tcp);
	void FromTCP_DATA( Kxe_TCPData_Param& tcpParam, const TCP_EXTRA_DATA& tcp);

	void ToUDP_DATA( const Kxe_UDPData_Param& udpParam, UDP_EXTRA_DATA& udp );
	void FromUDP_DATA( Kxe_UDPData_Param& udpParam, const UDP_EXTRA_DATA& udp );

	void ToICMP_DATA( const Kxe_ICMPData_Param& icmpParam, ICMP_EXTRA_DATA& icmp );
	void FromICMP_DATA( Kxe_ICMPData_Param& icmpParam, const ICMP_EXTRA_DATA& icmp );

	void ToIGMP_DATA( const Kxe_IGMPData_Param& igmpParam, IGMP_EXTRA_DATA& igmp );
	void FromIGMP_DATA( Kxe_IGMPData_Param& igmpParam, const IGMP_EXTRA_DATA& igmp );

	void ToDataUnion( const DataUnion& dataUnion, FILTERINFO_EXTRADATA& data );
	void FromDataUnion( DataUnion& dataUnion, const FILTERINFO_EXTRADATA& data );

	void ToFilterInfo( const Kxe_FilterInfo_Param& filterInfoParam, FILTER_INFO& filterInfo );
	void FromFilterInfo( Kxe_FilterInfo_Param& filterInfoParam, const FILTER_INFO& filterInfo );

	void ToIpFilterInfo( const Kxe_IPFilterInfo_Param& infoParam, KIP_FILTER_INFO& info );
	void FromIpFilterInfo( Kxe_IPFilterInfo_Param& infoParam, const KIP_FILTER_INFO& info );

	void ToIpRule( Kxe_IpRule_Param& ipRuleParam, KIpRule& ipRule );
	void FromIpRule( Kxe_IpRule_Param& ipRuleParam, KIpRule& ipRule );

	void ToIpRuleVec( Kxe_IpRuleVec_Param& ruleParam, KIpRuleVec& rule );
	void FromIpRuleVec( Kxe_IpRuleVec_Param& ruleParam, KIpRuleVec& rule );

	void ToIpRuleVec( Kxe_IpRuleVec_Ret& ruleParam, KIpRuleVec& rule );
	void FromIpRuleVec( Kxe_IpRuleVec_Ret& ruleParam, KIpRuleVec& rule );

	void ToMAC( const Kxe_MAC_Param* macParam, BYTE mac[6] );
	void FromMAC( Kxe_MAC_Param* macParam, const BYTE mac[6] );

	void ToArea( Kxe_Area_Param& areaParam, KArea& area );
	void FromArea( Kxe_Area_Param& areaParam, KArea& area );

	void ToAreaVec( Kxe_AreaVec_Ret& areaParam, KAreaVec& areaVec );
	void FromAreaVec( Kxe_AreaVec_Ret& areavecParam, KAreaVec& areaVec );

	void ToAttackInfo( const Kxe_AttackInfo_Param& attackInfoParam, AttackInfo& attackInfo );
	void FromAttackInfo( Kxe_AttackInfo_Param& attackInfoParam, const AttackInfo& attackInfo );

	void FromAttackList( Kxe_AttackList_Ret& attackListParam, const vector<AttackInfo>& vecAttack );
	void ToAttackList( const Kxe_AttackList_Ret& attackListParam, vector<AttackInfo>& vecAttack );

	void FromKpfwLog( Kxe_KpfwLog_Param& logParam, const KpfwLog& log );
	void ToKpfwLog( const Kxe_KpfwLog_Param& logParam, KpfwLog& log );

	void FromEventInfo( Kxe_EventInfo_Ret& infoParam, KNetEventInfo& info );
	void ToEventInfo( Kxe_EventInfo_Ret& infoParam, KNetEventInfo& info );

	void FromMd5( Kxe_MD5_Param* md5Param, const BYTE md5[16] );
	void ToMd5( const Kxe_MD5_Param* md5Param, BYTE md5[16] );

	void FromAppRule( Kxe_AppRule_Param& ruleParam, KAppRule& rule );
	void ToAppRule( Kxe_AppRule_Param& ruleParam, KAppRule& rule );

	void FromAppRuleVec(Kxe_AppRuleVec_Param&  vecParam, KAppRuleVec& rule );
	void ToAppRuleVec( Kxe_AppRuleVec_Param& vecParam, KAppRuleVec& vec );

	void FromAppRuleVec(Kxe_AppRuleVec_Ret&  vecParam, KAppRuleVec& rule );
	void ToAppRuleVec( Kxe_AppRuleVec_Ret& vecParam, KAppRuleVec& vec );

	void ToMacIp( const Kxe_MacIp_Param& macipParam, mac_ip& macip );
	void FromMacIp( Kxe_MacIp_Param& macipParam, const mac_ip& macip );

	void ToMacIp( const Kxe_MacIp_Ret& macipParam, mac_ip& macip );
	void FromMacIp( Kxe_MacIp_Ret& macipParam, const mac_ip& macip );

	void FromArpHostInfo( Kxe_ArpHostInfo_Ret& infoParam, const KArpHostInfo& info );
	void ToArpHostInfo( const Kxe_ArpHostInfo_Ret& infoParam, KArpHostInfo& info );

	void ToAdapterInfo( const Kxe_KAdapterInfo_Param& infoParam, KAdapterInfo& info );
	void FromAdapterInfo( Kxe_KAdapterInfo_Param& infoParam, const KAdapterInfo& info );

}// Dpt_hlp
