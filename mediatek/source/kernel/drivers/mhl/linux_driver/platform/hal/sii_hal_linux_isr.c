#define SII_HAL_LINUX_ISR_C
#include "sii_hal.h"
#include "sii_hal_priv.h"
#include "si_drvisrconfig.h"
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/completion.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <mach/irqs.h>
#include "mach/eint.h"
#include "mach/irqs.h"
#include <mach/mt6577_gpio.h>
#include <cust_gpio_usage.h>
#include <cust_eint.h>
#if !defined CUST_EINT_HDMI_HPD_NUM
	#error
#endif

static irqreturn_t HalThreadedIrqHandler(int irq, void *data)
{
	pMhlDeviceContext	pMhlDevContext = (pMhlDeviceContext)data;
	if (HalAcquireIsrLock() == HAL_RET_SUCCESS)
    {
        if(pMhlDevContext->CheckDevice &&!pMhlDevContext->CheckDevice(0))
        {
            SII_DEBUG_PRINT(SII_OSAL_DEBUG_TRACE,"mhl device errror\n");
            HalReleaseIsrLock();
            return IRQ_HANDLED;
        }
        if(pMhlDevContext->irqHandler)
        {
            (pMhlDevContext->irqHandler)();
        }
		HalReleaseIsrLock();
	}
    else
    {
        SII_DEBUG_PRINT(SII_OSAL_DEBUG_TRACE,"------------- irq missing! -------------\n");
    }
	return IRQ_HANDLED;
}


static struct task_struct *mhl_irq_task = NULL;

static wait_queue_head_t mhl_irq_wq;
static atomic_t mhl_irq_event = ATOMIC_INIT(0);

static void mhl8338_irq_handler(void)
{       
	printk("[mhl]mhl interrupt detected!!!\n");
	atomic_set(&mhl_irq_event, 1);
	wake_up_interruptible(&mhl_irq_wq); 
}

static int mhl_irq_kthread(void *data)
{
	struct sched_param param = { .sched_priority = RTPM_PRIO_SCRN_UPDATE };
	sched_setscheduler(current, SCHED_RR, &param);

	for( ;; ) {    	 
		wait_event_interruptible(mhl_irq_wq, atomic_read(&mhl_irq_event));
		printk("mhl_irq_kthread, mhl irq received\n");

		atomic_set(&mhl_irq_event, 0);
		SiiMhlTxDeviceIsr();

		if (kthread_should_stop())
			break;

		mt65xx_eint_unmask(CUST_EINT_HDMI_HPD_NUM);   
	}

	return 0;
}

halReturn_t HalInstallIrqHandler(fwIrqHandler_t irqHandler)
{
	int				retStatus;
	halReturn_t 	halRet;

	init_waitqueue_head(&mhl_irq_wq);
	
	mhl_irq_task = kthread_create(mhl_irq_kthread, NULL, "hdmi_update_kthread"); 
	wake_up_process(mhl_irq_task);

	
	if(irqHandler == NULL)
	{
		SII_DEBUG_PRINT(SII_OSAL_DEBUG_TRACE,"HalInstallIrqHandler: irqHandler cannot be NULL!\n");
		return HAL_RET_PARAMETER_ERROR;
	}
	halRet = I2cAccessCheck();
	if (halRet != HAL_RET_SUCCESS)
	{
		return halRet;
	}

	mt65xx_eint_set_sens(CUST_EINT_HDMI_HPD_NUM, MT65xx_LEVEL_SENSITIVE);
	mt65xx_eint_registration(CUST_EINT_HDMI_HPD_NUM, 0, MT65XX_EINT_POL_NEG, &mhl8338_irq_handler, 0);

	return HAL_RET_SUCCESS;
}
halReturn_t HalRemoveIrqHandler(void)
{
	halReturn_t 	halRet;
	halRet = I2cAccessCheck();
	if (halRet != HAL_RET_SUCCESS)
	{
		return halRet;
	}
	if(gMhlDevice.irqHandler == NULL)
	{
		SII_DEBUG_PRINT(SII_OSAL_DEBUG_TRACE,"HalRemoveIrqHandler: no irqHandler installed!\n");
		return HAL_RET_FAILURE;
	}
	free_irq(gMhlDevice.pI2cClient->irq, &gMhlDevice);
	gMhlDevice.irqHandler = NULL;
	return HAL_RET_SUCCESS;
}
void HalEnableIrq(uint8_t bEnable)
{
	return;
    if(bEnable)
    {
        enable_irq(gMhlDevice.pI2cClient->irq);
    }
    else
    {
        disable_irq(gMhlDevice.pI2cClient->irq);
    }
}

halReturn_t HalInstallCheckDeviceCB(fnCheckDevice fn)
{
    gMhlDevice.CheckDevice = fn;
	return HAL_RET_SUCCESS;
}
