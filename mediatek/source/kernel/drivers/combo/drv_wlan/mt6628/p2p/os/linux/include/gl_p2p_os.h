





#ifndef _GL_P2P_OS_H
#define _GL_P2P_OS_H










struct _GL_P2P_INFO_T {

    /* Device handle */
    struct net_device           *prDevHandler;

    #if CFG_ENABLE_WIFI_DIRECT_CFG_80211
    /* cfg80211 */
    struct wireless_dev         wdev;

    struct cfg80211_scan_request    *prScanRequest;

    UINT_64 u8Cookie;

    /* Generation for station list update. */
    INT_32 i4Generation;

    #endif

    /* Device statistics */
    struct net_device_stats     rNetDevStats;

    /* glue layer variables */
    /*move to glueinfo->adapter*/
   // BOOLEAN                     fgIsRegistered;
    UINT_32                     u4FreqInKHz;    /* frequency */
    UINT_8                      ucRole;         /* 0: P2P Device, 1: Group Client, 2: Group Owner */
    UINT_8                      ucIntent;       /* range: 0-15 */
    UINT_8                      ucScanMode;     /* 0: Search & Listen, 1: Scan without probe response */

    ENUM_PARAM_MEDIA_STATE_T    eState;
    UINT_32                     u4PacketFilter;
    PARAM_MAC_ADDRESS           aucMCAddrList[MAX_NUM_GROUP_ADDR];

    /* connection-requested peer information */
    UINT_8                      aucConnReqDevName[32];
    INT_32                      u4ConnReqNameLength;
    PARAM_MAC_ADDRESS           rConnReqPeerAddr;
    PARAM_MAC_ADDRESS           rConnReqGroupAddr;   /* For invitation group. */
    UINT_8                      ucConnReqDevType;
    INT_32                      i4ConnReqConfigMethod;
    INT_32                      i4ConnReqActiveConfigMethod;

    UINT_32                     u4CipherPairwise;
    UINT_8                      ucWSCRunning;

    UINT_8                      aucWSCIE[3][400]; /* 0 for beacon, 1 for probe req, 2 for probe response */
    UINT_16                     u2WSCIELen[3];

	UINT_8                      ucOperatingChnl;
	UINT_8                      ucInvitationType;

	UINT_32                     u4InvStatus;

    /* For SET_STRUCT/GET_STRUCT */
    UINT_8                      aucOidBuf[4096];

#if 1 /* CFG_SUPPORT_ANTI_PIRACY */
    UINT_8                      aucSecCheck[256];
    UINT_8                      aucSecCheckRsp[256];
#endif

};


BOOLEAN
p2pRegisterToWlan(
    P_GLUE_INFO_T prGlueInfo
    );

BOOLEAN
p2pUnregisterToWlan(
    P_GLUE_INFO_T prGlueInfo
    );

BOOLEAN
p2pLaunch(
    P_GLUE_INFO_T prGlueInfo
    );

BOOLEAN
p2pRemove(
    P_GLUE_INFO_T prGlueInfo
    );

BOOLEAN
glRegisterP2P(
    P_GLUE_INFO_T prGlueInfo,
    const char *prDevName,
    BOOLEAN fgIsApMode
    );

BOOLEAN
glUnregisterP2P(
    P_GLUE_INFO_T prGlueInfo
    );

BOOLEAN
p2pStopImmediate(
    P_GLUE_INFO_T prGlueInfo
    );



#endif
