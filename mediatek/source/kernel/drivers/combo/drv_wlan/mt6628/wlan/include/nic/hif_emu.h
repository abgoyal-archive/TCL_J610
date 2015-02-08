



#ifndef _HIF_EMU_H
#define _HIF_EMU_H




typedef enum
{
    HIF_TC_MBOX_LB = 0x100,
    HIF_TC_SW_INT,
    HIF_TC_COUNT_INCREASE,
    HIF_TC_COUNT_RESET,
    HIF_TC_TX_SINGLE_PACKET,
    HIF_TC_TX_AGG_PACKET,
    HIF_TC_TX_CLEAR_TC_COUNT,
    HIF_TC_TX_CHK_STATE,
    HIF_TC_RX_SINGLE_PACKET = 0x200,
    HIF_TC_RX_PACKET_LEN,
    HIF_TC_RX_PACKET_LEN_OVERFLOW,
    HIF_TC_RX_AGG_PACKET,
    HIF_TC_RX_CHK_STATE,
    HIF_TC_RX_SW_PKT_FORMAT,
    HIF_TC_RX_READ_HALF,
    HIF_TC_MIX_TX_RX_STRESS,
    HIF_TC_INTR_ENHANCE,
    HIF_TC_RX_ENHANCE_MODE,
    HIF_TC_TX_BURST,
    HIF_TC_RX_BURST,
} HIF_TEST_CASE;

#define HIF_TEST_CASE_START     BIT(16)





VOID
emuInit (
    IN P_ADAPTER_T prAdapter
    );

BOOLEAN
emuStart (
    IN P_ADAPTER_T prAdapter
    );


BOOLEAN
emuInitChkCis (
    IN P_ADAPTER_T prAdapter
    );

BOOLEAN
emuMailboxLoopback (
    IN P_ADAPTER_T prAdapter,
    IN BOOLEAN fgReadClearChk
    );

BOOLEAN
emuSoftwareInterruptLoopback (
    IN P_ADAPTER_T prAdapter,
    IN BOOLEAN fgIntrReadClear
    );

BOOLEAN
emuCheckTxCount (
    IN P_ADAPTER_T prAdapter,
    IN BOOLEAN fgReadByIntrEnhanMode
    );

BOOLEAN
emuSendPacket1 (
    IN P_ADAPTER_T prAdapter,
    IN UINT_32     u4PortId,
    IN BOOLEAN     fgUseEnhanceModeRead
    );

BOOLEAN
emuSendPacketAggN (
    IN P_ADAPTER_T prAdapter,
    IN UINT_32     u4PortId,
    IN UINT_32     u4AggNum,
    IN UINT_32     u4LenStart,
    IN UINT_32     u4LenEnd,
    IN BOOLEAN     fgUseIntrEnhanceModeRead
    );

BOOLEAN
emuReadHalfRxPacket (
    IN P_ADAPTER_T prAdapter
    );

BOOLEAN
emuLPown_ownback_stress (
    IN P_ADAPTER_T prAdapter,
    IN UINT_32     u4LoopCount
    );

BOOLEAN
emuLPown_illegal_access (
    IN P_ADAPTER_T prAdapter
    );

BOOLEAN
emuIntrEnhanceChk (
    IN P_ADAPTER_T prAdapter
    );

BOOLEAN
emuResetTxCount (
    IN P_ADAPTER_T prAdapter
    );

BOOLEAN
emuChkTxState (
    IN P_ADAPTER_T prAdapter
    );

BOOLEAN
emuChkRxState (
    IN P_ADAPTER_T prAdapter
    );

BOOLEAN
emuRxPacket1 (
    IN P_ADAPTER_T prAdapter,
    IN UINT_32      u4PortId,
    IN UINT_32      u4RxLen,
    IN BOOLEAN fgEnIntrEnhanceMode,
    IN BOOLEAN fgEnRxEnhanceMode,
    IN BOOLEAN fgMBoxReadClearByRxEnhance
    );

BOOLEAN
emuRxPacketLenChk (
    IN P_ADAPTER_T prAdapter
    );

BOOLEAN
emuRxPacketAggN (
    IN P_ADAPTER_T prAdapter,
    IN UINT_32      u4PortId,
    IN BOOLEAN fgEnRxEnhanceMode,
    IN UINT_32 u4RxLen,
    IN UINT_32 u4AggNum,
    IN UINT_32 u4MaxReadAggNum//0: unlimited
    );

BOOLEAN
emuRxPacketSwHdrFormat (
    IN P_ADAPTER_T prAdapter,
    IN UINT_32      u4PortId,
    IN UINT_32      u4RxLen,
    IN UINT_32      u4Num
    );

BOOLEAN
emuRxPacketLenOverflow (
    IN P_ADAPTER_T prAdapter
    );

BOOLEAN
emuTxPacketBurstInSwHdrFormat (
    IN P_ADAPTER_T prAdapter,
    IN BOOLEAN      fgEnable
    );

BOOLEAN
emuRxPacketBurstInSwHdrFormat (
    IN P_ADAPTER_T prAdapter,
    IN BOOLEAN      fgEnable
    );

BOOLEAN
emuSendPacketAggNSwHdrFormat (
    IN P_ADAPTER_T prAdapter,
    IN UINT_32     u4PortId,
    IN UINT_32     u4LenStart
    );

#define RUN_TEST_CASE(_Fmt) \
    { \
        if (status == FALSE) { \
            break; \
        } \
        if (_Fmt == FALSE) { \
            status = FALSE; \
            break; \
        } \
    }

#endif /* _HIF_EMU_H */



