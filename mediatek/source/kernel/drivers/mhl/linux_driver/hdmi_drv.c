
#if defined(MTK_HDMI_SUPPORT)
#include <linux/kernel.h>

#include <linux/xlog.h>

#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include "mhl_linuxdrv.h"
#include "osal/include/osal.h"
#include "si_mhl_tx_api.h"
#include "si_cra.h"
#include "si_drvisrconfig.h"
#include "si_drv_mhl_tx.h"

#include "hdmi_drv.h"

#include "mach/eint.h"
#include <cust_eint.h>

static size_t hdmi_log_on = true;
#define HDMI_LOG(fmt, arg...) \
	do { \
		if (hdmi_log_on) printk("[hdmi_drv]%s,%d ", __func__, __LINE__); printk(fmt, ##arg); \
	}while (0)

#define HDMI_FUNC()	\
	do { \
		if(hdmi_log_on) printk("[hdmi_drv] %s\n", __func__); \
	}while (0)

// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------

#define FRAME_WIDTH  (480)
#define FRAME_HEIGHT (800)

// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

static struct task_struct *hdmi_event_task = NULL;
static struct task_struct *hdmi_hpd_detect_task = NULL;

wait_queue_head_t hdmi_event_wq;
atomic_t hdmi_event = ATOMIC_INIT(0);
static int hdmi_event_status = HDMI_STATE_NO_DEVICE;

static HDMI_UTIL_FUNCS hdmi_util = {0};

#define SET_RESET_PIN(v)    (hdmi_util.set_reset_pin((v)))

#define UDELAY(n) (hdmi_util.udelay(n))
#define MDELAY(n) (hdmi_util.mdelay(n))



static void hdmi_drv_set_util_funcs(const HDMI_UTIL_FUNCS *util)
{
	memcpy(&hdmi_util, util, sizeof(HDMI_UTIL_FUNCS));
}

static void hdmi_drv_get_params(HDMI_PARAMS *params)
{
	memset(params, 0, sizeof(HDMI_PARAMS));
        HDMI_FUNC();
	params->init_config.vformat 		= HDMI_VIDEO_1280x720p_60Hz;
	params->init_config.aformat 		= HDMI_AUDIO_PCM_16bit_44100;

	params->clk_pol           = HDMI_POLARITY_FALLING;
	params->de_pol            = HDMI_POLARITY_RISING;
	params->vsync_pol         = HDMI_POLARITY_RISING;
	params->hsync_pol         = HDMI_POLARITY_RISING;

	params->hsync_front_porch = 110;
	params->hsync_pulse_width = 40;
	params->hsync_back_porch  = 220;

	params->vsync_front_porch = 5;
	params->vsync_pulse_width = 5;
	params->vsync_back_porch  = 20;

	params->rgb_order         = HDMI_COLOR_ORDER_RGB;

	params->io_driving_current = IO_DRIVING_CURRENT_2MA;
	params->intermediat_buffer_num = 4;
}

void hdmi_drv_suspend(void)
{
}

void hdmi_drv_resume(void)
{
}


static int hdmi_drv_video_config(HDMI_VIDEO_RESOLUTION vformat, HDMI_VIDEO_INPUT_FORMAT vin, HDMI_VIDEO_OUTPUT_FORMAT vout)
{
#if 1
	if(vformat == HDMI_VIDEO_720x480p_60Hz)
	{
		HDMI_LOG("[hdmi_drv]480p\n");
		siHdmiTx_VideoSel(HDMI_480P60_4X3);
	}
	else if(vformat == HDMI_VIDEO_1280x720p_60Hz)
	{
		HDMI_LOG("[hdmi_drv]720p\n");
		siHdmiTx_VideoSel(HDMI_720P60);
	}
	else
	{
		HDMI_LOG("%s, video format not support now\n", __func__);
	}
#endif
}

static int hdmi_drv_audio_config(HDMI_AUDIO_FORMAT aformat)
{
}

static int hdmi_drv_video_enable(bool enable)
{
}

static int hdmi_drv_audio_enable(bool enable)
{	
}
#define SiI8338DRIVER_INTERRUPT_MODE   1

static int hdmi_drv_init(void)
	{
		halReturn_t 	halStatus;
		SiiOsStatus_t osalStatus;
			printk("Starting %s\n", MHL_PART_NAME);
	
			if(!SiiCraInitialize())
			{
				SII_DEBUG_PRINT(SII_OSAL_DEBUG_TRACE,"Initialization of CRA layer failed!\n");
				return -EIO;
			}
			osalStatus = SiiOsInit(0);
			if (osalStatus != SII_OS_STATUS_SUCCESS)
			{
				SII_DEBUG_PRINT(SII_OSAL_DEBUG_TRACE,"Initialization of OSAL failed, error code: %d\n",osalStatus);
				return -EIO;
			}
			halStatus = HalInit();
			if (halStatus != HAL_RET_SUCCESS)
			{
				SII_DEBUG_PRINT(SII_OSAL_DEBUG_TRACE,"Initialization of HAL failed, error code: %d\n",halStatus);
				SiiOsTerm();
				return -EIO;
			}
#if MTK_project
	
			//xuecheng
			I2c_Init();
#endif
		
			halStatus = HalOpenI2cDevice(MHL_PART_NAME, MHL_DRIVER_NAME);
			if (halStatus != HAL_RET_SUCCESS)
			{
				SII_DEBUG_PRINT(SII_OSAL_DEBUG_TRACE,"Opening of I2c device %s failed, error code: %d\n",
						MHL_PART_NAME, halStatus);
				HalTerm();
				SiiOsTerm();
				return -EIO;
			}
			
	
		//msleep(200);
		#ifdef SiI8338DRIVER_INTERRUPT_MODE
				halStatus = HalInstallIrqHandler(SiiMhlTxDeviceIsr);
				if (halStatus != HAL_RET_SUCCESS)
				{
					SII_DEBUG_PRINT(SII_OSAL_DEBUG_TRACE,"Initialization of HAL interrupt support failed, error code: %d\n",
							halStatus);
					HalCloseI2cDevice();
					HalTerm();
					SiiOsTerm();
					return -EIO;
				}
		
#else
				StartEventThread(); 	/* begin monitoring for events if using polling mode*/
#endif
			return 0;
	}


static int hdmi_drv_exit(void)
{
}
extern void SiiMhlTxInitialize(unsigned char pollIntervalMs );
extern void SiiMhlTxHwReset(uint16_t hwResetPeriod,uint16_t hwResetDelay);
extern unsigned int pmic_config_interface (unsigned char RegNum, unsigned char val, unsigned char MASK, unsigned char SHIFT);
extern void SwitchToD3( void );
extern void SiiMhlTxHwResetKeepLow(void);
extern void ForceSwitchToD3( void );
extern void SiiMhlTxHwGpioSuspend(void);
extern void SiiMhlTxHwGpioResume(void);

void hdmi_drv_power_on(void)
{
	HDMI_FUNC();
	mt65xx_eint_mask(CUST_EINT_HDMI_HPD_NUM);
	SiiMhlTxHwGpioResume();
	pmic_config_interface(0x87,0x1,0x01,0x0);
	//msleep(100);
	SiiMhlTxHwReset(50,200);	

	//xuecheng
	HalInstallCheckDeviceCB(SiiCheckDevice);
	HalAcquireIsrLock();
	siHdmiTx_VideoSel(HDMI_720P60);
	siHdmiTx_AudioSel(I2S_44);
	SiiMhlTxInitialize(EVENT_POLL_INTERVAL_MS);
	HalReleaseIsrLock();
	mt65xx_eint_unmask(CUST_EINT_HDMI_HPD_NUM);
}

void hdmi_drv_power_off(void)
{
	HDMI_FUNC();
	SiiMhlTxHwGpioSuspend();
	SiiMhlTxHwReset(50,200);
	ForceSwitchToD3();
	//SwitchToD3();
	//SiiMhlTxHwResetKeepLow();
	//pmic_config_interface(0x87,0x0,0x01,0x0);
}

extern unsigned char SiiTxReadConnectionStatus(void);
HDMI_STATE hdmi_drv_get_state(void)
{
	HDMI_FUNC();
	int ret = SiiTxReadConnectionStatus();
	HDMI_LOG("sii status:%d\n", ret);
	if(ret == 1)
		return HDMI_STATE_ACTIVE;
	else
		return HDMI_STATE_NO_DEVICE;
}

const HDMI_DRIVER* HDMI_GetDriver(void)
{
        HDMI_FUNC();
	static const HDMI_DRIVER HDMI_DRV =
	{
		.set_util_funcs = hdmi_drv_set_util_funcs,
		.get_params     = hdmi_drv_get_params,
		.init           = hdmi_drv_init,
		.exit         = hdmi_drv_exit,
		.suspend        = hdmi_drv_suspend,
		.resume         = hdmi_drv_resume,
		.video_config	= hdmi_drv_video_config,
		.audio_config	= hdmi_drv_audio_config,
		.video_enable	= hdmi_drv_video_enable,
		.audio_enable	= hdmi_drv_audio_enable,
		.power_on		= hdmi_drv_power_on,
		.power_off		= hdmi_drv_power_off,
		.get_state     = hdmi_drv_get_state
	};

	return &HDMI_DRV;
}
#endif
