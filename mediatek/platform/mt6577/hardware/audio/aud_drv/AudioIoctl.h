

#ifndef __ANDES_IOCTL_H
#define __ANDES_IOCTL_H



#include <linux/ioctl.h>


typedef struct
{
    unsigned int  offset;
    unsigned int  value;
    unsigned int  mask;
}Register_Control;

typedef struct
{
   int bSpeechFlag;
   int bBgsFlag;
   int bRecordFlag;
   int bTtyFlag;
   int bVT;
   int bAudioPlay;
}SPH_Control;

struct _Info_Data      //HP
{
    unsigned int info;
    unsigned int param1;
    unsigned int param2;
};


enum SPEAKER_CHANNEL
{
      Channel_None = 0 ,
      Channel_Right,
      Channel_Left,
      Channel_Stereo
};

enum SOUND_PATH
{
	DEFAULT_PATH =0,
	IN1_PATH,
	IN2_PATH,
	IN3_PATH,
	IN1_IN2_MIX,
};

enum PolicyParameters{
  POLICY_LOAD_VOLUME=         0,
  POLICY_SET_FM_SPEAKER
};

// routing
static const char* keyAddOutputDevice    = "addoutputdevice";
static const char* keyRemoveOutputDevice = "Removeoutputdevice";
static const char* keyAddIntputDevice    = "addinputdevice";
static const char* keyRemoveIntputDevice = "Removeinputdevice";
// mode
static const char* keyPhoneStateRouting  = "phonestate_routing";
// forceuse
static const char* keyAddtForceuseNormal = "AddForceuseNormal";
static const char* keyAddtForceusePhone = "AddForceusePhone";

// detect headset
static const char* keyDetectHeadset = "DetectHeadset";

// samplerate
static const char* keySetSampleRate = "AudioSetSampleRate";
// FmEnable
static const char* keySetFmEnable = "AudioSetFmEnable";
static const char* keySetFmDigitalEnable = "AudioSetFmDigitalEnable";

static const char* keySetMatvDigitalEnable = "AudioSetMatvDigitalEnable";
//
static const char *keyRouting = "routing";

// keySetForceToSpeaker
static const char* keySetForceToSpeaker = "AudioSetForceToSpeaker";
// keySetFmVolume
static const char* keySetFmVolume = "SetFmVolume";
static const char* keySetMatvVolume = "SetMatvVolume";

// keySetFmTxEnable
static const char* keySetFmTxEnable = "SetFmTxEnable";
static const char* strFMRXForceDisableFMTX = "FMRXForceDisableFMTX";
static const char* keySetLineInEnable = "AtvAudioLineInEnable";
static const char* keySetTtyMode = "tty_mode";
// set VT Speech Call String
static const char* keySetVTSpeechCall = "SetVTSpeechCall";
static const char* keyPhoneMode = "PhoneMode";
// Force turon spealer
static const char* keyForceSpeakerOn = "ForceSpeakerOn";

// for User Case Manager
static const char* keyInitVoume = "VolumeInit";
static const char* keySetVoumeIndex = "SetVolumeIndex";
static const char* keySetStreamStart = "SetStreamStart";
static const char* keySetStreamStop = "SetStreamStop";
static const char* keySetRecordStreamStart = "SetRecordStreamStart";
static const char* keySetRecordStreamStop = "SetRecordStreamStop";



//Audio driver I/O control message
#define AUD_DRV_IOC_MAGIC 'C'

#define SET_AUDSYS_REG         _IOWR(AUD_DRV_IOC_MAGIC, 0x00, Register_Control*)
#define GET_AUDSYS_REG         _IOWR(AUD_DRV_IOC_MAGIC, 0x01, Register_Control*)
#define SET_ANAAFE_REG         _IOWR(AUD_DRV_IOC_MAGIC, 0x02, Register_Control*)
#define GET_ANAAFE_REG         _IOWR(AUD_DRV_IOC_MAGIC, 0x03, Register_Control*)

