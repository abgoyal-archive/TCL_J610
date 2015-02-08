




#ifndef _CONFIG_H
#define _CONFIG_H


//2 Flags for OS capability

#ifdef LINUX
    #ifdef CONFIG_X86
        #define MTK_WCN_HIF_SDIO        0
    #else
        #define MTK_WCN_HIF_SDIO        1
    #endif
#else
    #define MTK_WCN_HIF_SDIO            0
#endif

#if (CFG_SUPPORT_AEE == 1)
    #define CFG_ENABLE_AEE_MSG          1
#else
    #define CFG_ENABLE_AEE_MSG          0
#endif

//2 Flags for Driver Features
#define CFG_TX_FRAGMENT                         1 /*!< 1: Enable TX fragmentation
                                                       0: Disable */
#define CFG_SUPPORT_PERFORMANCE_TEST          0  /*Only for performance Test*/

#define CFG_COUNTRY_CODE                        NULL //"US"

#ifndef LINUX
    #define CFG_FW_FILENAME                         L"WIFI_RAM_CODE"
    #define CFG_FW_FILENAME_E6          L"WIFI_RAM_CODE_E6"
#else
    #define CFG_FW_FILENAME                         "WIFI_RAM_CODE"
#endif

#define CFG_SUPPORT_802_11D                     1 /*!< 1(default): Enable 802.11d
                                                     0: Disable */

#define CFG_SUPPORT_SPEC_MGMT       0   /* Spectrum Management (802.11h): TPC and DFS */
#define CFG_SUPPORT_RRM             0   /* Radio Reasource Measurement (802.11k) */
#define CFG_SUPPORT_QUIET           0   /* Quiet (802.11h) */


#define CFG_SUPPORT_RX_RDG          0   /* 11n feature. RX RDG capability */
#define CFG_SUPPORT_MFB             0   /* 802.11n MCS Feedback responder */
#define CFG_SUPPORT_RX_STBC         1   /* 802.11n RX STBC (1SS) */
#define CFG_SUPPORT_RX_SGI          1   /* 802.11n RX short GI for both 20M and 40M BW */
#define CFG_SUPPORT_RX_HT_GF        1   /* 802.11n RX HT green-field capability */

#define CFG_SLT_SUPPORT                         0


#ifdef NDIS60_MINIPORT
    #define CFG_NATIVE_802_11                       1

    #define CFG_TX_MAX_PKT_SIZE                     2304
    #define CFG_TCP_IP_CHKSUM_OFFLOAD_NDIS_60       0 /* !< 1: Enable TCP/IP header checksum offload
                                                            0: Disable */
    #define CFG_TCP_IP_CHKSUM_OFFLOAD               0
    #define CFG_WHQL_DOT11_STATISTICS               1
    #define CFG_WHQL_ADD_REMOVE_KEY                 1
    #define CFG_WHQL_CUSTOM_IE                      1
    #define CFG_WHQL_SAFE_MODE_ENABLED              1

#else
    #define CFG_TCP_IP_CHKSUM_OFFLOAD               1 /* !< 1: Enable TCP/IP header checksum offload
                                                            0: Disable */
    #define CFG_TCP_IP_CHKSUM_OFFLOAD_NDIS_60       0
    #define CFG_TX_MAX_PKT_SIZE                     1600
    #define CFG_NATIVE_802_11                       0
#endif


//2 Flags for Driver Parameters
#define CFG_EHPI_FASTER_BUS_TIMING              0 /*!< 1: Do workaround for faster bus timing
                                                     0(default): Disable */

#ifdef _lint
    #define _HIF_SDIO   1
#endif

#define CFG_SDIO_INTR_ENHANCE                        1 /*!< 1(default): Enable SDIO ISR & TX/RX status enhance mode
                                                           0: Disable */
#define CFG_SDIO_RX_ENHANCE                          0 /*!< 1(default): Enable SDIO ISR & TX/RX status enhance mode
                                                           0: Disable */
#define CFG_SDIO_TX_AGG                              1 /*!< 1: Enable SDIO TX enhance mode(Multiple frames in single BLOCK CMD)
                                                           0(default): Disable */

