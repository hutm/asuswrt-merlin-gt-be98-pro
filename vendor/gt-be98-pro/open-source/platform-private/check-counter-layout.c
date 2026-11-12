#include <stdint.h>
#include <stddef.h>
typedef uint32_t uint32;
typedef struct rtk_stat_port_cntr_s
{
	uint32 ifInOctets_H;	// 0
	uint32 ifInOctets_L;
	uint32 ifOutOctets_H;
	uint32 ifOutOctets_L;
	uint32 ifInUcastPkts_H;
	uint32 ifInUcastPkts_L;
	uint32 ifInMulticastPkts_H;
	uint32 ifInMulticastPkts_L;
	uint32 ifInBroadcastPkts_H;
	uint32 ifInBroadcastPkts_L;
	uint32 ifOutUcastPkts_H;
	uint32 ifOutUcastPkts_L;
	uint32 ifOutMulticastPkts_H;
	uint32 ifOutMulticastPkts_L;
	uint32 ifOutBroadcastPkts_H;
	uint32 ifOutBroadcastPkts_L;

	uint32 ifOutDiscards;
	uint32 dot1dTpPortInDiscards;
	uint32 dot3StatsSingleCollisionFrames;
	uint32 dot3StatMultipleCollisionFrames;
	uint32 dot3sDeferredTransmissions;
	uint32 dot3StatsLateCollisions;
	uint32 dot3StatsExcessiveCollisions;
	uint32 dot3StatsSymbolErrors;
	uint32 dot3ControlInUnknownOpcodes;
	uint32 dot3InPauseFrames;
	uint32 dot3OutPauseFrames;
	uint32 etherStatsDropEvents;
	uint32 tx_etherStatsBroadcastPkts;
	uint32 tx_etherStatsMulticastPkts;
	uint32 tx_etherStatsCRCAlignErrors;
	uint32 rx_etherStatsCRCAlignErrors;
	uint32 tx_etherStatsUndersizePkts;
	uint32 rx_etherStatsUndersizePkts;
	uint32 tx_etherStatsOversizePkts;
	uint32 rx_etherStatsOversizePkts;
	uint32 tx_etherStatsFragments;
	uint32 rx_etherStatsFragments;
	uint32 tx_etherStatsJabbers;
	uint32 rx_etherStatsJabbers;
	uint32 tx_etherStatsCollisions;
	uint32 tx_etherStatsPkts64Octets;
	uint32 rx_etherStatsPkts64Octets;
	uint32 tx_etherStatsPkts65to127Octets;
	uint32 rx_etherStatsPkts65to127Octets;
	uint32 tx_etherStatsPkts128to255Octets;
	uint32 rx_etherStatsPkts128to255Octets;
	uint32 tx_etherStatsPkts256to511Octets;
	uint32 rx_etherStatsPkts256to511Octets;
	uint32 tx_etherStatsPkts512to1023Octets;
	uint32 rx_etherStatsPkts512to1023Octets;
	uint32 tx_etherStatsPkts1024to1518Octets;
	uint32 rx_etherStatsPkts1024to1518Octets;

	uint32 rx_etherStatsUndersizedropPkts;	// 54
	uint32 tx_etherStatsPkts1519toMaxOctets;
	uint32 rx_etherStatsPkts1519toMaxOctets;
	uint32 tx_etherStatsPktsOverMaxOctets;
	uint32 rx_etherStatsPktsOverMaxOctets;
	uint32 tx_etherStatsPktsFlexibleOctetsSET1;
	uint32 rx_etherStatsPktsFlexibleOctetsSET1;
	uint32 tx_etherStatsPktsFlexibleOctetsCRCSET1;
	uint32 rx_etherStatsPktsFlexibleOctetsCRCSET1;
	uint32 tx_etherStatsPktsFlexibleOctetsSET0;
	uint32 rx_etherStatsPktsFlexibleOctetsSET0;
	uint32 tx_etherStatsPktsFlexibleOctetsCRSET0C;
	uint32 rx_etherStatsPktsFlexibleOctetsCRSET0C;
	uint32 lengthFieldError;
	uint32 falseCarrieimes;
	uint32 underSizeOctets;
	uint32 framingErrors;

	uint32 rxMacDiscards;		// 72
	uint32 rxMacIPGShortDropRT;

	uint32 dot1dTpLearnedEntryDiscards;	// 75
	uint32 egrQueue7DropPktRT;
	uint32 egrQueue6DropPktRT;
	uint32 egrQueue5DropPktRT;
	uint32 egrQueue4DropPktRT;
	uint32 egrQueue3DropPktRT;
	uint32 egrQueue2DropPktRT;
	uint32 egrQueue1DropPktRT;
	uint32 egrQueue0DropPktRT;
	uint32 egrQueue7OutPktRT;
	uint32 egrQueue6OutPktRT;
	uint32 egrQueue5OutPktRT;
	uint32 egrQueue4OutPktRT;
	uint32 egrQueue3OutPktRT;
	uint32 egrQueue2OutPktRT;
	uint32 egrQueue1OutPktRT;
	uint32 egrQueue0OutPktRT;
	uint32 TxGoodCnt_H;
	uint32 TxGoodCnt_L;
	uint32 RxGoodCnt_H;
	uint32 RxGoodCnt_L;
	uint32 RxErrorCnt;
	uint32 TxErrorCnt;
	uint32 TxGoodCnt_phy_H;
	uint32 TxGoodCnt_phy_L;
	uint32 RxGoodCnt_phy_H;
	uint32 RxGoodCnt_phy_L;
	uint32 RxErrorCnt_phy;
	uint32 TxErrorCnt_phy;
}rtk_stat_port_cntr_t;
_Static_assert(sizeof(rtk_stat_port_cntr_t)==404,"size");
_Static_assert(offsetof(rtk_stat_port_cntr_t,ifInOctets_L)==4,"ifInOctets_L");
_Static_assert(offsetof(rtk_stat_port_cntr_t,ifOutOctets_L)==12,"ifOutOctets_L");
_Static_assert(offsetof(rtk_stat_port_cntr_t,ifInUcastPkts_L)==20,"ifInUcastPkts_L");
_Static_assert(offsetof(rtk_stat_port_cntr_t,ifInMulticastPkts_L)==28,"ifInMulticastPkts_L");
_Static_assert(offsetof(rtk_stat_port_cntr_t,ifOutUcastPkts_L)==44,"ifOutUcastPkts_L");
_Static_assert(offsetof(rtk_stat_port_cntr_t,ifOutMulticastPkts_L)==52,"ifOutMulticastPkts_L");
_Static_assert(offsetof(rtk_stat_port_cntr_t,ifOutBroadcastPkts_L)==60,"ifOutBroadcastPkts_L");
_Static_assert(offsetof(rtk_stat_port_cntr_t,tx_etherStatsCRCAlignErrors)==120,"tx_etherStatsCRCAlignErrors");
_Static_assert(offsetof(rtk_stat_port_cntr_t,rx_etherStatsCRCAlignErrors)==124,"rx_etherStatsCRCAlignErrors");