#define GET_AFE_BUFFER_SIZE    _IOWR(AUD_DRV_IOC_MAGIC, 0x11, unsigned int)
#define SET_SPEAKER_VOL        _IOW(AUD_DRV_IOC_MAGIC, 0x12, int)
#define SET_SPEAKER_ON         _IOW(AUD_DRV_IOC_MAGIC, 0x13, int)
#define SET_SPEAKER_OFF        _IOW(AUD_DRV_IOC_MAGIC, 0x14, int)
#define SET_HEADSET_           _IOW(AUD_DRV_IOC_MAGIC, 0x15, int)
#define SET_HEADPHONE_ON       _IOW(AUD_DRV_IOC_MAGIC, 0x16, int)
#define SET_HEADPHONE_OFF      _IOW(AUD_DRV_IOC_MAGIC, 0x17, int)
#define SET_EARPIECE_ON		   _IOW(AUD_DRV_IOC_MAGIC, 0x18, int)
#define SET_EARPIECE_OFF	   _IOW(AUD_DRV_IOC_MAGIC, 0x19, int)


#define OPEN_DL1_STREAM        _IOWR(AUD_DRV_IOC_MAGIC, 0x20, int)
#define CLOSE_DL1_STREAM       _IOWR(AUD_DRV_IOC_MAGIC, 0x21, int)
#define INIT_DL1_STREAM        _IOWR(AUD_DRV_IOC_MAGIC, 0x22, int)
#define START_DL1_STREAM       _IOWR(AUD_DRV_IOC_MAGIC, 0x23, int)
#define STANDBY_DL1_STREAM     _IOWR(AUD_DRV_IOC_MAGIC, 0x24, int)
#define SET_DL1_AFE_BUFFER     _IOWR(AUD_DRV_IOC_MAGIC, 0x25, int)
#define SET_DL1_SLAVE_MODE     _IOWR(AUD_DRV_IOC_MAGIC, 0x26, int)
#define GET_DL1_SLAVE_MODE     _IOWR(AUD_DRV_IOC_MAGIC, 0x27, int)

#define GET_EAMP_PARAMETER	   _IOWR(AUD_DRV_IOC_MAGIC, 0x28, AMP_Control *)
#define SET_EAMP_PARAMETER	   _IOWR(AUD_DRV_IOC_MAGIC, 0x29, AMP_Control *)


#define OPEN_I2S_INPUT_STREAM       _IOWR(AUD_DRV_IOC_MAGIC, 0x30, int)
#define CLOSE_I2S_INPUT_STREAM      _IOWR(AUD_DRV_IOC_MAGIC, 0x31, int)
#define START_I2S_INPUT_STREAM      _IOWR(AUD_DRV_IOC_MAGIC, 0x33, int)
#define STANDBY_I2S_INPUT_STREAM    _IOWR(AUD_DRV_IOC_MAGIC, 0x34, int)
#define SET_I2S_Input_BUFFER        _IOWR(AUD_DRV_IOC_MAGIC, 0x35, int)
#define SET_I2S_Output_BUFFER       _IOWR(AUD_DRV_IOC_MAGIC, 0x36, int)
#define SET_AWB_INPUT_STREAM_STATE  _IOWR(AUD_DRV_IOC_MAGIC, 0x37, int)


#define SET_2IN1_SPEAKER        _IOW(AUD_DRV_IOC_MAGIC, 0x41, int)
#define SET_AUDIO_STATE         _IOWR(AUD_DRV_IOC_MAGIC, 0x42, SPH_Control*)
#define GET_AUDIO_STATE         _IOWR(AUD_DRV_IOC_MAGIC, 0x43, SPH_Control*)

#define AUD_SET_LINE_IN_CLOCK   _IOWR(AUD_DRV_IOC_MAGIC, 0x50, int)
#define AUD_SET_CLOCK           _IOWR(AUD_DRV_IOC_MAGIC, 0x51, int)
#define AUD_SET_26MCLOCK        _IOWR(AUD_DRV_IOC_MAGIC, 0x52, int)
#define AUD_SET_ADC_CLOCK       _IOWR(AUD_DRV_IOC_MAGIC, 0x53, int)
#define AUD_SET_I2S_CLOCK       _IOWR(AUD_DRV_IOC_MAGIC, 0x54, int)
#define AUD_SET_ANA_CLOCK       _IOWR(AUD_DRV_IOC_MAGIC, 0x55, int)
#define AUD_GET_ANA_CLOCK_CNT   _IOWR(AUD_DRV_IOC_MAGIC, 0x56, int)

