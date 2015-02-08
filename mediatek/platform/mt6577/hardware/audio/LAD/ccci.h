
#ifndef __CCCI_H__
#define __CCCI_H__


#define CCCI_DEV_NAME "ccci"
#define CCCI_SYSFS_INFO "info"


/* CCCI API return value */
typedef enum
{
    CCCI_SUCCESS = 0,
    CCCI_FAIL = -1001,
    CCCI_IN_USE = -1002,
    CCCI_NOT_OWNER = -1003,
    CCCI_INVALID_PARAM = -1004,
    CCCI_NO_PHY_CHANNEL = -1005,
    CCCI_IN_INTERRUPT = -1006,
    CCCI_IN_IRQ = -1007
}CCCI_RETURNVAL_T;

typedef enum
{
#define X_DEF_CH
#include "ccci_ch.h"
#undef X_DEF_CH
    CCCI_MAX_CHANNEL,
    CCCI_FORCE_RESET_MODEM_CHANNEL = 20090215,
} CCCI_CHANNEL_T;


/* CCCI mailbox channel structure */
typedef struct
{
    unsigned int magic;   /* 0xFFFFFFFF */
    unsigned int id;
} CCCI_MAILBOX_T;

/* CCCI stream channel structure */
typedef struct
{
    unsigned int addr;
    unsigned int len;
} CCCI_STREAM_T;

/* CCCI channel buffer structure */
typedef struct
{
    unsigned int data[2];
    unsigned int channel;
    unsigned int reserved;
} CCCI_BUFF_T;

/* CCCI callback function prototype */
typedef void (*CCCI_CALLBACK)(CCCI_BUFF_T *buff, void *private_data);

/* CCCI status */
typedef enum
{
    CCCI_IDLE = 0x00,
    CCCI_ACTIVE_READ = 0x01,
    CCCI_ACTIVE_WRITE = 0x02,
    CCCI_ACTIVE_ISR = 0x04
} CCCI_STATE_T;

/* CCCI control structure */


/* initialize a CCCI mailbox buffer */
#define CCCI_INIT_MAILBOX(buff, mailbox_id) \
        do {    \
            ((CCCI_MAILBOX_T *)((buff)->data))->magic = 0xFFFFFFFF; \
            ((CCCI_MAILBOX_T *)((buff)->data))->id = (mailbox_id);  \
            (buff)->channel = CCCI_MAX_CHANNEL;  \
            (buff)->reserved = 0;    \
        } while (0)

/* initialize a CCCI stream buffer */
#define CCCI_INIT_STREAM(buff, stream_addr, stream_len) \
        do {    \
            ((CCCI_STREAM_T *)((buff)->data))->addr = (stream_addr); \
            ((CCCI_STREAM_T *)((buff)->data))->len = (stream_len);  \
            (buff)->channel = CCCI_MAX_CHANNEL;  \
            (buff)->reserved = 0;    \
        } while (0)

/* check the CCCI buffer type */
#define CCCI_IS_MAILBOX(buff)   ((((CCCI_MAILBOX_T *)((buff)->data))->magic == 0xFFFFFFFF)? 1: 0)

/* get the id of the CCCI mailbox buffer */
#define CCCI_MAILBOX_ID(buff)   (((CCCI_MAILBOX_T *)((buff)->data))->id)

/* get the addr of the CCCI stream buffer */
#define CCCI_STREAM_ADDR(buff)   (((CCCI_STREAM_T *)((buff)->data))->addr)

/* get the len of the CCCI stream buffer */
#define CCCI_STREAM_LEN(buff)   (((CCCI_STREAM_T *)((buff)->data))->len)


#define CCCI_IOC_MAGIC 'C'

#define CCCI_IOC_MD_RESET _IO(CCCI_IOC_MAGIC, 0)

#define CCCI_IOC_PCM_BASE_ADDR _IOR(CCCI_IOC_MAGIC, 2, unsigned int)

#define CCCI_IOC_PCM_LEN _IOR(CCCI_IOC_MAGIC, 3, unsigned int)

#endif  /* !__CCCI_H__ */