#define CFG_SDIO_RX_AGG                              1 /*!< 1: Enable SDIO RX enhance mode(Multiple frames in single BLOCK CMD)
                                                               0(default): Disable */

#if (CFG_SDIO_RX_AGG == 1) && (CFG_SDIO_INTR_ENHANCE == 0)
    #error "CFG_SDIO_INTR_ENHANCE should be 1 once CFG_SDIO_RX_AGG equals to 1"
#elif (CFG_SDIO_INTR_ENHANCE == 1 || CFG_SDIO_RX_ENHANCE == 1) && (CFG_SDIO_RX_AGG == 0)
    #error "CFG_SDIO_RX_AGG should be 1 once CFG_SDIO_INTR_ENHANCE and/or CFG_SDIO_RX_ENHANCE equals to 1"
#endif

#define CFG_SDIO_MAX_RX_AGG_NUM                     0 /*!< 1: Setting the maximum RX aggregation number
                                                               0(default): no limited */

#ifdef WINDOWS_CE
    #define CFG_SDIO_PATHRU_MODE                    1 /*!< 1: Suport pass through (PATHRU) mode
                                                         0: Disable */
#else
    #define CFG_SDIO_PATHRU_MODE                    0 /*!< 0: Always disable if WINDOWS_CE is not defined */
#endif

#define CFG_MAX_RX_ENHANCE_LOOP_COUNT               3


#if defined(MT6620)
    #define MT6620_FPGA_BWCS    0
    #define MT6620_FPGA_V5      0

    #if (MT6620_FPGA_BWCS == 1) && (MT6620_FPGA_V5 == 1)
        #error
    #endif

    #if defined(MTK_WCN_HIF_SDIO)
        #define CFG_MULTI_ECOVER_SUPPORT    1
    #elif !defined(LINUX)
        #define CFG_MULTI_ECOVER_SUPPORT    1
    #else
        #define CFG_MULTI_ECOVER_SUPPORT    0
    #endif

    #define CFG_ENABLE_CAL_LOG      0
    #define CFG_REPORT_RFBB_VERSION       0
#elif defined(MT5931)

#define CFG_MULTI_ECOVER_SUPPORT    0
#define CFG_ENABLE_CAL_LOG      0
#define CFG_REPORT_RFBB_VERSION       0

#elif defined(MT6628)

#define CFG_MULTI_ECOVER_SUPPORT    0

#define CFG_ENABLE_CAL_LOG      1
#define CFG_REPORT_RFBB_VERSION       1

#endif

#define CFG_CHIP_RESET_SUPPORT                      1


#if defined(MT6620) && (MT6620_FPGA_BWCS == 0) && (MT6620_FPGA_V5 == 0)
    #define MT6620_E1_ASIC_HIFSYS_WORKAROUND            0
#else
    #define MT6620_E1_ASIC_HIFSYS_WORKAROUND            0
#endif

#define CFG_DRV_OWN_VERSION                     ((UINT_16)((NIC_DRIVER_MAJOR_VERSION << 8) | (NIC_DRIVER_MINOR_VERSION)))
#define CFG_DRV_PEER_VERSION                    ((UINT_16)0x0000)


#if defined(LINUX)
    #define CFG_TX_BUFFER_IS_SCATTER_LIST       1 /*!< 1: Do frame copy before write to TX FIFO.
                                                        Used when Network buffer is scatter-gather.
                                                     0(default): Do not copy frame */
#else /* WINDOWS/WINCE */
    #define CFG_TX_BUFFER_IS_SCATTER_LIST       1
#endif /* LINUX */


#if CFG_SDIO_TX_AGG || CFG_TX_BUFFER_IS_SCATTER_LIST
    #define CFG_COALESCING_BUFFER_SIZE          (CFG_TX_MAX_PKT_SIZE * NIC_TX_BUFF_SUM)
#else
    #define CFG_COALESCING_BUFFER_SIZE          (CFG_TX_MAX_PKT_SIZE)
#endif /* CFG_SDIO_TX_AGG || CFG_TX_BUFFER_IS_SCATTER_LIST */