#define AUDDRV_RESET_BT_FM_GPIO   _IOWR(AUD_DRV_IOC_MAGIC, 0x60, int)
#define AUDDRV_SET_BT_PCM_GPIO    _IOWR(AUD_DRV_IOC_MAGIC, 0x61, int)
#define AUDDRV_SET_FM_I2S_GPIO    _IOWR(AUD_DRV_IOC_MAGIC, 0x62, int)
#define AUDDRV_MT6573_CHIP_VER    _IOWR(AUD_DRV_IOC_MAGIC, 0x63, int)

#define AUDDRV_ENABLE_ATV_I2S_GPIO   _IOWR(AUD_DRV_IOC_MAGIC, 0x65, int)
#define AUDDRV_DISABLE_ATV_I2S_GPIO  _IOWR(AUD_DRV_IOC_MAGIC, 0x66, int)

#define AUDDRV_FM_ANALOG_PATH        _IOWR(AUD_DRV_IOC_MAGIC, 0x67, int)
#define AUD_SET_HDMI_CLOCK           _IOWR(AUD_DRV_IOC_MAGIC, 0x68, int)
#define AUD_SET_HDMI_GPIO            _IOWR(AUD_DRV_IOC_MAGIC, 0x69, int)
#define AUD_SET_HDMI_SR              _IOWR(AUD_DRV_IOC_MAGIC, 0x70, int)
#define AUD_SET_HDMI_MUTE            _IOWR(AUD_DRV_IOC_MAGIC, 0x72, int)

#define YUSU_INFO_FROM_USER       _IOWR(AUD_DRV_IOC_MAGIC, 0x71, struct _Info_Data*)      //by HP

#define AUDDRV_START_DAI_OUTPUT   _IOWR(AUD_DRV_IOC_MAGIC, 0x81, int)
#define AUDDRV_STOP_DAI_OUTPUT    _IOWR(AUD_DRV_IOC_MAGIC, 0x82, int)
#define AUDDRV_HQA_AMP_MODESEL    _IOWR(AUD_DRV_IOC_MAGIC, 0x90, int)
#define AUDDRV_HQA_AMP_AMPEN      _IOWR(AUD_DRV_IOC_MAGIC, 0x91, int)
#define AUDDRV_HQA_AMP_AMPVOL     _IOWR(AUD_DRV_IOC_MAGIC, 0x92, int)
#define AUDDRV_HQA_AMP_RECEIVER   _IOWR(AUD_DRV_IOC_MAGIC, 0x93, int)
#define AUDDRV_HQA_AMP_RECGAIN    _IOWR(AUD_DRV_IOC_MAGIC, 0x94, int)
#define AUDDRV_AMP_OC_CFG         _IOWR(AUD_DRV_IOC_MAGIC, 0x95, int)
#define AUDDRV_AMP_OC_READ        _IOWR(AUD_DRV_IOC_MAGIC, 0x96, int)
#define AUDDRV_MD_RST_RECOVERY    _IOWR(AUD_DRV_IOC_MAGIC, 0x97, int)

#define AUDDRV_LOG_PRINT          _IOWR(AUD_DRV_IOC_MAGIC, 0xFD, int)
#define AUDDRV_ASSERT_IOCTL       _IOW(AUD_DRV_IOC_MAGIC, 0xFE, int)
#define AUDDRV_BEE_IOCTL          _IOW(AUD_DRV_IOC_MAGIC, 0xFF, int)

// below defines the YUSU_INFO_FROM_USER message
#define INFO_U2K_MATV_AUDIO_START   0x1001
#define INFO_U2K_MATV_AUDIO_STOP     0x1002

//<--- for open MATV device
#ifndef MATV_QUERY_I2S_INFO
typedef struct matv_i2s_info
{
    int status;
    int mode;
    int rate;
} matv_i2s_info_t;

#ifndef MATV_IOC_MAGIC
#define MATV_IOC_MAGIC    'a'
#endif

#define MATV_QUERY_I2S_INFO     _IOW(MATV_IOC_MAGIC, 0x08,  struct matv_i2s_info*)

#endif
//--->
#endif


