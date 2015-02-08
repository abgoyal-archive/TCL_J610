




#ifndef _GL_RST_H
#define _GL_RST_H


#include "gl_typedef.h"


/* duplicated from wmt_exp.h for better driver isolation */
typedef enum _ENUM_WMTDRV_TYPE_T {
    WMTDRV_TYPE_BT = 0,
    WMTDRV_TYPE_FM = 1,
    WMTDRV_TYPE_GPS = 2,
    WMTDRV_TYPE_WIFI = 3,
    WMTDRV_TYPE_WMT = 4,
    WMTDRV_TYPE_STP = 5,
    WMTDRV_TYPE_SDIO1 = 6,
    WMTDRV_TYPE_SDIO2 = 7,
    WMTDRV_TYPE_LPBK = 8,
    WMTDRV_TYPE_MAX
} ENUM_WMTDRV_TYPE_T, *P_ENUM_WMTDRV_TYPE_T;

typedef enum _ENUM_WMTMSG_TYPE_T {
    WMTMSG_TYPE_POWER_ON = 0,
    WMTMSG_TYPE_POWER_OFF = 1,
    WMTMSG_TYPE_RESET = 2,
    WMTMSG_TYPE_STP_RDY= 3,
    WMTMSG_TYPE_HW_FUNC_ON= 4,
    WMTMSG_TYPE_MAX
} ENUM_WMTMSG_TYPE_T, *P_ENUM_WMTMSG_TYPE_T;

typedef enum _ENUM_WMTRSTMSG_TYPE_T{
    WMTRSTMSG_RESET_START = 0x0,
    WMTRSTMSG_RESET_END = 0x1,
    WMTRSTMSG_RESET_MAX,
    WMTRSTMSG_RESET_INVALID = 0xff
} ENUM_WMTRSTMSG_TYPE_T, *P_ENUM_WMTRSTMSG_TYPE_T;

typedef void (*PF_WMT_CB)(
    ENUM_WMTDRV_TYPE_T, /* Source driver type */
    ENUM_WMTDRV_TYPE_T, /* Destination driver type */
    ENUM_WMTMSG_TYPE_T, /* Message type */
    void *, /* READ-ONLY buffer. Buffer is allocated and freed by WMT_drv. Client
               can't touch this buffer after this function return. */
    unsigned int /* Buffer size in unit of byte */
    );


typedef enum _ENUM_WIFI_NETLINK_GRP_T{
    WIFI_NETLINK_GRP_RESET,
    WIFI_NETLINK_GRP_MAX
} ENUM_WIFI_NETLINK_GRP_T, *P_ENUM_WIFI_NETLINK_GRP_T;







/*----------------------------------------------------------------------------*/
/* Reset Initialization/Uninitialization                                      */
/*----------------------------------------------------------------------------*/
VOID
glResetInit(
    VOID
    );

VOID
glResetUninit(
    VOID
    );

VOID
glSendResetRequest(
    VOID
    );

BOOLEAN
kalIsResetting(
    VOID
    );


#endif /* _GL_RST_H */