/*! Maximum number of SW TX packet queue */
#define CFG_TX_MAX_PKT_NUM                      256

/*! Maximum number of SW TX CMD packet buffer */
#define CFG_TX_MAX_CMD_PKT_NUM                  32

/*! Maximum number of associated STAs */
#define CFG_NUM_OF_STA_RECORD                   20


/*! Max. descriptor number - sync. with firmware */
#if CFG_SLT_SUPPORT
#define CFG_NUM_OF_RX0_HIF_DESC                 42
#else
#define CFG_NUM_OF_RX0_HIF_DESC                 16
#endif
#define CFG_NUM_OF_RX1_HIF_DESC                 2

/*! Max. buffer hold by QM */
#define CFG_NUM_OF_QM_RX_PKT_NUM                120

/*! Maximum number of SW RX packet buffer */
#define CFG_RX_MAX_PKT_NUM                      ((CFG_NUM_OF_RX0_HIF_DESC + CFG_NUM_OF_RX1_HIF_DESC) * 3 \
                                                + CFG_NUM_OF_QM_RX_PKT_NUM)

#define CFG_RX_REORDER_Q_THRESHOLD              8

#ifndef LINUX
#define CFG_RX_RETAINED_PKT_THRESHOLD           (CFG_NUM_OF_RX0_HIF_DESC + CFG_NUM_OF_RX1_HIF_DESC + CFG_NUM_OF_QM_RX_PKT_NUM)
#else
#define CFG_RX_RETAINED_PKT_THRESHOLD           0
#endif

/*! Maximum RX packet size, if exceed this value, drop incoming packet */
/* 7.2.3 Maganement frames */
#define CFG_RX_MAX_PKT_SIZE   ( 28 + 2312 + 12 /*HIF_RX_HEADER_T*/ )  //TODO: it should be 4096 under emulation mode

/*! Minimum RX packet size, if lower than this value, drop incoming packet */
#define CFG_RX_MIN_PKT_SIZE                     10 /*!< 802.11 Control Frame is 10 bytes */

#if CFG_SDIO_RX_AGG
    /* extra size for CS_STATUS and enhanced response */
    #define CFG_RX_COALESCING_BUFFER_SIZE       ((CFG_NUM_OF_RX0_HIF_DESC  + 1) \
                                                * CFG_RX_MAX_PKT_SIZE)
#else
    #define CFG_RX_COALESCING_BUFFER_SIZE       (CFG_RX_MAX_PKT_SIZE)
#endif

/*! RX BA capability */
#define CFG_NUM_OF_RX_BA_AGREEMENTS             8
#define CFG_RX_BA_MAX_WINSIZE                   16
#define CFG_RX_BA_INC_SIZE                      4
#define CFG_RX_MAX_BA_TID_NUM                   8
#define CFG_RX_REORDERING_ENABLED               1

#define CFG_RESPONSE_POLLING_TIMEOUT            512


/*! Maximum number of BSS in the SCAN list */
#define CFG_MAX_NUM_BSS_LIST                    64

#define CFG_MAX_COMMON_IE_BUF_LEN         (1500 * CFG_MAX_NUM_BSS_LIST) / 3

/*! Maximum size of IE buffer of each SCAN record */
#define CFG_IE_BUFFER_SIZE                      512

/*! Maximum number of STA records */
#define CFG_MAX_NUM_STA_RECORD                  32



#define CFG_ENABLE_FULL_PM                      1
#define CFG_ENABLE_WAKEUP_ON_LAN                0

#define CFG_INIT_POWER_SAVE_PROF                    ENUM_PSP_FAST_SWITCH

#define CFG_INIT_ENABLE_PATTERN_FILTER_ARP                    0

#define CFG_INIT_UAPSD_AC_BMP                    0//(BIT(3) | BIT(2) | BIT(1) | BIT(0))

//#define CFG_SUPPORT_WAPI                        0
#define CFG_SUPPORT_WPS                          1
#define CFG_SUPPORT_WPS2                         1

#define CFG_MAX_PMKID_CACHE                     16      /*!< max number of PMKID cache
                                                           16(default) : The Max PMKID cache */

