


#ifndef _STP_EXP_H_
#define _STP_EXP_H_

#include "osal_typedef.h"
#include "osal.h"



#define BT_TASK_INDX        (0)
#define FM_TASK_INDX        (1)
#define GPS_TASK_INDX       (2)
#define WIFI_TASK_INDX      (3)
#define WMT_TASK_INDX       (4)
#define STP_TASK_INDX       (5)
#define INFO_TASK_INDX      (6)
#define MTKSTP_MAX_TASK_NUM (7)

#define MTKSTP_BUFFER_SIZE  (16384) //Size of RX Queue









typedef void (*MTK_WCN_STP_EVENT_CB)(void);
typedef INT32 (*MTK_WCN_STP_IF_TX)(const UINT8 *data, const UINT32 size, UINT32 *written_size);
/* export for HIF driver */
typedef void (*MTK_WCN_STP_IF_RX)(const UINT8 *data, INT32 size);

typedef enum {
    STP_UART_IF_TX = 0,
    STP_SDIO_IF_TX,
    STP_MAX_IF_TX
}ENUM_STP_TX_IF_TYPE;





extern INT32 mtk_wcn_stp_receive_data(UINT8 *buffer, UINT32 length, UINT8 type);

extern INT32 mtk_wcn_stp_send_data(const UINT8 *buffer, const UINT32 length, const UINT8 type);

extern MTK_WCN_BOOL mtk_wcn_stp_is_rxqueue_empty(UINT8 type);

extern MTK_WCN_BOOL mtk_wcn_stp_is_ready(void);

extern int mtk_wcn_stp_parser_data(UINT8 *buffer, UINT32 length);

extern void mtk_wcn_stp_set_bluez(MTK_WCN_BOOL sdio_flag);

extern int mtk_wcn_stp_register_tx_event_cb(int type, MTK_WCN_STP_EVENT_CB func);

extern int mtk_wcn_stp_register_event_cb(int type, MTK_WCN_STP_EVENT_CB func);


extern int mtk_wcn_stp_register_if_tx(ENUM_STP_TX_IF_TYPE stp_if, MTK_WCN_STP_IF_TX func);


extern int mtk_wcn_stp_register_if_rx(MTK_WCN_STP_IF_RX func);


#endif /* _WMT_EXP_H_ */













