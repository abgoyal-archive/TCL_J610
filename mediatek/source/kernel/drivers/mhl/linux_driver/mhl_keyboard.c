#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/input.h>
#include <linux/workqueue.h>
#include <linux/timer.h>
#include <linux/interrupt.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <linux/earlysuspend.h>
#include <linux/slab.h>
#include <linux/xlog.h>

#include <linux/device.h>
#include <linux/errno.h>
#include <asm/atomic.h>
#include <asm/uaccess.h>
#include "si_mhl_tx_api.h"

static unsigned short mhl_keycode[] = {
	  KEY_SELECT,  
	  KEY_UP,
	  KEY_DOWN,
	  KEY_LEFT,
	  KEY_RIGHT,
	  KEY_MENU, 
	  KEY_BACK, 
	  KEY_0, 
	  KEY_1, 
	  KEY_2, 
	  KEY_3, 
	  KEY_4, 
	  KEY_5, 
	  KEY_6, 
	  KEY_7, 
	  KEY_8,
	  KEY_9, 
	  KEY_DOT, 
	  KEY_ENTER, 
	  KEY_CLEAR, 
	  KEY_SOUND, 
	  KEY_PLAY, 
	  KEY_PAUSE, 
	  KEY_STOP, 
	  KEY_FASTFORWARD, 
	  KEY_REWIND, 
	  KEY_EJECTCD, 
	  KEY_FORWARD, 
	  KEY_PLAYPAUSE,  
	  KEY_PREVIOUSSONG,
	  KEY_NEXTSONG,
	  KEY_F1,
	  KEY_F2,  
	  KEY_F3,  
	  KEY_F4,  
	  KEY_F5,  
};

#define MHL_KBD_NAME  "mhl-keyboard"
#define MHL_KEY_PRESS 1
#define MHL_KEY_RELEASE 0

static struct input_dev *mhl_input_dev = NULL;

struct mhlkeyboard {
    struct input_dev *input;
    unsigned short keymap[ARRAY_SIZE(mhl_keycode)];
};

struct mhlkeyboard *mhlkbd;
static atomic_t dev_open_count;

struct input_dev* get_mhl_input_dev(void)
{
	return mhl_input_dev;
}

static int mhl_kbd_validate_keycode(unsigned char keycode)
{
	int i;
	for(i=0; i<ARRAY_SIZE(mhl_keycode); i++)
	{
		if(mhl_keycode[i] == keycode)
			return keycode;
	}

	return -1;	
}

static long mhl_kbd_dev_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	void __user *uarg = (void __user *)arg;
	short value;
	unsigned char keycode;
	TX_DEBUG_PRINT(("mhl_kbd_dev_ioctl,cmd=%d\n",cmd));			
	switch(cmd)
	{
		case MHL_KEY_PRESS:
		{
			if (copy_from_user(&keycode, uarg, sizeof(keycode)))
				return -EFAULT;
			TX_DEBUG_PRINT(("mhl keycode  %d press\n",keycode));
			if(mhl_kbd_validate_keycode(keycode) < 0)
				return -EINVAL;
			input_report_key(mhl_input_dev,keycode, 1);
			input_sync(mhl_input_dev);
			break;
		}
		case MHL_KEY_RELEASE:
		{
			if (copy_from_user(&keycode, uarg, sizeof(keycode)))
				return -EFAULT;
			TX_DEBUG_PRINT(("mhl keycode %d release\n",keycode));
			if(mhl_kbd_validate_keycode(keycode) < 0)
				return -EINVAL;
			input_report_key(mhl_input_dev,keycode, 0);
			input_sync(mhl_input_dev);
			break;
		}

		default:
			return -EINVAL;
	}
	return 0;
}

static int mhl_kbd_dev_open(struct inode *inode, struct file *file)
{
	int ret = -1;
	TX_DEBUG_PRINT(("*** mhlkeyboard mhl_kbd_dev_open ***\n"));
	atomic_inc(&dev_open_count);	
	ret = nonseekable_open(inode, file);
	
	return ret;
}

static int mhl_kbd_dev_release(struct inode *inode, struct file *file)
{
	TX_DEBUG_PRINT(("*** mhlkeyboard mhl_kbd_dev_release ***\n"));
	atomic_dec(&dev_open_count);
	return 0;
}

static struct file_operations mhl_kbd_dev_fops = {
	.owner		= THIS_MODULE,
	.unlocked_ioctl	= mhl_kbd_dev_ioctl,
	.open		= mhl_kbd_dev_open,
	.release		= mhl_kbd_dev_release
};

static struct miscdevice mhl_kbd_dev = {
	.minor	= MISC_DYNAMIC_MINOR,
	.name	= MHL_KBD_NAME,
	.fops	= &mhl_kbd_dev_fops,
};


static int mhl_keyboard_probe(struct platform_device *pdev)
{
	int i,err;

	TX_DEBUG_PRINT(("*** mhlkeyboard probe ***\n"));

	mhlkbd = kzalloc(sizeof(struct mhlkeyboard), GFP_KERNEL);
	mhl_input_dev = input_allocate_device();
	if (!mhlkbd || !mhl_input_dev) 
		goto fail;

	memcpy(mhlkbd->keymap, mhl_keycode,	sizeof(mhl_keycode));
	mhlkbd->input = mhl_input_dev;
	__set_bit(EV_KEY, mhl_input_dev->evbit);
	platform_set_drvdata(pdev, mhlkbd);

	mhl_input_dev->name = MHL_KBD_NAME;
	mhl_input_dev->keycode = mhlkbd->keymap;
	mhl_input_dev->keycodesize = sizeof(unsigned short);
	mhl_input_dev->keycodemax = ARRAY_SIZE(mhl_keycode);
	mhl_input_dev->id.bustype = BUS_HOST;
	mhl_input_dev->dev.parent = &pdev->dev;

	for (i = 0; i < ARRAY_SIZE(mhlkbd->keymap); i++){
		__set_bit(mhlkbd->keymap[i], mhl_input_dev->keybit);		
	}

	atomic_set(&dev_open_count, 0);

	err = input_register_device(mhl_input_dev);
	if (err) {
		TX_DEBUG_PRINT(("register input device failed (%d)\n", err));
		input_free_device(mhl_input_dev);
		return err;
	}
	mhl_kbd_dev.parent = &pdev->dev;
	err = misc_register(&mhl_kbd_dev);
	if (err) {
		TX_DEBUG_PRINT(("register device failed (%d)\n", err));
		input_unregister_device(mhl_input_dev);
		return err;
	}

	return 0;

fail:
	platform_set_drvdata(pdev, NULL);	
	if(mhl_input_dev)		
		input_free_device(mhl_input_dev);
	if(mhlkbd)		
		kfree(mhlkbd);

	return -EINVAL;
}

static struct platform_driver mhl_keyboard_driver = {
    .probe = mhl_keyboard_probe,
    .driver = {
        .name = MHL_KBD_NAME,
    },
};

static int __devinit mhl_keyboard_init(void)
{
	TX_DEBUG_PRINT(("mhl_keyboard_init OK\n"));
	return platform_driver_register(&mhl_keyboard_driver);
}

static void __exit mhl_keyboard_exit(void)
{
}

module_init(mhl_keyboard_init);
module_exit(mhl_keyboard_exit);

MODULE_DESCRIPTION("mhl keyboard Device");
MODULE_LICENSE("GPL");