#define CFG_INIT_ADHOC_FREQ                     (2462000)
#define CFG_INIT_ADHOC_MODE                     AD_HOC_MODE_MIXED_11BG
#define CFG_INIT_ADHOC_BEACON_INTERVAL          (100)
#define CFG_INIT_ADHOC_ATIM_WINDOW              (0)




#if defined(_HIF_SDIO) && defined(WINDOWS_CE)
    #define CFG_IST_LOOP_COUNT                  1
#else
    #define CFG_IST_LOOP_COUNT                  1
#endif /* _HIF_SDIO */

#define CFG_INT_WRITE_CLEAR                     0

#if defined(LINUX)
#define CFG_DBG_GPIO_PINS                       0 /* if 1, use MT6516 GPIO pin to log TX behavior */
#endif

//2 Flags for Driver Debug Options
#define CFG_DBG_MGT_BUF                         1 /*!< 1: Debug statistics usage of MGMT Buffer
                                                       0: Disable */

#define CFG_HIF_STATISTICS                      0

#define CFG_HIF_RX_STARVATION_WARNING           0

#define CFG_STARTUP_DEBUG                       0

#define CFG_RX_PKTS_DUMP                        1

#define CFG_ENABLE_FW_DOWNLOAD                  1

#define CFG_ENABLE_FW_DOWNLOAD_ACK              1
#define CFG_ENABLE_FW_ENCRYPTION                1

#if defined(MT6620) || defined(MT6628)
    #define CFG_ENABLE_FW_DOWNLOAD_AGGREGATION  0
    #define CFG_ENABLE_FW_DIVIDED_DOWNLOAD      1
#else
    #define CFG_ENABLE_FW_DOWNLOAD_AGGREGATION  0
    #define CFG_ENABLE_FW_DIVIDED_DOWNLOAD      0
#endif



#if defined(MT6620)
    #if MT6620_FPGA_BWCS
        #define CFG_FW_LOAD_ADDRESS                     0x10014000
        #define CFG_OVERRIDE_FW_START_ADDRESS           0
        #define CFG_FW_START_ADDRESS                    0x10014001
    #elif MT6620_FPGA_V5
        #define CFG_FW_LOAD_ADDRESS                     0x10008000
        #define CFG_OVERRIDE_FW_START_ADDRESS           0
        #define CFG_FW_START_ADDRESS                    0x10008001
    #else
        #define CFG_FW_LOAD_ADDRESS                     0x10008000
        #define CFG_OVERRIDE_FW_START_ADDRESS           0
        #define CFG_FW_START_ADDRESS                    0x10008001
    #endif
#elif defined(MT5931)
    #define CFG_FW_LOAD_ADDRESS                     0xFF900000
    #define CFG_FW_START_ADDRESS                    0x00000000
#elif defined(MT6628)
    #define CFG_FW_LOAD_ADDRESS                     0x00060000
    #define CFG_OVERRIDE_FW_START_ADDRESS           0
    #define CFG_FW_START_ADDRESS                    0x00060000
#endif



#ifdef LINUX
    #ifdef CONFIG_X86
        #define CFG_ENABLE_BT_OVER_WIFI         0
    #else
    #define CFG_ENABLE_BT_OVER_WIFI             1
    #endif
#else
    #define CFG_ENABLE_BT_OVER_WIFI             0
#endif

#define CFG_BOW_SEPARATE_DATA_PATH              1

#define CFG_BOW_PHYSICAL_LINK_NUM               4

#define CFG_BOW_TEST                            0

#define CFG_BOW_LIMIT_AIS_CHNL                  1

#define CFG_BOW_SUPPORT_11N                     0

#define CFG_BOW_RATE_LIMITATION                 1

#ifdef LINUX
    #ifdef CONFIG_X86
        #define CFG_ENABLE_WIFI_DIRECT          0
        #define CFG_SUPPORT_802_11W             0
    #else
        #define CFG_ENABLE_WIFI_DIRECT          1
        #define CFG_SUPPORT_802_11W             0 /*!< 0(default): Disable 802.11W */
    #endif
#else
    #define CFG_ENABLE_WIFI_DIRECT              0
    #define CFG_SUPPORT_802_11W                 0 /* Not support at WinXP */
#endif

#define CFG_SUPPORT_PERSISTENT_GROUP     0

#define CFG_TEST_WIFI_DIRECT_GO                 0

#define CFG_TEST_ANDROID_DIRECT_GO              0

#define CFG_UNITEST_P2P                         0

#define CFG_ENABLE_WIFI_DIRECT_CFG_80211        1

#define CFG_SUPPORT_EXT_CONFIG                  0

#if CFG_SUPPORT_PERFORMANCE_TEST
#define CFG_ENABLE_STATISTICS_BUFFERING         1
#else
#define CFG_ENABLE_STATISTICS_BUFFERING         0
#endif
#define CFG_STATISTICS_VALID_CYCLE              2000
#define CFG_LINK_QUALITY_VALID_PERIOD           5000

#define CFG_SUPPORT_ADHOC                       1
#define CFG_SUPPORT_AAA                         1


#if (defined(MT5931) && defined(LINUX))
#define CFG_SUPPORT_BCM                         1
#define CFG_SUPPORT_BCM_BWCS                    1
#define CFG_SUPPORT_BCM_BWCS_DEBUG              1
#else
#define CFG_SUPPORT_BCM                         0
#define CFG_SUPPORT_BCM_BWCS                    0
#define CFG_SUPPORT_BCM_BWCS_DEBUG              0
#endif

#define CFG_SUPPORT_RDD_TEST_MODE       0

#define CFG_SUPPORT_PWR_MGT                     1

#define CFG_RSN_MIGRATION                       1

#define CFG_PRIVACY_MIGRATION                   1

#define CFG_ENABLE_HOTSPOT_PRIVACY_CHECK        1

#define CFG_MGMT_FRAME_HANDLING                 1

#define CFG_MGMT_HW_ACCESS_REPLACEMENT          0

#if CFG_SUPPORT_PERFORMANCE_TEST

#else

#endif

#define CFG_SUPPORT_AIS_5GHZ                    1
#define CFG_SUPPORT_BEACON_CHANGE_DETECTION     0

#define CFG_SUPPORT_NVRAM                       1
#define CFG_NVRAM_EXISTENCE_CHECK               1
#define CFG_SW_NVRAM_VERSION_CHECK              1
#define CFG_SUPPORT_NIC_CAPABILITY              1


#define CFG_STRESS_TEST_SUPPORT                 0

#define LINT_SAVE_AND_DISABLE                   /*lint -save -e* */

#define LINT_RESTORE                            /*lint -restore */

#define LINT_EXT_HEADER_BEGIN                   LINT_SAVE_AND_DISABLE

#define LINT_EXT_HEADER_END                     LINT_RESTORE


#define CFG_SUPPORT_QOS             1   /* Enable/disable QoS TX, AMPDU */
#define CFG_SUPPORT_AMPDU_TX        1
#define CFG_SUPPORT_AMPDU_RX        1
#define CFG_SUPPORT_TSPEC           0   /* Enable/disable TS-related Action frames handling */
#define CFG_SUPPORT_UAPSD           1
#define CFG_SUPPORT_UL_PSMP         0

#define CFG_SUPPORT_ROAMING         1  /* Roaming System */
#define CFG_SUPPORT_SWCR            1

#define CFG_SUPPORT_ANTI_PIRACY     1

#define CFG_SUPPORT_OSC_SETTING     1

#if defined(MT5931)
#define CFG_SUPPORT_WHOLE_CHIP_RESET    1  /* for e3 chip only */
#endif

#define CFG_SUPPORT_P2P_RSSI_QUERY        0

#define CFG_SUPPORT_802_11V                    0  /* Support 802.11v Wireless Network Management */
#define CFG_SUPPORT_802_11V_TIMING_MEASUREMENT 0
#if (CFG_SUPPORT_802_11V_TIMING_MEASUREMENT == 1) && (CFG_SUPPORT_802_11V == 0)
    #error "CFG_SUPPORT_802_11V should be 1 once CFG_SUPPORT_802_11V_TIMING_MEASUREMENT equals to 1"
#endif





#endif /* _CONFIG_H */


