

#include <stdint.h>
#include <sys/types.h>

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sched.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <sys/prctl.h>

#define LOG_TAG "AudioYusuHardware"
#include <utils/Log.h>
#include <utils/String8.h>

#include "AudioYusuHardware.h"
#include "AudioYusuStreamHandler.h"
#include "AudioYusuVolumeController.h"
#include "AudioYusuLadPlayer.h"
#include "AudioYusuDef.h"
#include "AudioYusuLad.h"
#include "AudioYusuHeadsetMessage.h"
#include "AudioCustParam.h"
#include "AudioYusuStreamOut.h"
#include "AudioYusuStreamIn.h"
#include "AudioYusuI2SStreamIn.h"
#include "AudioYusuApStreamIn.h"
#include "AudioYusuStreamHandler.h"
#include <media/mediarecorder.h>
#include <media/AudioParameter.h>

//for Audio Compensation Filter
#include <AudioCompFltCustParam.h>
#include <HeadphoneCompFltCustParam.h>

// now audio hardware dirver shloud independent with policy
//#include "AudioYusuPolicyManager.h"
#include <hardware_legacy/AudioHardwareBase.h>
#include <hardware_legacy/AudioSystemLegacy.h>
#include <hardware_legacy/AudioPolicyInterface.h>
#include <hardware_legacy/power.h>

#include <binder/IServiceManager.h>
#include <media/IAudioPolicyService.h>
#include <AudioYusuPolicyManager.h>

#include "AudioPcm2way.h"
#include <linux/fm.h>
#include "audio_custom_exp.h"
#include "AudioIoctl.h"
#include "AudioAfe.h"
#include "AudioAnalogAfe.h"
#include "AudioI2S.h"
#include "AudioFtm.h"
#include "hdmitx.h"

#ifdef MTK_AUDIO_GAIN_TABLE_SUPPORT
#include <AudioUcm.h>
#include <AudioUcmInterface.h>
#endif


#define BOOT_INFO_FILE "/sys/class/BOOT/BOOT/boot/boot_mode"

//MT6516 boot type definitions
typedef enum
{
    NORMAL_BOOT = 0,
    META_BOOT = 1,
    UNKNOWN_BOOT
} BOOTMODE;

typedef enum
{
    DEFAULT_HDREC_MODE = 0,
    INDOOR_HDREC_MODE,
    OUTDOOR_HDREC_MODE,
    MAX_HDREC_MODE
} HDRECORD_MODE;

#ifdef ENABLE_LOG_HARDWARE
#define LOG_HARDWARE LOGD
#else
#define LOG_HARDWARE LOGV
#endif


#define HEADPHONE_CONTROL
#define EARPIECE_CONTROL

namespace android
{

//#define FORCE_VM_ENABLE

// ----------------------------------------------------------------------------
//static char const * const kAudioDeviceName = "/dev/eac";
#define LAD_BUFFER_LATENCY (300)
#define BT_SCO_SAMPLERATE (16000)

#ifdef TEMP_SOLUTION_VM
#define VMRECORD_DATASIZE (640 * 90)
#define VMRECORD_READSIZE (2048)
#define VM_RECORD_WAKELOCK_NAME    "VM_RECORD_WAKELOCK"

// for VM record thread
static bool VmThreadExit;
static pthread_cond_t mMdExit_Cond;
static pthread_mutex_t mMdRecordMutex;
#endif


static String8 key1 = String8(keyAddOutputDevice);
static String8 key2 = String8(keyRemoveOutputDevice);
static String8 key3 = String8(keyAddIntputDevice);
static String8 key4 = String8(keyRemoveIntputDevice);
static String8 key5 = String8(keyPhoneStateRouting);

static String8 key7 = String8(keySetSampleRate);

// FM relate command
static String8 key8 = String8(keySetFmEnable);
static String8 key9 = String8(keySetForceToSpeaker);
static String8 key10 = String8(keySetFmVolume);
static String8 key11 = String8(keySetFmTxEnable);
static String8 keyFMRXForceDisableFMTX = String8(strFMRXForceDisableFMTX);
static String8 key14 = String8(keySetFmDigitalEnable);

// MATV relate command
static String8 key13 = String8(keySetLineInEnable);
static String8 key16 = String8(keySetMatvDigitalEnable);
static String8 Key17 = String8(keySetMatvVolume);

static String8 key12 = String8(keySetTtyMode);

static String8 key6 = String8(keyAddtForceuseNormal);
static String8 Key20 = String8(keyAddtForceusePhone);


static String8 key22 = String8(keyRouting);

// VT Speech Call Setting
static String8 key15 = String8(keySetVTSpeechCall);
static String8 key31 = String8(keyPhoneMode);

static String8 key41 = String8(keyForceSpeakerOn);

// detect headset

static String8 keyHeadsetDetect = String8(keyDetectHeadset);

static String8 keySetMatvMute = String8("SetMatvMute");

//User case manager COMMAND
static String8 keyHwInitStreamVoume = String8(keyInitVoume);
static String8 keyHwSetVoumeIndex = String8(keySetVoumeIndex);
static String8 keyHwSetStreamStart = String8(keySetStreamStart);
static String8 keyHwSetStreamStop = String8(keySetStreamStop);
static String8 keyHwSetRecordStreamStart = String8(keySetRecordStreamStart);
static String8 keyHwSetRecordStreamStop = String8(keySetRecordStreamStop);

static String8 keyEnableStereoOutput = String8("EnableStereoOutput");

#if defined(MTK_DUAL_MIC_SUPPORT)||defined(MTK_AUDIO_HD_REC_SUPPORT)
static String8 keyMETA_SET_DUAL_MIC_FLAG  = String8("META_SET_DUAL_MIC_FLAG");
static String8 keyMETA_SET_DUAL_MIC_WB  = String8("META_SET_DUAL_MIC_WB");
static String8 keyMETA_DUAL_MIC_IN_FILE_NAME = String8("META_DUAL_MIC_IN_FILE_NAME");
static String8 keyMETA_DUAL_MIC_OUT_FILE_NAME = String8("META_DUAL_MIC_OUT_FILE_NAME");
static String8 keyMETA_DUAL_MIC_RECORD    = String8("META_DUAL_MIC_RECORD");
static String8 keyMETA_DUAL_MIC_REC_PLAY  = String8("META_DUAL_MIC_REC_PLAY");
static String8 keyMETA_DUAL_MIC_REC_PLAY_HS  = String8("META_DUAL_MIC_REC_PLAY_HS");
static String8 keyEnable_Dual_Mic_Setting = String8("Enable_Dual_Mic_Setting");
static String8 keyMETA_DUAL_MIC_GET_UL_GAIN = String8("META_DUAL_MIC_GET_UL_GAIN");
static String8 keyMETA_DUAL_MIC_SET_UL_GAIN = String8("META_DUAL_MIC_SET_UL_GAIN");
static String8 keyMETA_DUAL_MIC_GET_DL_GAIN = String8("META_DUAL_MIC_GET_DL_GAIN");
static String8 keyMETA_DUAL_MIC_SET_DL_GAIN = String8("META_DUAL_MIC_SET_DL_GAIN");
static String8 keyMETA_DUAL_MIC_GET_HSDL_GAIN = String8("META_DUAL_MIC_GET_HSDL_GAIN");
static String8 keyMETA_DUAL_MIC_SET_HSDL_GAIN = String8("META_DUAL_MIC_SET_HSDL_GAIN");
static String8 keyMETA_DUAL_MIC_GET_WB_SUPPORT = String8("META_DUAL_MIC_GET_WB_SUPPORT");
#define USE_DUALMIC_SDBUF			//enable dual mic support
#endif

#if defined(MTK_SPH_EHN_CTRL_SUPPORT)
#define VCE_CFG_FILENAME "/data/system/vcecfg"
static String8 key_SET_VCE_ENABLE = String8("SET_VCE_ENABLE");
static String8 key_GET_VCE_STATUS = String8("GET_VCE_STATUS");
#endif


static String8 key_GET_HDMI_AUDIO_STATUS = String8("GetHDMIAudioStatus");
static String8 key_SET_HDMI_AUDIO_ENABLE = String8("SetHDMIAudioEnable");

#if defined(MTK_AUDIO_HD_REC_SUPPORT)
//static String8 key_HD_REC_MODE = String8("HDRecordMode");
static String8 keyHDREC_SET_VOICE_MODE = String8("HDREC_SET_VOICE_MODE");
static String8 keyHDREC_SET_VIDEO_MODE = String8("HDREC_SET_VIDEO_MODE");
#endif



/// Loobpacks
static String8 key_SET_LOOPBACK_TYPE  = String8("SET_LOOPBACK_TYPE");

/// RILSetHeadsetState start
static String8 keyRILSetHeadsetState = String8("RILSetHeadsetState");
/// RILSetHeadsetState end
//<---for speech/ACF/HCF parameters calibration
static String8 keySpeechParams_Update = String8("UpdateSpeechParameter");
static String8 keySpeechVolume_Update = String8("UpdateSphVolumeParameter");
static String8 keyACFHCF_Update = String8("UpdateACFHCFParameters");
//--->

enum
{
    Normal_Coef_Index,
    Headset_Coef_Index,
    Handfree_Coef_Index,
    VOIPBT_Coef_Index,
    VOIPNormal_Coef_Index,
    VOIPHandfree_Coef_Index,
    AUX1_Coef_Index,
    AuX2_Coef_Index
};

AudioYusuHardware::AudioYusuHardware()
    : mFd(-1), mMicMute(false)
{
    unsigned int err = -1;
    LOG_HARDWARE("AudioYusuHardware constructor");
    mFd = ::open(kAudioDeviceName, O_RDWR);
    mAfe_handle = NULL;
    mAnaReg = NULL;
    mStreamHandler = NULL;
    mVolumeController = NULL;
    mI2S = NULL;
    mSpeakerStatus = false;
    mBgsStatus = false;
    mVoiceStatus = false;
    mCommunicationStatus = false;
    mFmDigitalInStatus = false;
    mFmDigitalOutStatus = false;
    mFmStatus =false;
    mFmSpeakerStatus =false;
    mMatvAnalogstatus = false;
    mMatvDigitalstatus = false;
    mTty_Ctm = AUD_TTY_OFF;
    mVT_Speech      = false;
    mPcm2Way_Enable = false;
    mFakeMode = false;
    mMode = android_audio_legacy::AudioSystem::MODE_NORMAL;
    mPreviousMode= android_audio_legacy::AudioSystem::MODE_NORMAL;
    mRecordFM = false;

    mEarpieceStatus = false;
    mHeadPhoneStatus = false;
    mAuioDevice = NULL;

    mForceSpeakerStatus = false;
    mPhoneMode = 0;
    mForceBGSoff = false;
    m_AnalogIn_ATV_Enable_during_InCall = false;
    m_Speaker_Enable_during_InCall = false;
    mMicMute_ModemStatus = false;
    m_HDMIAudioStatus = false;
    mEnableStereoOutput = true;
    mFMRXForceDisableFMTX = false;

#if defined(MTK_DUAL_MIC_SUPPORT)||defined(MTK_AUDIO_HD_REC_SUPPORT)
    // create a file to keep dual mic config when first boot
    FILE *dmFp;
    int dmValue = 1;
    dmFp = fopen("/data/system/dualmiccfg","rb");
    if (dmFp == NULL)
    {
        LOG_HARDWARE("/data/system/dualmiccfg doesn't exist, new a file");
        dmFp = fopen("/data/system/dualmiccfg","wb");
        dmValue = 1;
        if(dmFp) chmod("/data/system/dualmiccfg", 0770);
        else LOGE("cannot new file in consturoctor");
        if(dmFp) fwrite(&dmValue, sizeof(int), 1, dmFp);
        if(dmFp) fclose(dmFp);
    }
    else
    {
        LOG_HARDWARE("/data/system/dualmiccfg exists");
        if(dmFp) fread(&dmValue, sizeof(int), 1, dmFp);
        if(dmFp) fclose(dmFp);
    }
    if (dmValue == 1)
    {
        LOG_HARDWARE(" HW constructor property_get dualmic %d", dmValue);
        //mMMI_DualMic_Setting = true;
        mHAL_DualMic_Setting = true;
    }
    else
    {
        LOG_HARDWARE(" HW constructor property_get dualmic %d", dmValue);
        //mMMI_DualMic_Setting = false;
        mHAL_DualMic_Setting = false;
    }
#if defined(MTK_WB_SPEECH_SUPPORT)
    mDualMicMetaWB = FALSE;                    //always false, in case old tool(only NB) to use this load, calibrate NB only
#endif

    mDualMic_Test = false;
    //mDualMicTool_micGain = 35;
    mDualMicTool_micGain = Get_NVRAM_ULGain_For_DualMic(0);
    mDualMicTool_receiverGain = Get_NVRAM_DLGain_For_DualMic(0);
    mDualMicTool_headsetGain = Get_NVRAM_DLGain_For_DualMic(1);
    //mDualMicTool_receiverGain = 3;
    //mDualMicTool_headsetGain = 3;
    {
        int ret = pthread_mutex_init(&dmicMutex, NULL);
        if ( ret != 0 ) LOGE("Failed to initialize pthread dmicMutex!");
#ifdef USE_DUALMIC_SDBUF
        ret = pthread_mutex_init(&mDmsdMutex, NULL);
        if ( ret != 0 ) LOGE("Failed to initialize mDmsdMutex!");
        ret = pthread_mutex_init(&mDmsdBufMutex, NULL);
        if ( ret != 0 ) LOGE("Failed to initialize mDmsdBufMutex!");
        ret = pthread_cond_init(&mDmsdExit_Cond, NULL);
        if ( ret != 0 ) LOGE("Failed to initialize mDmsdExit_Cond!");
#endif
    }

#endif

#if defined(MTK_SPH_EHN_CTRL_SUPPORT)
    mVceIsEnable = true; //default on

    FILE *f_vce = fopen(VCE_CFG_FILENAME, "rb");
    if (f_vce == NULL)
    {
        LOG_HARDWARE("%s doesn't exist, create a file", VCE_CFG_FILENAME);
        f_vce = fopen(VCE_CFG_FILENAME, "wb");

        if(f_vce == NULL)
        {
            LOGE("cannot create file %s in consturoctor", VCE_CFG_FILENAME);
        }
        else
        {
            chmod(VCE_CFG_FILENAME, 0644);
            fwrite(&mVceIsEnable, sizeof(bool), 1, f_vce);
            fclose(f_vce);
        }
    }
    else
    {
        LOG_HARDWARE("%s exists", VCE_CFG_FILENAME);
        fread(&mVceIsEnable, sizeof(bool), 1, f_vce);
        fclose(f_vce);
    }

    LOG_HARDWARE("HW constructor property_get VCE %d", mVceIsEnable);
#endif //MTK_SPH_EHN_CTRL_SUPPORT

    // default setting route
    mDevice = android_audio_legacy::AudioSystem::DEVICE_OUT_SPEAKER ;
    mRoutingDevices = 0;
    mOutputAvailable[android_audio_legacy::AudioSystem::MODE_NORMAL]   = android_audio_legacy::AudioSystem::DEVICE_OUT_SPEAKER;
    mOutputAvailable[android_audio_legacy::AudioSystem::MODE_RINGTONE] = android_audio_legacy::AudioSystem::DEVICE_OUT_SPEAKER;
    mOutputAvailable[android_audio_legacy::AudioSystem::MODE_IN_CALL]	= android_audio_legacy::AudioSystem::DEVICE_OUT_EARPIECE;
    mOutputAvailable[android_audio_legacy::AudioSystem::MODE_IN_COMMUNICATION]	= android_audio_legacy::AudioSystem::DEVICE_OUT_EARPIECE;
    mOuputForceuse[android_audio_legacy::AudioSystem::MODE_NORMAL] = android_audio_legacy::AudioSystem::FORCE_NONE;
    mOuputForceuse[android_audio_legacy::AudioSystem::MODE_RINGTONE] = android_audio_legacy::AudioSystem::FORCE_NONE;
    mOuputForceuse[android_audio_legacy::AudioSystem::MODE_IN_CALL] = android_audio_legacy::AudioSystem::FORCE_NONE;
    mOuputForceuse[android_audio_legacy::AudioSystem::MODE_IN_COMMUNICATION]	= android_audio_legacy::AudioSystem::FORCE_NONE;
    mInputAvailable = android_audio_legacy::AudioSystem::DEVICE_IN_BUILTIN_MIC;
    memset(mOuputForceuseRef,0,sizeof(mOuputForceuseRef));
    SetApMicType (LADIN_Microphone1);

#ifdef  AUD_DL1_USE_SLAVE
    ::ioctl(mFd,SET_DL1_SLAVE_MODE, 1);  //Set Audio DL1 slave mode
#else
    ::ioctl(mFd,SET_DL1_SLAVE_MODE, 0);  //Set Audio DL1 master mode
#endif

    ::ioctl(mFd,INIT_DL1_STREAM, ASM_BUFFER_SIZE);  // init asm buffer size

    mAfe_handle = new AudioAfe(this);
    err = mAfe_handle->Afe_Init(mFd);
    if(err == false)
        LOGE("AudioAfe() FAIL");

    mAnaReg = new AudioAnalog(this);
    err = mAnaReg->AnalogAFE_Init(mFd);
    if(err == false)
        LOGE("AudioAnalog() FAIL");

    mStreamHandler = new AudioStreamHandler(mFd,this,mAfe_handle,mAnaReg);
    if(!mStreamHandler)
        LOGE("mStreamHandler() FAIL");

    mVolumeController = new AudioYusuVolumeController(mFd,this,mAfe_handle,mAnaReg);

   #ifdef MTK_AUDIO_GAIN_TABLE_SUPPORT
   mAudioUCM = new android::AudioYusuUserCaseManager(this);
   if(mAudioUCM->initCheck ()){
       LOGE("mAudioUCM initCheck fail");
   }
   #endif
    int fd;
    size_t s;
    int boot_mode = NORMAL_BOOT;
    // to see if't meta mode ,
    fd = open(BOOT_INFO_FILE, O_RDWR);
    if (fd < 0)
    {
        LOGE("fail to open: %s ", BOOT_INFO_FILE);
    }
    else
    {
        s = read(fd, (void *)boot_mode, sizeof(boot_mode));
        if (s <= 0)
        {
            LOGD("fail to read %s: ", BOOT_INFO_FILE);
        }
        close(fd);
    }
    LOGD("BOOT MODE=%d\n",boot_mode);

    this->Get_Recovery_State();

    // create LAD and  create LadPlayer
    if(boot_mode ==NORMAL_BOOT)
    {
        pLad = new LAD(this);
        /*init accdet, accdet need audiohardware to init it, so do it as soon
        ** as possible . Hope  this dependency can be  removed next version*/
        if( pLad->mHeadSetMessager->SetHeadInit () == false)
        {
            LOGE("mHeadSetMessager SetHeadInit ");
        }
        if( !pLad->LAD_Initial() )
        {
            YAD_LOGE("Failed to initialize LAD!");
        }
        else
        {
            LOG_HARDWARE("pLad->LAD_Initial() success and new LADPlayer");
            pLadPlayer = new LADPlayer(this);
            DumpSpeechCoefficient ();
        }
    }

    mAudFtm = new AudioFtm(this,mAfe_handle,mAnaReg);
    err = mAudFtm->AudFtm_Init(mFd, pLad);

    // for event handling */
#if defined(PC_EMULATION)
    LadRecNotifyEvent = CreateEvent(NULL,TRUE,FALSE,"LadRecNotifyEvent");
    LadPlayRequestEvent = CreateEvent(NULL,TRUE,FALSE,"LadPlayRequestEvent");
#else
    // event initialize
    int ret = 0;
    ret |= pthread_mutex_init(&lrnMutex, NULL);
    ret |= pthread_mutex_init(&lprMutex, NULL);
    ret |= pthread_cond_init(&LadRecNotifyEvent, NULL);
    ret |= pthread_cond_init(&LadPlayRequestEvent, NULL);
    if ( ret != 0 ) YAD_LOGE("Failed to initialize pthread components for LAD!");
#endif

    ret =0;
    ret |= pthread_mutex_init(&LadMutex, NULL);
    if ( ret != 0 ) LOGE("Failed to initialize pthread LadMutex!");

    ret |= pthread_mutex_init(&LadBufferMutex, NULL);
    if ( ret != 0 ) LOGE("Failed to initialize pthread LadBufferMutex!");

    ret |= pthread_mutex_init(&ParametersMutex, NULL);
    if ( ret != 0 ) LOGE("Failed to initialize pthread ParametersMutex!");

    ret |= pthread_mutex_init(&StreaminMutex, NULL);
    if ( ret != 0 ) LOGE("Failed to initialize pthread StreaminMutex!");

    ret |= pthread_mutex_init(&ModeMutex, NULL);
    if ( ret != 0 ) LOGE("Failed to initialize pthread ModeMutex!");

    ret |= pthread_mutex_init(&MasterVolumeMutex, NULL);
    if ( ret != 0 ) LOGE("Failed to initialize pthread MasterVolumeMutex!");

    ret |= pthread_mutex_init(&ttyMutex, NULL);
    if ( ret != 0 ) LOGE("Failed to initialize pthread ttyMutex!");


#ifdef TEMP_SOLUTION_VM
    ret |= pthread_mutex_init(&mMdRecordMutex, NULL);
    if ( ret != 0 ) LOGE("Failed to initialize pthread mMdRecordMutex!");
    ret = pthread_cond_init(&mMdExit_Cond, NULL);
    if ( ret != 0 ) LOGE("Failed to initialize mMdExit_Cond!");

    VmThreadExit = true;
#endif

    this->Recover_State();
    if(!mVolumeController)
    {
        LOGE("mVolumeController() FAIL");
    }
    else
    {
        LOGD("InitVolumeController ");
        mVolumeController->InitVolumeController();
    }
    //pLad->LAD_SwitchMicBias(false);

#if defined(MTK_SPH_EHN_CTRL_SUPPORT)
    //set VCE status
    SPH_ENH_INFO_T eSphEnhInfo = pLad->mSphEnhInfo;

    if (mVceIsEnable == true)
        eSphEnhInfo.spe_usr_subFunc_mask |= SPH_ENH_SUBFUNC_MASK_VCE;
    else
        eSphEnhInfo.spe_usr_subFunc_mask &= ~(SPH_ENH_SUBFUNC_MASK_VCE);

    pLad->LAD_Set_Speech_Enhancement_Info(eSphEnhInfo);
#endif

    // create PCM2WAY module
    pPlay2Way    = new Play2Way(this);
    pRecord2Way  = new Record2Way(this);

#ifdef  ENABLE_EXT_DAC
    mAudioCustomFunction = new AudioCustomization();
    // init EXT_DAC
    mAudioCustomFunction->EXT_DAC_Init ();
#endif

#ifdef ENABLE_STEREO_SPEAKER
    mAfe_handle->Afe_Set_Stereo();
#else
    mAfe_handle->Afe_Set_Mono();
#endif

    mAuioDevice = AudioDeviceManger::createInstance(mFd);
    LOGD("mAuioDevice = AudioDeviceManger::createInstance(mFd);");
    if(mAuioDevice == NULL)
    {
        LOGE("Fail to Create DeviceControl!!!");
    }
    SetModeToAMP(mMode); // need to setmode to amp if mediaserver die?
#ifndef HEADPHONE_CONTROL
    SetHeadPhoneStatus(true);
    EnableHeadPhone(); // now always turn on headphone
#endif
    SetSpeakerStatus(true);  // default set speaker
     mHeadsetDetect = new HeadSetDetect(this,&AudioEarphoneCallback);  //earphone callback
#if defined(MTK_AUDIO_HD_REC_SUPPORT)
    mHdRecScene = -1; //default Normal mode
#endif
}

AudioYusuHardware::~AudioYusuHardware()
{
    LOG_HARDWARE("AudioYusuHardware De-constructor ");
    AudioDeviceManger::releaseInstance(mAuioDevice);
    pLad->LAD_Deinitial();
    if(mHeadsetDetect != 0 )
    {
        mHeadsetDetect->requestExitAndWait();
        mHeadsetDetect.clear();
    }
    if (mFd >= 0)
        ::close(mFd);
}

int AudioYusuHardware::Audio_Match_ForceInput_device(uint32_t Force_config)
{
    LOG_HARDWARE("Audio_Match_ForceInput_device Force_config=0x%x", Force_config);
    switch(Force_config)
    {
    case android_audio_legacy::AudioSystem::FORCE_SPEAKER:
    {
        return android_audio_legacy::AudioSystem::DEVICE_IN_BUILTIN_MIC;
    }
    case android_audio_legacy::AudioSystem::FORCE_HEADPHONES:
    {
        return android_audio_legacy::AudioSystem::DEVICE_IN_WIRED_HEADSET;
    }
    case android_audio_legacy::AudioSystem::FORCE_BT_SCO:
    {
        return android_audio_legacy::AudioSystem::DEVICE_IN_BLUETOOTH_SCO_HEADSET;
    }
    case android_audio_legacy::AudioSystem::FORCE_BT_A2DP:
    {
        return android_audio_legacy::AudioSystem::DEVICE_IN_BUILTIN_MIC;
    }
    case android_audio_legacy::AudioSystem::FORCE_WIRED_ACCESSORY:
    {
        return android_audio_legacy::AudioSystem::DEVICE_IN_AUX_DIGITAL;
    }
    case android_audio_legacy::AudioSystem::FORCE_BT_CAR_DOCK:
    case android_audio_legacy::AudioSystem::FORCE_BT_DESK_DOCK:
    {
        return android_audio_legacy::AudioSystem::DEVICE_IN_BLUETOOTH_SCO_HEADSET;
    }
    default:
    {
        LOGE("Audio_Match_Force_device with no config =%d",Force_config);
        return android_audio_legacy::AudioSystem::FORCE_NONE;
    }
    }
    return android_audio_legacy::AudioSystem::FORCE_NONE;
}


int AudioYusuHardware::Audio_Match_Force_device(uint32_t Force_config)
{
    LOG_HARDWARE("Audio_Match_Force_device Force_config=0x%x", Force_config);
    switch(Force_config)
    {
    case android_audio_legacy::AudioSystem::FORCE_SPEAKER:
    {
        return android_audio_legacy::AudioSystem::DEVICE_OUT_SPEAKER;
    }
    case android_audio_legacy::AudioSystem::FORCE_HEADPHONES:
    {
        return android_audio_legacy::AudioSystem::DEVICE_OUT_WIRED_HEADPHONE;
    }
    case android_audio_legacy::AudioSystem::FORCE_BT_SCO:
    {
        return android_audio_legacy::AudioSystem::DEVICE_OUT_BLUETOOTH_SCO;
    }
    case android_audio_legacy::AudioSystem::FORCE_BT_A2DP:
    {
        return android_audio_legacy::AudioSystem::DEVICE_OUT_BLUETOOTH_A2DP;
    }
    case android_audio_legacy::AudioSystem::FORCE_WIRED_ACCESSORY:
    {
        return android_audio_legacy::AudioSystem::DEVICE_OUT_AUX_DIGITAL;
    }
    case android_audio_legacy::AudioSystem::FORCE_BT_CAR_DOCK:
    case android_audio_legacy::AudioSystem::FORCE_BT_DESK_DOCK:
    default:
    {
        LOGE("Audio_Match_Force_device with no config =%d",Force_config);
        return android_audio_legacy::AudioSystem::FORCE_NONE;
    }
    }
    return android_audio_legacy::AudioSystem::FORCE_NONE;
}

int AudioYusuHardware::Audio_Find_Communcation_Output_Device(uint32_t mRoutes)
{
    //LOG_HARDWARE("Audio_Find_Communcation_Output_Device mRoutes=0x%x", mRoutes);
    //can be adjust to control output deivces
    if(mRoutes &(android_audio_legacy::AudioSystem::DEVICE_OUT_WIRED_HEADPHONE) ) // if headphone . still ouput from headsetphone
        return android_audio_legacy::AudioSystem::DEVICE_OUT_WIRED_HEADPHONE;
    else if(mRoutes &	(android_audio_legacy::AudioSystem::DEVICE_OUT_WIRED_HEADSET) )
        return android_audio_legacy::AudioSystem::DEVICE_OUT_WIRED_HEADSET;
    else if(mRoutes &	(android_audio_legacy::AudioSystem::DEVICE_OUT_EARPIECE) )
        return android_audio_legacy::AudioSystem::DEVICE_OUT_EARPIECE;
    else if(mRoutes &	(android_audio_legacy::AudioSystem::DEVICE_OUT_SPEAKER) )
        return android_audio_legacy::AudioSystem::DEVICE_OUT_SPEAKER;
    else if(mRoutes &  (android_audio_legacy::AudioSystem::DEVICE_OUT_BLUETOOTH_SCO_HEADSET) )
        return android_audio_legacy::AudioSystem::DEVICE_OUT_BLUETOOTH_SCO;
    else if(mRoutes &  (android_audio_legacy::AudioSystem::DEVICE_OUT_BLUETOOTH_SCO) )
        return android_audio_legacy::AudioSystem::DEVICE_OUT_BLUETOOTH_SCO;
    else
    {
        LOGE("Audio_Find_Incall_Output_Device with no devices");
        return android_audio_legacy::AudioSystem::DEVICE_OUT_EARPIECE;
    }
}


int AudioYusuHardware::Audio_Find_Incall_Output_Device(uint32_t mRoutes)
{
    LOG_HARDWARE("Audio_Find_Incall_Output_Device mRoutes=0x%x", mRoutes);

    //can be adjust to control output deivces
    if(mRoutes &(android_audio_legacy::AudioSystem::DEVICE_OUT_WIRED_HEADPHONE) ) // if headphone . still ouput from headsetphone
        return android_audio_legacy::AudioSystem::DEVICE_OUT_WIRED_HEADPHONE;
    else if(mRoutes &	(android_audio_legacy::AudioSystem::DEVICE_OUT_WIRED_HEADSET) )
        return android_audio_legacy::AudioSystem::DEVICE_OUT_WIRED_HEADSET;
    else if(mRoutes &	(android_audio_legacy::AudioSystem::DEVICE_OUT_EARPIECE) )
        return android_audio_legacy::AudioSystem::DEVICE_OUT_EARPIECE;
    else if(mRoutes &	(android_audio_legacy::AudioSystem::DEVICE_OUT_SPEAKER) )
        return android_audio_legacy::AudioSystem::DEVICE_OUT_SPEAKER;
    else if(mRoutes &  (android_audio_legacy::AudioSystem::DEVICE_OUT_BLUETOOTH_SCO_HEADSET) )
        return android_audio_legacy::AudioSystem::DEVICE_OUT_BLUETOOTH_SCO;
    else if(mRoutes &  (android_audio_legacy::AudioSystem::DEVICE_OUT_BLUETOOTH_SCO) )
        return android_audio_legacy::AudioSystem::DEVICE_OUT_BLUETOOTH_SCO;
    else
    {
        LOGE("Audio_Find_Incall_Output_Device with no devices");
        return android_audio_legacy::AudioSystem::DEVICE_OUT_EARPIECE;
    }
}

int AudioYusuHardware::Audio_Find_Normal_Output_Device(uint32_t mRoutes)
{
    LOG_HARDWARE("Audio_Find_Normal_Output_Device mRoutes=0x%x", mRoutes);

    // don't BT-SCO in normal mode
    /*
    if(mRoutes&(android_audio_legacy::AudioSystem::DEVICE_OUT_BLUETOOTH_SCO_CARKIT) )
      return android_audio_legacy::AudioSystem::DEVICE_OUT_BLUETOOTH_SCO_CARKIT;
    else if(mRoutes &  (android_audio_legacy::AudioSystem::DEVICE_OUT_BLUETOOTH_SCO_HEADSET) )
      return android_audio_legacy::AudioSystem::DEVICE_OUT_BLUETOOTH_SCO;
    else if(mRoutes &  (android_audio_legacy::AudioSystem::DEVICE_OUT_BLUETOOTH_SCO) )
      return android_audio_legacy::AudioSystem::DEVICE_OUT_BLUETOOTH_SCO;
    */

    // Device priority: FM Tx > Headset = Headphone > HDMI audio > Speaker
    if(mRoutes &  (android_audio_legacy::AudioSystem::DEVICE_OUT_AUX_DIGITAL) )
        return android_audio_legacy::AudioSystem::DEVICE_OUT_AUX_DIGITAL;
    else if(mRoutes &  (android_audio_legacy::AudioSystem::DEVICE_OUT_WIRED_HEADPHONE) )
        return android_audio_legacy::AudioSystem::DEVICE_OUT_WIRED_HEADPHONE;
    else if(mRoutes &  (android_audio_legacy::AudioSystem::DEVICE_OUT_WIRED_HEADSET) )
        return android_audio_legacy::AudioSystem::DEVICE_OUT_WIRED_HEADSET;
    else if(mRoutes &  (android_audio_legacy::AudioSystem::DEVICE_OUT_EARPIECE) )
        return android_audio_legacy::AudioSystem::DEVICE_OUT_EARPIECE;
    else if(mRoutes &  (android_audio_legacy::AudioSystem::DEVICE_OUT_HDMI_AUD ) )
        return android_audio_legacy::AudioSystem::DEVICE_OUT_HDMI_AUD;
    else if(mRoutes &  (android_audio_legacy::AudioSystem::DEVICE_OUT_SPEAKER) )
        return android_audio_legacy::AudioSystem::DEVICE_OUT_SPEAKER;
    else
    {
        LOGE("Audio_Find_Normal_Output_Device with no devices");
        return android_audio_legacy::AudioSystem::DEVICE_OUT_SPEAKER;
    }
}

int AudioYusuHardware::Audio_Find_Ringtone_Output_Device(uint32_t mRoutes)
{
    LOG_HARDWARE("Audio_Find_Ringtone_Output_Device mRoutes=0x%x", mRoutes);
    return Audio_Find_Normal_Output_Device(mRoutes); //ALPS00109966
}

int AudioYusuHardware::Audio_Find_Input_Device(uint32_t mRoutes)
{
    LOG_HARDWARE("Audio_Find_Input_Device mRoutes=0x%x", mRoutes);
    if(mRoutes&(android_audio_legacy::AudioSystem::DEVICE_IN_COMMUNICATION) )
        return android_audio_legacy::AudioSystem::DEVICE_IN_COMMUNICATION;
    else if(mRoutes &  (android_audio_legacy::AudioSystem::DEVICE_IN_WIRED_HEADSET) )
        return android_audio_legacy::AudioSystem::DEVICE_IN_WIRED_HEADSET;
    else if(mRoutes &  (android_audio_legacy::AudioSystem::DEVICE_IN_BUILTIN_MIC) )
        return android_audio_legacy::AudioSystem::DEVICE_IN_BUILTIN_MIC;
    else if(mRoutes &  (android_audio_legacy::AudioSystem::DEVICE_IN_AMBIENT) )
        return android_audio_legacy::AudioSystem::DEVICE_IN_AMBIENT;
    else if(mRoutes &  (android_audio_legacy::AudioSystem::DEVICE_IN_WIRED_HEADSET) )
        return android_audio_legacy::AudioSystem::DEVICE_IN_WIRED_HEADSET;
    else if(mRoutes &  (android_audio_legacy::AudioSystem::DEVICE_IN_AUX_DIGITAL) )
        return android_audio_legacy::AudioSystem::DEVICE_IN_AUX_DIGITAL;
    else if(mRoutes &  (android_audio_legacy::AudioSystem::DEVICE_IN_BACK_MIC) )
        return android_audio_legacy::AudioSystem::DEVICE_IN_BACK_MIC;
    else
    {
        LOGE("Audio_Match_input_device with no devices");
    }
    return 0;

}

int AudioYusuHardware::Audio_Find_Input_Communication_Device(uint32_t mRoutes)
{
    LOG_HARDWARE("Audio_Find_Input_Communication_Device mRoutes=0x%x", mRoutes);
    if(mRoutes&(android_audio_legacy::AudioSystem::DEVICE_IN_COMMUNICATION) )
        return android_audio_legacy::AudioSystem::DEVICE_IN_COMMUNICATION;
    else if(mRoutes &  (android_audio_legacy::AudioSystem::DEVICE_IN_BLUETOOTH_SCO_HEADSET))
        return android_audio_legacy::AudioSystem::DEVICE_IN_BLUETOOTH_SCO_HEADSET;
    else if(mRoutes &  (android_audio_legacy::AudioSystem::DEVICE_IN_BUILTIN_MIC) )
        return android_audio_legacy::AudioSystem::DEVICE_IN_BUILTIN_MIC;
    else if(mRoutes &  (android_audio_legacy::AudioSystem::DEVICE_IN_AMBIENT) )
        return android_audio_legacy::AudioSystem::DEVICE_IN_AMBIENT;
    else if(mRoutes &  (android_audio_legacy::AudioSystem::DEVICE_IN_WIRED_HEADSET) )
        return android_audio_legacy::AudioSystem::DEVICE_IN_WIRED_HEADSET;
    else if(mRoutes &  (android_audio_legacy::AudioSystem::DEVICE_IN_AUX_DIGITAL) )
        return android_audio_legacy::AudioSystem::DEVICE_IN_AUX_DIGITAL;
    else if(mRoutes &  (android_audio_legacy::AudioSystem::DEVICE_IN_BACK_MIC) )
        return android_audio_legacy::AudioSystem::DEVICE_IN_BACK_MIC;
    else
    {
        LOGE("Audio_Match_input_device with no devices");
    }
    return 0;
}

// Find out Whcih device should be return base on route and Mode
int AudioYusuHardware::Audio_Match_Output_device(uint32_t mRoutes, uint32 Mode, bool CheckForceUse)
{
    /*
       LOG_HARDWARE("Audio_Match_Output_device mRoutes=0x%x, mMode=%d, Mode=%d, CheckForceUse=%d, mOuputForceuse[Mode]=%d",
           mRoutes,mMode,Mode,CheckForceUse,mOuputForceuse[Mode]);
           */

    if(Mode == android_audio_legacy::AudioSystem::MODE_IN_CALL)
    {
        // check if force use exist
        if( mOuputForceuse[Mode] != 0 && CheckForceUse )
        {
            return Audio_Match_Force_device(mOuputForceuse[Mode]);
        }
        //return normal case
        else
        {
            return Audio_Find_Incall_Output_Device(mRoutes);
        }
    }
    else if(Mode == android_audio_legacy::AudioSystem::MODE_IN_COMMUNICATION)
    {
        // check if force use exist
        if( mOuputForceuse[Mode] != 0 && CheckForceUse )
        {
            return Audio_Match_Force_device(mOuputForceuse[Mode]);
        }
        //return normal case
        else
        {
            return Audio_Find_Communcation_Output_Device(mRoutes);
        }
    }
    else if(Mode == android_audio_legacy::AudioSystem::MODE_RINGTONE)
    {
        // check if force use exist
        if( mOuputForceuse[Mode] != 0 && CheckForceUse)
        {
            return Audio_Match_Force_device(mOuputForceuse[Mode]);
        }
        else
        {
            return Audio_Find_Ringtone_Output_Device(mRoutes);
        }
    }
    else if(Mode == android_audio_legacy::AudioSystem::MODE_NORMAL)
    {
        // check if force use exist
        if( mOuputForceuse[Mode] != 0 && CheckForceUse)
        {
            return Audio_Match_Force_device(mOuputForceuse[Mode]);
        }
        else
        {
            return Audio_Find_Normal_Output_Device(mRoutes);
        }
    }

    return 0;
}


int AudioYusuHardware::Audio_Match_Input_device(uint32_t mRoutes, bool CheckForceUse)
{
    // TODO:: check if need modify when IN_CALL mode
    LOG_HARDWARE("Audio_Match_Input_device mRoutes=0x%x, CheckForceUse=%d mInputForceuse = 0x%x",
                 mRoutes,CheckForceUse,mInputForceuse);
    if( mInputForceuse != 0 && CheckForceUse )
    {
        return Audio_Match_ForceInput_device(mInputForceuse);
    }
    //return normal case
    else
    {
        if(mMode == android_audio_legacy::AudioSystem::MODE_IN_COMMUNICATION)
        {
            return Audio_Find_Input_Communication_Device(mRoutes);
        }
        else
        {
            return Audio_Find_Input_Device(mRoutes);
        }
    }
    return 0;
}

status_t AudioYusuHardware::initCheck()
{
    LOG_HARDWARE("+initCheck ");
    if (mFd >= 0)
    {
        if (::access(kAudioDeviceName, O_RDWR) == NO_ERROR)
            return NO_ERROR;
    }
    return NO_INIT;
}

void AudioYusuHardware::Recover_State()
{
    LOGW("Recover_State bSpeechFlag=%d,bBgsFlag=%d,bRecordFlag=%d,bTtyFlag=%d,bVT=%d",
         mAudio_Control_State.bSpeechFlag ,
         mAudio_Control_State.bBgsFlag,
         mAudio_Control_State.bRecordFlag,
         mAudio_Control_State.bTtyFlag,
         mAudio_Control_State.bVT);

    if(mAudio_Control_State.bRecordFlag == true)
    {
        pLad->LAD_CloseNormalRecPath();
    }

    if(mAudio_Control_State.bBgsFlag == true)
    {
        pLad->LAD_BGSoundOff();
    }
    if(mAudio_Control_State.bVT == true)
    {
        pLad->LAD_PCM2WayOff();
        pLad->LAD_Speech_Off();
        mVT_Speech      = true;
        mPcm2Way_Enable = false;
        mAudio_Control_State.bSpeechFlag = false;
    }

    if(mAudio_Control_State.bSpeechFlag == true)
    {
        pLad->LAD_Speech_Off();
    }
    else
    {
        //if(mStreamHandler->GetSrcBlockRunning()){
        LOG_HARDWARE("Recover_State, +Afe_DL_Stop");
        mAfe_handle->Afe_DL_Stop(AFE_MODE_DAC);
        LOG_HARDWARE("Recover_State, -Afe_DL_Stop");
        //}
    }

    if(mAudio_Control_State.bTtyFlag != 0)
    {
        // todo  20100921
        pLad->LAD_TtyCtmOff();
        //SetTtyCtmMode(AUD_TTY_OFF);
    }

    // clean for previous setting.
    memset((void*)&mAudio_Control_State,0,sizeof(SPH_Control));
    ::ioctl(mFd, SET_AUDIO_STATE, &mAudio_Control_State);
}


void AudioYusuHardware::Get_Recovery_State()
{
    ::ioctl(mFd, GET_AUDIO_STATE,&mAudio_Control_State);
    LOGW("Get_Recovery_State bSpeechFlag=%d, bBgsFlag=%d, bRecordFlag=%d, bTtyFlag=%d, bVT=%d ",
         mAudio_Control_State.bSpeechFlag ,
         mAudio_Control_State.bBgsFlag,
         mAudio_Control_State.bRecordFlag,
         mAudio_Control_State.bTtyFlag,
         mAudio_Control_State.bVT);
}

void AudioYusuHardware::Set_Recovery_State()
{
    ::ioctl(mFd, SET_AUDIO_STATE,&mAudio_Control_State);
    LOGW("Set_Recovery_State bSpeechFlag=%d, bBgsFlag=%d, bRecordFlag=%d, bTtyFlag=%d, bVT=%d ",
         mAudio_Control_State.bSpeechFlag ,
         mAudio_Control_State.bBgsFlag,
         mAudio_Control_State.bRecordFlag,
         mAudio_Control_State.bTtyFlag,
         mAudio_Control_State.bVT);
}

int AudioYusuHardware::Get_Recovery_Speech(void)
{
    return mAudio_Control_State.bSpeechFlag;
}

void AudioYusuHardware::Set_Recovery_Speech(int Enable)
{
    mAudio_Control_State.bSpeechFlag = Enable;
    Set_Recovery_State();
}

int AudioYusuHardware::Get_Recovery_Bgs(void)
{
    return mAudio_Control_State.bBgsFlag;
}

void AudioYusuHardware::Set_Recovery_Bgs(int Enable)
{
    mAudio_Control_State.bBgsFlag = Enable;
    Set_Recovery_State();
}

int AudioYusuHardware::Get_Recovery_Record(void)
{
    return mAudio_Control_State.bRecordFlag;
}

void AudioYusuHardware::Set_Recovery_Record(int Enable)
{
    mAudio_Control_State.bRecordFlag = Enable;
    Set_Recovery_State();
}

void AudioYusuHardware::Set_Recovery_Tty(int tty_mode)
{
    mAudio_Control_State.bTtyFlag = tty_mode;
    Set_Recovery_State();
}

int AudioYusuHardware::Get_Recovery_VT(void)
{
    return mAudio_Control_State.bVT;
}

void AudioYusuHardware::Set_Recovery_VT(int Enable)
{
    mAudio_Control_State.bVT = Enable;
    Set_Recovery_State();
}


android_audio_legacy::AudioStreamOut* AudioYusuHardware::openOutputStream(uint32_t devices,int *format,
        uint32_t *channels,uint32_t *sampleRate,status_t *status)
{
    AutoMutex lock(mLock);
    // create new output stream
    LOG_HARDWARE("+openOutputStream()");
    android_audio_legacy::AudioStreamOut *out = mStreamHandler->openOutputStream(devices, format,  channels,  sampleRate, status);
    return out;
}

void AudioYusuHardware::closeOutputStream(android_audio_legacy::AudioStreamOut* out)
{
    mStreamHandler->closeOutputStream(out);
}

android_audio_legacy::AudioStreamIn* AudioYusuHardware::openInputStream(
    uint32_t devices,int *format, uint32_t *channels, uint32_t *sampleRate, status_t *status,
    android_audio_legacy::AudioSystem::audio_in_acoustics acoustics)
{

    AutoMutex lock(mLock);
    android_audio_legacy::AudioStreamIn *In = mStreamHandler->openInputStream(devices,format, channels,  sampleRate, status,acoustics);
    return In ;
}

size_t AudioYusuHardware::getInputBufferSize(uint32_t sampleRate, int format, int channelCount)
{
    return 320;
}

void AudioYusuHardware::closeInputStream(android_audio_legacy::AudioStreamIn* in)
{
    mStreamHandler->closeInputStream(in);
    // if (in == mInput) mInput = 0;
}

status_t AudioYusuHardware::setVoiceVolume(float v)
{
    #ifndef MTK_AUDIO_GAIN_TABLE_SUPPORT
    uint32 output_device =  Audio_Match_Output_device(mDevice,mMode,true);// get output devices
    LOGD("setVoiceVolume :%f, mode:%d, route:%d ",v,mMode,output_device);
    // setvoicevolume should only affect IN_CALL mode
    if(mMode == android_audio_legacy::AudioSystem::MODE_IN_COMMUNICATION)
    {
        mVolumeController->setVoIPVolume(v,mMode,output_device);
    }
    else
    {
        mVolumeController->setVoiceVolume(v,mMode,output_device);
    }
    #endif
    return NO_ERROR;
}

status_t AudioYusuHardware::setMasterVolume(float v)
{
   #ifndef MTK_AUDIO_GAIN_TABLE_SUPPORT
    LOG_HARDWARE("setMasterVolume=%f, device=0x%x, mMode:%d",v,mDevice,mMode);
    uint32 output_device =  Audio_Match_Output_device(mDevice,mMode,true);// get output devices
    if(mMode == android_audio_legacy::AudioSystem::MODE_IN_COMMUNICATION)
    {
        mVolumeController->setVoIPVolume(v,mMode,output_device);
    }
    else
    {
        mVolumeController->setMasterVolume(v,mMode,output_device);  // adust volume base on different devices and mode
    }
   #endif
    return NO_ERROR;
}

status_t AudioYusuHardware::setMicMute(bool state)
{
    LOG_HARDWARE("setMicMute state=%d, mMicMute=%d",state,mMicMute);

    // remove un-necessary Mic setting
    if(mMicMute_ModemStatus == state)
        return NO_ERROR;

    mMicMute = state;
    pLad->LAD_MuteMicrophone(mMicMute);
    mMicMute_ModemStatus = state;
    return NO_ERROR;
}

status_t AudioYusuHardware::getMicMute(bool* state)
{
    *state = mMicMute;
    LOG_HARDWARE("getMicMute mMicMute=%d",mMicMute);
    return NO_ERROR;
}

bool AudioYusuHardware::SetAnalogGain(unsigned int gain)
{
    LOGD("SetAnalogGain gain = %x",gain);
    mAnaReg->AnalogAFE_Set_DL_AUDHPL_PGA_Gain (gain);
    mAnaReg->AnalogAFE_Set_DL_AUDHPR_PGA_Gain (gain);
    return true;
}

void AudioYusuHardware::EnableStereoOutput(int value)
{
    LOG_HARDWARE("EnableStereoOutput= %d\n",value);
    if(value > 0)
    {
        mEnableStereoOutput = true;
        mAfe_handle->Afe_Set_Stereo();
    }
    else
    {
        mEnableStereoOutput = false;
        mAfe_handle->Afe_Set_Mono();
    }
}

bool AudioYusuHardware::SetSpeechGaintableVolume(int gain)
{
    LOGD("SetSpeechGaintableVolume = %x",gain);
    pLad->LAD_SetGainTableOutputVolume (gain);
    return true;
}

bool AudioYusuHardware::SetApMicType(int mictype)
{
    if((mictype < LADIN_FM_Radio) || (mictype >NUM_OF_InputDevice ))
    {
        LOGW("SetApMicType error mictype = %d",mictype);
        return false;
    }
    LOGD("SetApMicType mictype = %d",mictype);
    ApRecordMicYype = mictype;
    return true;
}
int AudioYusuHardware::GetApMicType()
{
    return ApRecordMicYype;
}

//Is there need another mode for Voip gain??
bool AudioYusuHardware::SetMicGain()
{
    int Route_device = -1;
    LOG_HARDWARE("SetMicGain mInputAvailable=0x%x", mInputAvailable);
    Route_device = Audio_Match_Input_device(mInputAvailable,true);
    // don't set Mic gain when in call mode.
    if(mMode == android_audio_legacy::AudioSystem::MODE_IN_CALL)
        return false;

    LOG_HARDWARE("SetMicGain Route_device=0x%x", Route_device);

    switch(Route_device)
    {
    case android_audio_legacy::AudioSystem::DEVICE_IN_BUILTIN_MIC:
    case android_audio_legacy::AudioSystem::DEVICE_IN_COMMUNICATION:
    case android_audio_legacy::AudioSystem::DEVICE_IN_AUX_DIGITAL:
    {
        LOG_HARDWARE("SetMicGain, Earpiece/Headphone, Handfree_Mic");
        SetInputSource(LADIN_Microphone1);
        pLad->LAD_SetSpeechMode(SPH_MODE_LOUDSPK);
        mVolumeController->SetLadMicGain(Idle_Normal_Record);
        break;
    }
    case android_audio_legacy::AudioSystem::DEVICE_IN_WIRED_HEADSET:
    {
        LOG_HARDWARE("SetMicGain, Headset, Headset_Mic");
        SetInputSource(LADIN_Microphone2);
        pLad->LAD_SetSpeechMode(SPH_MODE_EARPHONE);
        mVolumeController->SetLadMicGain(Idle_Headset_Record);
        break;
    }
    case android_audio_legacy::AudioSystem::DEVICE_IN_BLUETOOTH_SCO_HEADSET:
    {
        LOG_HARDWARE("SetMicGain, BT, BT-SCO ");
#ifdef ENABLE_VOIP_BTSCO
        pLad->LAD_SetSpeechMode(SPH_MODE_BT_EARPHONE);
        SetInputSource(LADIN_BTIn);
        mVolumeController->SetLadMicGain(Headset_Mic);
#endif
        break;
    }
    default:
    {
        LOG_HARDWARE("SetMicGain default Idle_Normal_Record");
        SetInputSource(LADIN_Microphone1);
        pLad->LAD_SetSpeechMode(SPH_MODE_NORMAL);
        mVolumeController->SetLadMicGain(Idle_Normal_Record);
        break;
    }
    }
    return true;
}


status_t AudioYusuHardware::setMode(int mode)
{
    LOGD("setMode=%d", mode);

    if ((mode < 0) || (mode >= android_audio_legacy::AudioSystem::NUM_MODES))
        return BAD_VALUE;
    if (mMode == mode)
        return ALREADY_EXISTS;

    // Fix issue: only for leave InCall mode and enter Normal mode.
    // if leave InCall mode and enter RingTone mode, don't sleep.
    /*
       if(mMode == android_audio_legacy::AudioSystem::MODE_IN_CALL &&
          mode == android_audio_legacy::AudioSystem::MODE_NORMAL && GetVmFlag() == false ){
          LOGD("setMode=%d, usleep(%d ms)", mode,LAD_BUFFER_LATENCY);
          usleep(LAD_BUFFER_LATENCY*1000);
       }
    */

    if(mMode == android_audio_legacy::AudioSystem::MODE_IN_CALL &&  mode == android_audio_legacy::AudioSystem::MODE_NORMAL)
    {
        // Fixed end call tone issue.
        //-- Temp solution for Modem side --//
        // End the call, sleep for a while.
        // Because modem side need to consume all the background sound data.
        // only for this case: Leave InCall mode and Enter Normal mode. Otherwise, don't sleep.
        // This is experienced value. The right solution is modem side fixed.
        int m_sleep = 800;
        LOGD("setMode=%d, -(InCall) +(Normal) usleep(%d ms)", mode,m_sleep);
        usleep(m_sleep*1000);
    }
    else if(mMode == android_audio_legacy::AudioSystem::MODE_IN_COMMUNICATION &&  mode == android_audio_legacy::AudioSystem::MODE_NORMAL)
    {
        int m_sleep = 500;
        LOGD("setMode=%d, -(MODE_IN_COMMUNICATION) +(Normal) usleep(%d ms)", mode,m_sleep);
        usleep(m_sleep*1000);
    }
    else if(mMode == android_audio_legacy::AudioSystem::MODE_RINGTONE&&  mode == android_audio_legacy::AudioSystem::MODE_IN_CALL)
    {
        // Fixed answer call ringtone issue.
        // Answer the call, sleep for a while.
        // Because some ringtone data is not consumed by audio hw, they will be routed to LAD_BGS after answering phone call.
        // only for this case: Leave Ringtone mode and Enter InCall mode. Otherwise, don't sleep.
        // This is experienced value.
        int m_sleep = 250;
        LOGD("setMode=%d, -(Ringtone) +(InCall) usleep(%d ms)", mode,m_sleep);
        usleep(m_sleep*1000);
    }
    pthread_mutex_lock(&ModeMutex);
    mMode = mode;  // mode change ,save mode
   #ifdef MTK_AUDIO_GAIN_TABLE_SUPPORT
   mAudioUCM->SetPhoneMode (mode);
   #endif
    doModeRouting ();
    mPreviousMode = mMode; // save old mode
    pthread_mutex_unlock(&ModeMutex);

    return NO_ERROR;
}

status_t AudioYusuHardware::doModeRouting()
{
    LOG_HARDWARE("+doModeRouting ");
    ioctl(mFd,AUD_SET_CLOCK,1);
    int PreRoute_device = -1,Route_device = -1;
    Route_device = Audio_Match_Output_device(mOutputAvailable[mMode],mMode,true); // turn on this deivce
    PreRoute_device = Audio_Match_Output_device(mOutputAvailable[mPreviousMode],mPreviousMode,true); // turn off this device
    mPredevice = PreRoute_device;
    mDevice = Route_device;
    LOG_HARDWARE("doModeRouting PreRoute_device=0x%x, Route_device=0x%x, mMode=%d, mPreviousmode=%d",
                 PreRoute_device,Route_device,mMode,mPreviousMode);

    // CR fix , whne incall mode and vm flag is disable , check for audiostreamin is record for vm and close.
    //if(mPreviousMode == android_audio_legacy::AudioSystem::MODE_IN_CALL && (GetVmFlag () == true||mStreamHandler->mInput->format () == android_audio_legacy::AudioSystem::VM_FMT))
    if(mPreviousMode == android_audio_legacy::AudioSystem::MODE_IN_CALL && (GetVmFlag () == true ||
                          (mStreamHandler->mInput[0] && mStreamHandler->mInput[0]->format ()==android_audio_legacy::AudioSystem::VM_FMT)))
    {
        LOGD("doModeRouting, Enable_Modem_Record false");
        Enable_Modem_Record(false);
    }

#ifdef HEADPHONE_CONTROL
	if(PreRoute_device == android_audio_legacy::AudioSystem::DEVICE_OUT_WIRED_HEADSET ||
		PreRoute_device == android_audio_legacy::AudioSystem::DEVICE_OUT_WIRED_HEADPHONE)
	{
		SetHeadPhoneStatus(false);
		EnableHeadPhone();
	}

	if(PreRoute_device ==  android_audio_legacy::AudioSystem::DEVICE_OUT_SPEAKER &&
		( mOuputForceuse[mPreviousMode] == android_audio_legacy::AudioSystem::FORCE_SPEAKER) &&
		IsHeadsetPlugin(mPreviousMode))
	{
		LOG_HARDWARE("close speaker and headphone");
		SetHeadPhoneStatus(false);
		EnableHeadPhone();
	}
#endif

#ifndef HEADPHONE_CONTROL
#ifdef ALL_USING_VOICEBUFFER_INCALL
    SetHeadPhoneStatus(false);
    EnableHeadPhone();
#endif
#endif

    if((mPreviousMode == android_audio_legacy::AudioSystem::MODE_RINGTONE && mMode == android_audio_legacy::AudioSystem::MODE_NORMAL )||
            (mMode == android_audio_legacy::AudioSystem::MODE_RINGTONE && mPreviousMode == android_audio_legacy::AudioSystem::MODE_NORMAL ))
    {
        if(mPredevice ==mDevice )
        {
            LOGD("doModeRouting, device not change");
            if(mMode == android_audio_legacy::AudioSystem::MODE_NORMAL &&GetAnalogLineinEnable () == true)
            {
                LOGD("doModeRouting with device not change but fmenable");
                ForceDisableSpeaker();
                mAnaReg->AnalogAFE_Close(NONE_PATH);
                mAnaReg->AnalogAFE_Open(FM_PATH_STEREO);
                SetModeToAMP(mMode);
                if(GetFmSpeakerStatus ())
                {
                    SetFmSpeaker ();
                }
                else
                {
                    EnableSpeaker();
                }
            }

            // set the inter-connection path when modem reset and current device = speaker
            // Because when MT call coming and don't answer the call.
            // Make modem reset. The inter-connection (I6-O3, I5-O4) path would be clean by modem side.
            // And here if device(only for speaker) is not changed, the itner-connection path would not be set again.
            if(mDevice == android_audio_legacy::AudioSystem::DEVICE_OUT_SPEAKER)
            {
                LOGD("doModeRouting, device not change, SPK");
#ifdef ENABLE_STEREO_SPEAKER
                mAfe_handle->Afe_Set_Stereo();
#else
                mAfe_handle->Afe_Set_Mono();
#endif
            }

            goto MODE_CHANGE_EXIT;
        }
    }

    // move here to avoid  mutex lock but goto MODE_CHANGE_EXIT
    if(mStreamHandler->SetCloseRec(mPreviousMode,mMode)== true)
    {
        LOG_HARDWARE("doModeRouting, SetCloseRec");
    }

    switch(mPreviousMode)  // this is turn off devices
    {
    case android_audio_legacy::AudioSystem::MODE_NORMAL:  //  hardware mode
    {
        switch(PreRoute_device)
        {
        case android_audio_legacy::AudioSystem::DEVICE_OUT_WIRED_HEADSET:
        {
            LOG_HARDWARE("close, N, HEADSET");
            ForceDisableSpeaker();
            mAnaReg->AnalogAFE_Close(NONE_PATH);
            break;
        }
        case android_audio_legacy::AudioSystem::DEVICE_OUT_WIRED_HEADPHONE:
        {
            LOG_HARDWARE("close, N, HEADPHONE");
            ForceDisableSpeaker();
            mAnaReg->AnalogAFE_Close(NONE_PATH);
            break;
        }
        case android_audio_legacy::AudioSystem::DEVICE_OUT_BLUETOOTH_SCO:
        case android_audio_legacy::AudioSystem::DEVICE_OUT_BLUETOOTH_SCO_HEADSET:
        case android_audio_legacy::AudioSystem::DEVICE_OUT_BLUETOOTH_SCO_CARKIT:
        {
            LOG_HARDWARE("close, N, BT_SCO");
#ifdef ENABLE_VOIP_BTSCO
            pLad->LAD_SetSpeechMode(SPH_MODE_NORMAL);
#endif
            break;
        }
        case android_audio_legacy::AudioSystem::DEVICE_OUT_SPEAKER:
        {
            LOG_HARDWARE("close, N, SPEAKER");
            SetSpeakerStatus(false);
            EnableSpeaker();
            mAnaReg->AnalogAFE_Close(NONE_PATH);
            break;
        }
        case android_audio_legacy::AudioSystem::DEVICE_OUT_EARPIECE:
        {
            LOG_HARDWARE("close, N, EARPIECE");
#ifdef EARPIECE_CONTROL
            SetEarpieceStatus(false);
            EnableEarpiece();
#endif
            mAnaReg->AnalogAFE_Close(NONE_PATH);
            break;
        }
        case android_audio_legacy::AudioSystem::DEVICE_OUT_AUX_DIGITAL:
        {
            //FM TX Close
            LOGD("close, N, FM TX");
            SetSpeakerStatus(false);
            EnableSpeaker();
            mAnaReg->AnalogAFE_Close(NONE_PATH);
            if(mStreamHandler->GetSrcBlockRunning ())
            {
                //connect dac
                mAfe_handle->Afe_Set_FmTx(false,1);
            }
            else
            {
                mAfe_handle->Afe_Set_FmTx(false,0);
            }
            break;
        }
        case android_audio_legacy::AudioSystem::DEVICE_OUT_HDMI_AUD:
        {
            LOGD("close, N, HDMI_Audio");
            SetSpeakerStatus(false);
            EnableSpeaker();
            mAnaReg->AnalogAFE_Close(NONE_PATH);
            SetHDMIAudioConnect(false);
            break;
        }
        default:
        {
            LOG_HARDWARE("doModeRouting, PreRoute_device not specified ");
        }
        }
        break;
    }
    case android_audio_legacy::AudioSystem::MODE_RINGTONE: // case rintgone
    {
        switch(PreRoute_device)
        {
        case android_audio_legacy::AudioSystem::DEVICE_OUT_WIRED_HEADSET :
        {
            LOG_HARDWARE("close, R, HEADSET");
            SetSpeakerStatus(false);
            EnableSpeaker();
            mAnaReg->AnalogAFE_Close(AUDIO_PATH);
            break;
        }
        case android_audio_legacy::AudioSystem::DEVICE_OUT_WIRED_HEADPHONE :
        {
            LOG_HARDWARE("close, R, HEADPHONE");
            SetSpeakerStatus(false);
            EnableSpeaker();
            mAnaReg->AnalogAFE_Close(AUDIO_PATH);
            break;
        }
        case android_audio_legacy::AudioSystem::DEVICE_OUT_BLUETOOTH_SCO:
        case android_audio_legacy::AudioSystem::DEVICE_OUT_BLUETOOTH_SCO_HEADSET:
        case android_audio_legacy::AudioSystem::DEVICE_OUT_BLUETOOTH_SCO_CARKIT:
        {
            LOG_HARDWARE("close, R, BT_SCO");
            SetSpeakerStatus(false); // set spaker status false
            EnableSpeaker();
            pLad->LAD_SetSpeechMode(SPH_MODE_NORMAL);
            break;
        }
        case android_audio_legacy::AudioSystem::DEVICE_OUT_SPEAKER:
        {
            LOG_HARDWARE("close, R, SPEAKER");
            SetSpeakerStatus(false);
            EnableSpeaker();
#ifdef HEADPHONE_CONTROL
#endif
            mAnaReg->AnalogAFE_Close(AUDIO_PATH);
            break;
        }
        case android_audio_legacy::AudioSystem::DEVICE_OUT_EARPIECE:
        {
            LOG_HARDWARE("close, R, EARPIECE");
#ifdef EARPIECE_CONTROL
            SetEarpieceStatus(false);
            EnableEarpiece();
#endif
            mAnaReg->AnalogAFE_Close(NONE_PATH);
            break;
        }
        case android_audio_legacy::AudioSystem::DEVICE_OUT_AUX_DIGITAL:
        {
            //FM TX Close
            LOGD("close, R, FM TX");
            SetSpeakerStatus(false);
            EnableSpeaker();
            mAnaReg->AnalogAFE_Close(NONE_PATH);
            if(mStreamHandler->GetSrcBlockRunning ())
            {
                //connect dac
                mAfe_handle->Afe_Set_FmTx(false,1);
            }
            else
            {
                mAfe_handle->Afe_Set_FmTx(false,0);
            }
            break;
        }
        case android_audio_legacy::AudioSystem::DEVICE_OUT_HDMI_AUD:
        {
            LOGD("close, R, HDMI_Audio");
            SetSpeakerStatus(false);
            EnableSpeaker();
            mAnaReg->AnalogAFE_Close(NONE_PATH);
            SetHDMIAudioConnect(false);
            break;
        }
        default:
        {
            LOG_HARDWARE("doModeRouting, PreRoute_device not specified ");
        }
        }
        break;
    }
    case android_audio_legacy::AudioSystem::MODE_IN_CALL:  // incall mode
    {
        SetVoiceStatus(false);
        // need configure BGS off before speech off
        if(mPreviousMode == android_audio_legacy::AudioSystem::MODE_IN_CALL)
        {
            //mStreamHandler->ResetLadPlayer(mMode);
            mStreamHandler->ForceResetLadPlayer(mMode);
        }

        if(mPcm2Way_Enable == true)
        {
            pLad->LAD_PCM2WayOff();
            mPcm2Way_Enable = false;
        }

        if(pLad->mTty_used_in_phone_call == true)
        {
            LOG_HARDWARE("doModeRouting, LAD_TtyCtmOff()");
            mTty_Ctm = AUD_TTY_OFF;

            // prevent dead lock when doOutputDeviceRouting()
            pthread_mutex_unlock(&ModeMutex);
            SetTtyCtmMode(AUD_TTY_OFF); //pLad->LAD_TtyCtmOff(); // turn off TTY-CTM
            pthread_mutex_lock(&ModeMutex);
        }

#ifdef ENABLE_EXT_DAC
        mAudioCustomFunction->EXT_DAC_TurnOffSpeaker (EXT_DAC_LINEIN,AUDIO_EXP_DAC_BUF0);
        mAudioCustomFunction->EXT_DAC_TurnOffSpeaker (EXT_DAC_LINEIN,AUDIO_EXP_DAC_BUF1);
#endif

        LOG_HARDWARE("doModeRouting, LAD_Speech_Off()");
        pLad->LAD_Speech_Off();   // turn off SPEECH
        //this->Set_Recovery_Speech(false);
        mVT_Speech = false;

//#if defined(MTK_DUAL_MIC_SUPPORT)
//         if(mMMI_DualMic_Setting != mHAL_DualMic_Setting)       //sets only when mmi & HAL different
//            MMI_Set_Dual_Mic_Enable(mMMI_DualMic_Setting);
//#endif

        switch(PreRoute_device)
        {
        case android_audio_legacy::AudioSystem::DEVICE_OUT_WIRED_HEADSET :
        {
            LOG_HARDWARE("close, C, HEADSET");
            ForceDisableSpeaker();
            mAnaReg->AnalogAFE_Close(AUDIO_PATH);
            break;
        }
        case android_audio_legacy::AudioSystem::DEVICE_OUT_WIRED_HEADPHONE :
        {
            LOG_HARDWARE("close, C, HEADPHONE");
            ForceDisableSpeaker();
            mAnaReg->AnalogAFE_Close(AUDIO_PATH);
            break;
        }
        case android_audio_legacy::AudioSystem::DEVICE_OUT_BLUETOOTH_SCO:
        case android_audio_legacy::AudioSystem::DEVICE_OUT_BLUETOOTH_SCO_HEADSET:
        case android_audio_legacy::AudioSystem::DEVICE_OUT_BLUETOOTH_SCO_CARKIT:
        {
            LOG_HARDWARE("close, C, BT_SCO");
            pLad->LAD_SetSpeechMode(SPH_MODE_NORMAL);
            break;
        }
        case android_audio_legacy::AudioSystem::DEVICE_OUT_SPEAKER:
        {
            LOG_HARDWARE("close, C, SPEAKER");
            SetSpeakerStatus(false);
            ForceDisableSpeaker();
            mAnaReg->AnalogAFE_Close(AUDIO_PATH);
            break;
        }
        case android_audio_legacy::AudioSystem::DEVICE_OUT_EARPIECE:
        {
            LOG_HARDWARE("close, C, EARPIECE");
#ifdef EARPIECE_CONTROL
            SetEarpieceStatus(false);
            EnableEarpiece();
#endif
            //Disable: This will make pop noise.(Modem side will close analog hw)
            //mAnaReg->AnalogAFE_Close(NONE_PATH);
            break;
        }
        case android_audio_legacy::AudioSystem::DEVICE_OUT_AUX_DIGITAL:
        {
            LOGD("close, C, FM TX");
            SetSpeakerStatus(false);
            EnableSpeaker();
            mAnaReg->AnalogAFE_Close(NONE_PATH);
            if(mStreamHandler->GetSrcBlockRunning ())
            {
                //connect dac
                mAfe_handle->Afe_Set_FmTx(false,1);
            }
            else
            {
                mAfe_handle->Afe_Set_FmTx(false,0);
            }
            break;
        }
        case android_audio_legacy::AudioSystem::DEVICE_OUT_HDMI_AUD:
        {
            LOGD("close, C, HDMI_Audio");
            SetSpeakerStatus(false);
            EnableSpeaker();
            mAnaReg->AnalogAFE_Close(NONE_PATH);
            SetHDMIAudioConnect(false);
            break;
        }
        default:
        {
            LOG_HARDWARE("doModeRouting, In-Call, PreRoute_device not specified");
        }
        }
        LOGD("doModeRouting, Power Leaveing In-Call mode.");
        SwitchAudioClock(false);  // Leave In-Call mode, disable the audio power
        break;
    }
    case android_audio_legacy::AudioSystem::MODE_IN_COMMUNICATION:
    {
        // VOIP call is finish ......
        switch(PreRoute_device)
        {
        case android_audio_legacy::AudioSystem::DEVICE_OUT_WIRED_HEADSET:
        {
            LOG_HARDWARE("close, CM, HEADSET");
            mAnaReg->AnalogAFE_Close(NONE_PATH);
            break;
        }
        case android_audio_legacy::AudioSystem::DEVICE_OUT_WIRED_HEADPHONE:
        {
            LOG_HARDWARE("close, CM, HEADPHONE");
            mAnaReg->AnalogAFE_Close(NONE_PATH);
            break;
        }
        case android_audio_legacy::AudioSystem::DEVICE_OUT_BLUETOOTH_SCO:
        case android_audio_legacy::AudioSystem::DEVICE_OUT_BLUETOOTH_SCO_HEADSET:
        case android_audio_legacy::AudioSystem::DEVICE_OUT_BLUETOOTH_SCO_CARKIT:
        {
            LOG_HARDWARE("close, CM, BT_SCO");
#ifdef ENABLE_VOIP_BTSCO
            // when VOIP call is stopped,  stop record and start record if needed.
            if(!(mPreviousMode==android_audio_legacy::AudioSystem::MODE_IN_CALL||mMode==android_audio_legacy::AudioSystem::MODE_IN_CALL))
            {
                mStreamHandler->InputStreamLock();
            }
            pLad->LAD_SetSpeechMode(SPH_MODE_NORMAL);
            if(!(mPreviousMode==android_audio_legacy::AudioSystem::MODE_IN_CALL||mMode==android_audio_legacy::AudioSystem::MODE_IN_CALL))
            {
                mStreamHandler->InputStreamUnLock();
            }
            mStreamHandler->RestoreOutputStream();
#endif

            break;
        }
        case android_audio_legacy::AudioSystem::DEVICE_OUT_SPEAKER:
        {
            LOG_HARDWARE("close, CM, SPEAKER");
            SetSpeakerStatus(false);
            EnableSpeaker();
            mAnaReg->AnalogAFE_Close(NONE_PATH);
            break;
        }
        case android_audio_legacy::AudioSystem::DEVICE_OUT_EARPIECE:
        {
            LOG_HARDWARE("close, CM, EARPIECE");
#ifdef EARPIECE_CONTROL
            SetEarpieceStatus(false);
            EnableEarpiece();
#endif
            mAnaReg->AnalogAFE_Close(NONE_PATH);
            break;
        }
        case android_audio_legacy::AudioSystem::DEVICE_OUT_AUX_DIGITAL:
        {
            //FM TX Close
            LOGD("close, CM, FM TX");
            SetSpeakerStatus(false);
            EnableSpeaker();
            mAnaReg->AnalogAFE_Close(NONE_PATH);
            if(mStreamHandler->GetSrcBlockRunning ())
            {
                //connect dac
                mAfe_handle->Afe_Set_FmTx(false,1);
            }
            else
            {
                mAfe_handle->Afe_Set_FmTx(false,0);
            }
            break;
        }
        case android_audio_legacy::AudioSystem::DEVICE_OUT_HDMI_AUD:
        {
            LOGD("close, CM, HDMI_Audio");
            SetSpeakerStatus(false);
            EnableSpeaker();
            mAnaReg->AnalogAFE_Close(NONE_PATH);
            SetHDMIAudioConnect(false);
            break;
        }
        default:
        {
            LOG_HARDWARE("doModeRouting, CM, PreRoute_device not specified ");
        }
        }
        LOGD("doModeRouting SetCommunicationStatus false");
        SetCommunicationStatus (false);
        SwitchAudioClock(false);
        break;
    }
    default:
    {
        LOG_HARDWARE("doModeRouting, PreviousMode no switch");
        break;
    }
    }

    SetModeToAMP(mMode);
    switch(mMode)  // this is turn on devices
    {
        LOG_HARDWARE("doModeRouting, mMode=%d",mMode);
    case android_audio_legacy::AudioSystem::MODE_NORMAL:
    {
        LOG_HARDWARE("doModeRouting, mMode = MODE_NORMAL");
        //pLad->LAD_SwitchMicBias(0);
        switch(Route_device)
        {
        case android_audio_legacy::AudioSystem::DEVICE_OUT_WIRED_HEADSET :
        {
            LOG_HARDWARE("open, N, HEADSET");
            mAfe_handle->Afe_Set_Stereo();
            SetSpeakerStatus(false);
            EnableSpeaker();
            mAnaReg->AnalogAFE_Close(AUDIO_PATH);
            SetInputSource(LADIN_Microphone2);
            if(GetAnalogLineinEnable())
                mAnaReg->AnalogAFE_Open(FM_PATH_STEREO);
            else if(mStreamHandler->GetSrcBlockRunning ())
                mAnaReg->AnalogAFE_Open(AUDIO_PATH);

            break;
        }
        case android_audio_legacy::AudioSystem::DEVICE_OUT_WIRED_HEADPHONE :
        {
            LOG_HARDWARE("open, N, HEADPHONE");
            mAfe_handle->Afe_Set_Stereo();
            SetSpeakerStatus(false);
            EnableSpeaker();
            mAnaReg->AnalogAFE_Close(AUDIO_PATH);
            if(GetAnalogLineinEnable())
                mAnaReg->AnalogAFE_Open(FM_PATH_STEREO);
            else if(mStreamHandler->GetSrcBlockRunning ())
                mAnaReg->AnalogAFE_Open(AUDIO_PATH);
            break;
        }
        case android_audio_legacy::AudioSystem::DEVICE_OUT_BLUETOOTH_SCO:
        case android_audio_legacy::AudioSystem::DEVICE_OUT_BLUETOOTH_SCO_HEADSET:
        case android_audio_legacy::AudioSystem::DEVICE_OUT_BLUETOOTH_SCO_CARKIT:
        {
            LOG_HARDWARE("open, N, BT_SCO");
            //pLad->LAD_SetInputSource(ChooseInputSource(LADIN_BTIn));
            break;
        }
        case android_audio_legacy::AudioSystem::DEVICE_OUT_SPEAKER:
        {
            LOG_HARDWARE("open, N, SPEAKER");
            if((mStreamHandler->GetSrcBlockRunning() > 0 )||GetFmRxStatus ()||GetMatvAnalogEnable ())
            {//Limit situation to set afe as mono or stereo. It might cause pop noise.
#ifdef ENABLE_STEREO_SPEAKER
                mAfe_handle->Afe_Set_Stereo();
#else
                mAfe_handle->Afe_Set_Mono();
#endif
            }
            SetSpeakerStatus(false); // set spaker status false
            EnableSpeaker();
            mAnaReg->AnalogAFE_Close(AUDIO_PATH);
            SetInputSource(LADIN_Microphone1);
            if(GetAnalogLineinEnable()&& GetFmSpeakerStatus ())
                mAnaReg->AnalogAFE_Open(FM_PATH_STEREO);
            else if(mStreamHandler->GetSrcBlockRunning ())
                mAnaReg->AnalogAFE_Open(AUDIO_PATH);

            SetSpeakerStatus(true); // set spaker status true

            if(mStreamHandler->GetSrcBlockRunning() > 0 )
            {
                EnableSpeaker();// open speaker
            }
            //judge if fm enable situation
            else if(GetFmRxStatus ())
            {
                SetFmSpeaker ();
            }
            else if(GetMatvAnalogEnable ())
            {
                SetMatvSpeaker ();
            }
            break;
        }
        case android_audio_legacy::AudioSystem::DEVICE_OUT_AUX_DIGITAL:
        {
            LOG_HARDWARE("open, N, AUX_DIGITAL");
            SetSpeakerStatus(false); // set spaker status false
            EnableSpeaker();
            if(mStreamHandler->GetSrcRunningNumber() > 0)
            {
#ifdef FM_ANALOG_OUT_SUPPORT
                mAnaReg->AnalogAFE_Open(AUDIO_PATH);
#endif
                mAfe_handle->Afe_Set_FmTx(mI2STxStatus,1);
            }
            else
            {
                mAfe_handle->Afe_Set_FmTx(mI2STxStatus,0);
            }
            break;
        }
        case android_audio_legacy::AudioSystem::DEVICE_OUT_EARPIECE:
        {
            LOG_HARDWARE("open, N, EARPIECE");
            mAfe_handle->Afe_Set_Mono();
            SetInputSource(LADIN_Microphone1);
            if(mStreamHandler->GetSrcBlockRunning ())
            {
                mAnaReg->AnalogAFE_Open(VOICE_PATH);
            }
#ifdef EARPIECE_CONTROL
            SetEarpieceStatus(true);
            EnableEarpiece();
#endif
            break;
        }
        case android_audio_legacy::AudioSystem::DEVICE_OUT_HDMI_AUD:
        {
            LOGD("open, N, HDMI_Audio");
            SetSpeakerStatus(false);
            EnableSpeaker();
            SetHDMIAudioConnect(true);
            break;
        }
        default:
        {
            LOG_HARDWARE("doModeRouting, mMode not specidied");
        }
        }
        break;
    }
    case android_audio_legacy::AudioSystem::MODE_RINGTONE:
    {
        LOG_HARDWARE("doModeRouting, mMode = MODE_RINGTONE");  // in RINGOTNE mode
        //pLad->LAD_SwitchMicBias(1);
        switch(Route_device)
        {
        case android_audio_legacy::AudioSystem::DEVICE_OUT_WIRED_HEADSET :
        {
            LOG_HARDWARE("open, R, HEADSET");
            mAfe_handle->Afe_Set_Stereo();
            EnableSpeaker();
            mAnaReg->AnalogAFE_Close(AUDIO_PATH);
            SetInputSource(LADIN_Microphone2);
            mAnaReg->AnalogAFE_Open(AUDIO_PATH);
            SetSpeakerStatus(true);
            EnableSpeaker();
            break;
        }
        case android_audio_legacy::AudioSystem::DEVICE_OUT_WIRED_HEADPHONE :
        {
            LOG_HARDWARE("open, R, HEADPHONE");
            mAfe_handle->Afe_Set_Stereo();
            EnableSpeaker();
            mAnaReg->AnalogAFE_Close(AUDIO_PATH);
            mAnaReg->AnalogAFE_Open(AUDIO_PATH);
            SetSpeakerStatus(true);
            EnableSpeaker();
            break;
        }
        case android_audio_legacy::AudioSystem::DEVICE_OUT_BLUETOOTH_SCO:
        case android_audio_legacy::AudioSystem::DEVICE_OUT_BLUETOOTH_SCO_HEADSET:
        case android_audio_legacy::AudioSystem::DEVICE_OUT_BLUETOOTH_SCO_CARKIT:
        {
            LOG_HARDWARE("open, R, BT_SCO");
            EnableSpeaker();
            mAnaReg->AnalogAFE_Close(AUDIO_PATH);
            SetInputSource(LADIN_BTIn);
            mAnaReg->AnalogAFE_Open(AUDIO_PATH);
            SetSpeakerStatus(true);
            EnableSpeaker();
            break;
        }
        case android_audio_legacy::AudioSystem::DEVICE_OUT_SPEAKER:
        {
            LOG_HARDWARE("open, R, SPEAKER");
#ifdef ENABLE_STEREO_SPEAKER
            mAfe_handle->Afe_Set_Stereo();
#else
            mAfe_handle->Afe_Set_Mono();
#endif
            //--> to fix ringtone's pop noise for ALPS00039680
            /*
            mAnaReg->AnalogAFE_Close(AUDIO_PATH);
            pLad->LAD_SetInputSource(ChooseInputSource(LADIN_Microphone1));
            mAnaReg->AnalogAFE_Open(AUDIO_PATH);
            */
            //<-- to fix ringtone's pop noise for ALPS00039680
            if(mStreamHandler->GetSrcBlockRunning ())//to prevent the audio hw is turned off on close stage.
            {
                mAnaReg->AnalogAFE_Open(AUDIO_PATH);
            }
            if(Get_Recovery_Speech()==false)//to prevent TDD noise before speech is turned off
            {
                SetSpeakerStatus(true); // set spaker status true
               if(mStreamHandler->GetSrcBlockRunning() > 0 ){
                  EnableSpeaker();// open speaker
               }
            }
#ifdef HEADPHONE_CONTROL
#endif
            break;
        }
        case android_audio_legacy::AudioSystem::DEVICE_OUT_EARPIECE:
        {
            LOG_HARDWARE("open, R, EARPIECE");
            mAfe_handle->Afe_Set_Mono();
            SetInputSource(LADIN_Microphone1);
            if(mStreamHandler->GetSrcBlockRunning ())
            {
                mAnaReg->AnalogAFE_Open(VOICE_PATH);
            }
#ifdef EARPIECE_CONTROL
            SetEarpieceStatus(true);
            EnableEarpiece();
#endif
            break;
        }
        case android_audio_legacy::AudioSystem::DEVICE_OUT_AUX_DIGITAL:
        {
            LOGW("doModeRouting, R, AUX_DIGITAL");
            SetSpeakerStatus(false); // set spaker status false
            EnableSpeaker();
            if(mStreamHandler->GetSrcRunningNumber() > 0)
            {
#ifdef FM_ANALOG_OUT_SUPPORT
                mAnaReg->AnalogAFE_Open(AUDIO_PATH);
#endif
                mAfe_handle->Afe_Set_FmTx(mI2STxStatus,1);
            }
            else
            {
                mAfe_handle->Afe_Set_FmTx(mI2STxStatus,0);
            }
            break;
        }
        case android_audio_legacy::AudioSystem::DEVICE_OUT_HDMI_AUD:
        {
            LOGD("open, R, HDMI_Audio(disable)");
            SetHDMIAudioConnect(false);
            break;
        }
        default:
        {
            LOG_HARDWARE("doModeRouting, R, mMode not specidied");
        }
        }
        break;
    }
    case android_audio_legacy::AudioSystem::MODE_IN_CALL:
    {
        LOGD("doModeRouting, Power Entering In-Call mode.");
        SwitchAudioClock(true);  // Enter In-Call mode, enable the audio power
        SetVoiceStatus(true);

        LOG_HARDWARE("MODE_IN_CALL SetVoiceStatus(true)");
        switch(Route_device)
        {
        case android_audio_legacy::AudioSystem::DEVICE_OUT_WIRED_HEADSET :
        {
            LOG_HARDWARE("open, C, HEADSET");
            mAfe_handle->Afe_Set_Stereo();
            SetSpeakerStatus(false);
            EnableSpeaker();
            if(mTty_Ctm == AUD_TTY_VCO)
            {
                LOG_HARDWARE("doModeRouting, C, HEADSET, TTY_VCO");
#ifdef ENABLE_EXT_DAC
                pLad->LAD_SetOutputDevice(LADOUT_SPEAKER1);
#else
#ifdef ALL_USING_VOICEBUFFER_INCALL
                pLad->LAD_SetOutputDevice(LADOUT_SPEAKER1);
#else
                pLad->LAD_SetOutputDevice(LADOUT_SPEAKER2);
#endif
#endif
                SetInputSource(LADIN_Microphone1);
                pLad->LAD_SetSpeechMode(SPH_MODE_NORMAL);
                mVolumeController->SetLadMicGain(Normal_Mic);
            }
            else if(mTty_Ctm == AUD_TTY_HCO)
            {
                LOG_HARDWARE("doModeRouting, C, HEADSET, TTY_HCO");
                pLad->LAD_SetOutputDevice(LADOUT_SPEAKER1);
                SetInputSource(LADIN_Microphone2);
                pLad->LAD_SetSpeechMode(SPH_MODE_NORMAL);
                mVolumeController->SetLadMicGain(TTY_CTM_Mic);
            }
            else if(mTty_Ctm == AUD_TTY_FULL)
            {
                LOG_HARDWARE("doModeRouting, C, HEADSET, TTY_FULL");
#ifdef ENABLE_EXT_DAC
                pLad->LAD_SetOutputDevice(LADOUT_SPEAKER1);
#else
#ifdef ALL_USING_VOICEBUFFER_INCALL
                pLad->LAD_SetOutputDevice(LADOUT_SPEAKER1);
#else
                pLad->LAD_SetOutputDevice(LADOUT_SPEAKER2);
#endif
#endif
                SetInputSource(LADIN_Microphone2);
                pLad->LAD_SetSpeechMode(SPH_MODE_EARPHONE);
                mVolumeController->SetLadMicGain(TTY_CTM_Mic);
            }
            else  // Headset or TTY-full case
            {
#ifdef ENABLE_EXT_DAC
                pLad->LAD_SetOutputDevice(LADOUT_SPEAKER1);
#else
#ifdef ALL_USING_VOICEBUFFER_INCALL
                pLad->LAD_SetOutputDevice(LADOUT_SPEAKER1);
#else
                pLad->LAD_SetOutputDevice(LADOUT_SPEAKER2);
#endif
#endif
                SetInputSource(LADIN_Microphone2);
                pLad->LAD_SetSpeechMode(SPH_MODE_EARPHONE);
                mVolumeController->SetLadMicGain(Headset_Mic);
            }
            break;
        }
        case android_audio_legacy::AudioSystem::DEVICE_OUT_WIRED_HEADPHONE :
        {
            LOG_HARDWARE("open, C, HEADPHONE");
            if(mTty_Ctm == AUD_TTY_VCO)
            {
                LOG_HARDWARE("doModeRouting, C, HEADPHONE, TTY_VCO");
#ifdef ENABLE_EXT_DAC
                pLad->LAD_SetOutputDevice(LADOUT_SPEAKER1);
#else
#ifdef ALL_USING_VOICEBUFFER_INCALL
                pLad->LAD_SetOutputDevice(LADOUT_SPEAKER1);
#else
                pLad->LAD_SetOutputDevice(LADOUT_SPEAKER2);
#endif
#endif
                SetInputSource(LADIN_Microphone1);
                pLad->LAD_SetSpeechMode(SPH_MODE_NORMAL);
                mVolumeController->SetLadMicGain(Normal_Mic);
            }
            else if(mTty_Ctm == AUD_TTY_HCO)
            {
                LOG_HARDWARE("doModeRouting, C, HEADPHONE, TTY_HCO");
                pLad->LAD_SetOutputDevice(LADOUT_SPEAKER1);
                SetInputSource(LADIN_Microphone2);
                pLad->LAD_SetSpeechMode(SPH_MODE_NORMAL);
                mVolumeController->SetLadMicGain(TTY_CTM_Mic);
            }
            else if(mTty_Ctm == AUD_TTY_FULL)
            {
                LOG_HARDWARE("doModeRouting, C, HEADPHONE, TTY_FULL");
#ifdef ENABLE_EXT_DAC
                pLad->LAD_SetOutputDevice(LADOUT_SPEAKER1);
#else
#ifdef ALL_USING_VOICEBUFFER_INCALL
                pLad->LAD_SetOutputDevice(LADOUT_SPEAKER1);
#else
                pLad->LAD_SetOutputDevice(LADOUT_SPEAKER2);
#endif
#endif
                SetInputSource(LADIN_Microphone2);
                pLad->LAD_SetSpeechMode(SPH_MODE_EARPHONE);
                mVolumeController->SetLadMicGain(TTY_CTM_Mic);
            }
            else
            {
                mAfe_handle->Afe_Set_Stereo();
#ifdef ENABLE_EXT_DAC
                pLad->LAD_SetOutputDevice(LADOUT_SPEAKER1);
#else
#ifdef ALL_USING_VOICEBUFFER_INCALL
                pLad->LAD_SetOutputDevice(LADOUT_SPEAKER1);
#else
                pLad->LAD_SetOutputDevice(LADOUT_SPEAKER2);
#endif
#endif
                SetInputSource(LADIN_Microphone1);
                pLad->LAD_SetSpeechMode(SPH_MODE_EARPHONE);
                mVolumeController->SetLadMicGain(Handfree_Mic);
                SetSpeakerStatus(false); // disable speaker
                EnableSpeaker();
            }
            break;
        }
        case android_audio_legacy::AudioSystem::DEVICE_OUT_BLUETOOTH_SCO:
        case android_audio_legacy::AudioSystem::DEVICE_OUT_BLUETOOTH_SCO_HEADSET:
        case android_audio_legacy::AudioSystem::DEVICE_OUT_BLUETOOTH_SCO_CARKIT:
        {
            LOG_HARDWARE("open, C, BT_SCO");
            pLad->LAD_SetOutputDevice(LADOUT_BTOut);
            SetInputSource(LADIN_BTIn);
            pLad->LAD_SetSpeechMode(SPH_MODE_BT_EARPHONE);
            break;
        }
        case android_audio_legacy::AudioSystem::DEVICE_OUT_SPEAKER:
        {
            LOG_HARDWARE("open, C, SPEAKER");
#ifdef ENABLE_STEREO_SPEAKER
            mAfe_handle->Afe_Set_Stereo();
#else
            mAfe_handle->Afe_Set_Mono();
#endif

#ifdef ENABLE_EXT_DAC
            pLad->LAD_SetOutputDevice(LADOUT_SPEAKER1);
#else
#ifdef ALL_USING_VOICEBUFFER_INCALL
            pLad->LAD_SetOutputDevice(LADOUT_SPEAKER1);
#else
            pLad->LAD_SetOutputDevice(LADOUT_LOUDSPEAKER);
#endif
#endif
            SetInputSource(LADIN_Microphone1);
            pLad->LAD_SetSpeechMode(SPH_MODE_LOUDSPK);
            mVolumeController->SetLadMicGain(Handfree_Mic);
            SetSpeakerStatus(true); // set spaker status true
            //[ALPS0067166]Not to enable speaker before LAD_speech_on is acked.
            // Move to ccco read thread.
            /*
            EnableSpeaker();
            */
            //~By
            break;
        }
        case android_audio_legacy::AudioSystem::DEVICE_OUT_EARPIECE:
        {
            LOG_HARDWARE("open, C, EARPIECE");
            pLad->LAD_SetOutputDevice(LADOUT_SPEAKER1);
            SetInputSource(LADIN_Microphone1);
            pLad->LAD_SetSpeechMode(SPH_MODE_NORMAL);
            mVolumeController->SetLadMicGain(Normal_Mic);
            SetSpeakerStatus(false); // disable speaker
#ifdef EARPIECE_CONTROL
            SetEarpieceStatus(true);
            EnableEarpiece();
#endif
            break;
        }
        case android_audio_legacy::AudioSystem::DEVICE_OUT_AUX_DIGITAL:
        {
            LOGW("open, C, AUX_DIGITAL");
            SetSpeakerStatus(false); // set spaker status false
            EnableSpeaker();
            if(mStreamHandler->GetSrcRunningNumber() > 0)
            {
#ifdef FM_ANALOG_OUT_SUPPORT
                mAnaReg->AnalogAFE_Open(AUDIO_PATH);
#endif
                mAfe_handle->Afe_Set_FmTx(mI2STxStatus,1);
            }
            else
            {
                mAfe_handle->Afe_Set_FmTx(mI2STxStatus,0);
            }
            break;
        }
        case android_audio_legacy::AudioSystem::DEVICE_OUT_HDMI_AUD:
        {
            LOGD("open, C, HDMI_Audio(disable)");
            SetHDMIAudioConnect(false);
            break;
        }
        default:
        {
            LOG_HARDWARE("doModeRouting, C, not specified, set EARPIECE");
            pLad->LAD_SetOutputDevice(LADOUT_SPEAKER1);
            SetInputSource(LADIN_Microphone1);
            pLad->LAD_SetSpeechMode(SPH_MODE_NORMAL);
            mVolumeController->SetLadMicGain(Normal_Mic);
            SetSpeakerStatus(false); // disable speaker
            break;
        }
        }
        LOG_HARDWARE("doModeRouting, LAD_Speech_On");  // open speech function

        // During phone call, Disable AP side to use audio HW.
        // Modem side would control Audio HW for phone call.
        if(mStreamHandler->GetSrcBlockRunning())
        {
            mAfe_handle->Afe_DL_Stop(AFE_MODE_DAC);
        }

        if(mVT_Speech == true)
        {
            this->Set_Recovery_VT(true);
            // if this is VT phone call, set the flag=1
            pLad->LAD_Speech_On(2);
            mMicMute_ModemStatus = false;
            mPcm2Way_Enable = true;
            pLad->LAD_PCM2WayOn();
            setMicMute(mMicMute);
        }
        else
        {
            this->Set_Recovery_Speech(true);
            pLad->LAD_Speech_On(0);
            mMicMute_ModemStatus = false;
            setMicMute(mMicMute);
        }
        if(mMode == android_audio_legacy::AudioSystem::MODE_IN_CALL)
        {
            mStreamHandler->SetLadPlayer(mMode);
        }
        if( (pLad->mTty_used_in_phone_call == false) && (mTty_Ctm != 0) )
        {
            // Unlock ModeMutex here, because the SetTtyCtmMode() would call doOutputDeviceRouting() function and
            // doOutputDeviceRouting() function also lock ModeMutex. It would cause deadlock.
            LOG_HARDWARE("Enable TTY when doModeRouting and set mTty_Ctm = %d", mTty_Ctm);
            pthread_mutex_unlock(&ModeMutex);
            SetTtyCtmMode(mTty_Ctm);
            pthread_mutex_lock(&ModeMutex);
        }

#ifdef  ENABLE_EXT_DAC
        mAudioCustomFunction->EXT_DAC_TurnOnSpeaker (EXT_DAC_LINEIN,AUDIO_EXP_DAC_BUF0);
        mAudioCustomFunction->EXT_DAC_TurnOnSpeaker (EXT_DAC_LINEIN,AUDIO_EXP_DAC_BUF1);
#endif
        break;
    }
    case android_audio_legacy::AudioSystem::MODE_IN_COMMUNICATION:
    {
        LOG_HARDWARE("doModeRouting, mMode = MODE_IN_COMMUNICATION");
        SetCommunicationStatus (true);
        SwitchAudioClock(true);
        switch(Route_device)
        {
        case android_audio_legacy::AudioSystem::DEVICE_OUT_WIRED_HEADSET :
        {
            LOG_HARDWARE("open, CM, HEADSET");
            mAfe_handle->Afe_Set_Stereo();
            SetSpeakerStatus(false);
            EnableSpeaker();
            mAnaReg->AnalogAFE_Close(AUDIO_PATH);
            SetInputSource(LADIN_Microphone2);
            if(mStreamHandler->GetSrcBlockRunning ())
            {
                mAnaReg->AnalogAFE_Open(AUDIO_PATH);
            }
            break;
        }
        case android_audio_legacy::AudioSystem::DEVICE_OUT_WIRED_HEADPHONE :
        {
            LOG_HARDWARE("open, CM, HEADPHONE");
            mAfe_handle->Afe_Set_Stereo();
            SetSpeakerStatus(false);
            EnableSpeaker();
            mAnaReg->AnalogAFE_Close(AUDIO_PATH);
            if(mStreamHandler->GetSrcBlockRunning ())
            {
                mAnaReg->AnalogAFE_Open(AUDIO_PATH);
            }
            break;
        }
        case android_audio_legacy::AudioSystem::DEVICE_OUT_BLUETOOTH_SCO:
        case android_audio_legacy::AudioSystem::DEVICE_OUT_BLUETOOTH_SCO_HEADSET:
        case android_audio_legacy::AudioSystem::DEVICE_OUT_BLUETOOTH_SCO_CARKIT:
        {
            LOG_HARDWARE("open, CM, BT_SCO");
#ifdef ENABLE_VOIP_BTSCO
            mAfe_handle->Afe_Set_Mono();
            SetSpeakerStatus(false); // set spaker status false
            EnableSpeaker();
            if(!(mPreviousMode==android_audio_legacy::AudioSystem::MODE_IN_CALL||mMode==android_audio_legacy::AudioSystem::MODE_IN_CALL))
            {
                mStreamHandler->InputStreamLock();
            }
            pLad->LAD_SetOutputDevice(LADOUT_BTOut);
            SetInputSource(LADIN_BTIn);
            pLad->LAD_SetSpeechMode(SPH_MODE_BT_EARPHONE);
            if(!(mPreviousMode==android_audio_legacy::AudioSystem::MODE_IN_CALL||mMode==android_audio_legacy::AudioSystem::MODE_IN_CALL))
            {
                mStreamHandler->InputStreamUnLock();
            }
            LOGD("SetOutputStreamToBT");
            mStreamHandler->SetOutputStreamToBT();
#endif
            break;
        }
        case android_audio_legacy::AudioSystem::DEVICE_OUT_SPEAKER:
        {
            LOG_HARDWARE("open, CM, SPEAKER");
#ifdef ENABLE_STEREO_SPEAKER
            mAfe_handle->Afe_Set_Stereo();
#else
            mAfe_handle->Afe_Set_Mono();
#endif
            SetSpeakerStatus(false); // set spaker status false
            EnableSpeaker();
            mAnaReg->AnalogAFE_Close(AUDIO_PATH);
            SetInputSource(LADIN_Microphone1);
            if(mStreamHandler->GetSrcBlockRunning ())
            {
                mAnaReg->AnalogAFE_Open(AUDIO_PATH);
            }
            SetSpeakerStatus(true); // set spaker status true
            if(mStreamHandler->GetSrcBlockRunning() > 0 )
            {
                EnableSpeaker();// open speaker
            }
            //judge if fm enable situation
            else if(GetFmRxStatus ())
            {
                SetFmSpeaker ();
            }
            else if(GetMatvAnalogEnable ())
            {
                SetMatvSpeaker();
            }
            break;
        }
        case android_audio_legacy::AudioSystem::DEVICE_OUT_AUX_DIGITAL:
        {
            LOG_HARDWARE("open, CM, AUX_DIGITAL");
            SetSpeakerStatus(false); // set spaker status false
            EnableSpeaker();
            if(mStreamHandler->GetSrcRunningNumber() > 0)
            {
#ifdef FM_ANALOG_OUT_SUPPORT
                mAnaReg->AnalogAFE_Open(AUDIO_PATH);
#endif
                mAfe_handle->Afe_Set_FmTx(mI2STxStatus,1);
            }
            else
            {
                mAfe_handle->Afe_Set_FmTx(mI2STxStatus,0);
            }
            break;
        }
        case android_audio_legacy::AudioSystem::DEVICE_OUT_EARPIECE:
        {
            LOG_HARDWARE("open, CM, EARPIECE");
            mAfe_handle->Afe_Set_Mono();
            SetSpeakerStatus(false);
            EnableSpeaker();
            mAnaReg->AnalogAFE_Open(VOICE_PATH);
#ifdef EARPIECE_CONTROL
            SetEarpieceStatus(true);
            EnableEarpiece();
#endif
            break;
        }
        case android_audio_legacy::AudioSystem::DEVICE_OUT_HDMI_AUD:
        {
            LOGD("open, CM, HDMI_Audio(diable)");
            SetHDMIAudioConnect(false);
            break;
        }
        default:
            LOG_HARDWARE("doModeRouting, CM, mMode not specidied");
        }
        // here to set mic_type
        setMicMute(mMicMute);
        break;
    }
    default:
    {
        LOG_HARDWARE("doModeRouting,  mMode no switch");
        break;
    }
    }
    //LOG_HARDWARE("AudioYusuHardware::CheckPinmux");
    //CheckPinmux(mPreviousMode,mMode); // change pinmux

    // check if need reopen record
    if(mStreamHandler->SetOpenRec(mPreviousMode,mMode)==true)
    {
        LOG_HARDWARE("doModeRouting, SetOpenRec");
    }
    else
    {
        LOGW("SetOpenRec return false");
    }

    if(mMode==android_audio_legacy::AudioSystem::MODE_IN_CALL && GetVmFlag()==true)
    {
        LOGD("Enable_Modem_Record true");
        Enable_Modem_Record(true);
    }

    //adjust volume for current mode and routes
    if(mMode == android_audio_legacy::AudioSystem::MODE_IN_CALL || mMode == android_audio_legacy::AudioSystem::MODE_IN_COMMUNICATION)
    {
        mVolumeController->setVoiceVolume(mVolumeController->getVoiceVolume(),mMode,Route_device);
    }
    else
    {
        mVolumeController->setMasterVolume(mVolumeController->getMasterVolume(),mMode,Route_device);
    }

MODE_CHANGE_EXIT:

#ifdef HEADPHONE_CONTROL
		if(Route_device == android_audio_legacy::AudioSystem::DEVICE_OUT_WIRED_HEADSET ||
			Route_device == android_audio_legacy::AudioSystem::DEVICE_OUT_WIRED_HEADPHONE )
		{
			SetHeadPhoneStatus(true);
			EnableHeadPhone();
		}
		if(Route_device ==  android_audio_legacy::AudioSystem::DEVICE_OUT_SPEAKER &&     //openspeaker
			( mOuputForceuse[mMode] == android_audio_legacy::AudioSystem::FORCE_SPEAKER) && // force use
			(mMode != android_audio_legacy::AudioSystem::MODE_IN_CALL && mMode != android_audio_legacy::AudioSystem::MODE_IN_COMMUNICATION) && // not voice force to speaker
			IsHeadsetPlugin(mMode))                                  // headset is open
		{
			LOG_HARDWARE("open speaker and headphone");
			SetHeadPhoneStatus(true);
			EnableHeadPhone();
		}
#endif

#ifndef HEADPHONE_CONTROL
#ifdef ALL_USING_VOICEBUFFER_INCALL
    SetHeadPhoneStatus(true);
    EnableHeadPhone();
#endif
#endif
    pLad->mHeadSetMessager->SetHeadSetState(mMode); // tell headset driver status

    mPreviousMode = mMode;  //save mMode to mPreviousMode
    LOG_HARDWARE("-doModeRouting");
    ioctl(mFd,AUD_SET_CLOCK,0);
    return NO_ERROR;
}

status_t AudioYusuHardware::doOutputDeviceRouting()
{
    return doOutputDeviceRouting(true, true);
}

status_t AudioYusuHardware::doOutputDeviceRouting(bool checkPredevice, bool checknowDevice)
{
    LOG_HARDWARE("+doOutputDeviceRouting checkPredevice = %d checknowDevice = %d",checkPredevice,checknowDevice);
    ioctl(mFd,AUD_SET_CLOCK,1);

    int PreRoute_device = -1,Route_device = -1;
    pthread_mutex_lock(&ModeMutex);
    Route_device = Audio_Match_Output_device(mDevice,mMode,checknowDevice); // turn on this deivce
    PreRoute_device = Audio_Match_Output_device(mPredevice,mMode,checkPredevice); // turn off this device
    LOG_HARDWARE("doOutputDeviceRouting PreRoute_device=0x%x, Route_device=0x%x, mMode=0x%x", PreRoute_device, Route_device, mMode);

    if(PreRoute_device == Route_device)
    {
        LOG_HARDWARE("doOutputDeviceRouting PreRoute_device == Route_device");
        //goto NOT_ROUTING;
        //break;
    }

#ifdef HEADPHONE_CONTROL
	if(PreRoute_device == android_audio_legacy::AudioSystem::DEVICE_OUT_WIRED_HEADSET   ||
		PreRoute_device == android_audio_legacy::AudioSystem::DEVICE_OUT_WIRED_HEADPHONE)
	{
		SetHeadPhoneStatus(false);
		EnableHeadPhone();
	}
	if(PreRoute_device ==  android_audio_legacy::AudioSystem::DEVICE_OUT_SPEAKER &&
		( mOuputForceuse[mMode] == android_audio_legacy::AudioSystem::FORCE_SPEAKER) &&
		IsHeadsetPlugin(mMode))
	{
		LOG_HARDWARE("close speaker and headphone");
		SetHeadPhoneStatus(false);
		EnableHeadPhone();
	}
#endif

    switch(mMode)
    {
    case android_audio_legacy::AudioSystem::MODE_NORMAL:
    case android_audio_legacy::AudioSystem::MODE_RINGTONE:
    {
        switch(PreRoute_device)  // previous devices ,close
        {
        case android_audio_legacy::AudioSystem::DEVICE_OUT_WIRED_HEADSET :
        case android_audio_legacy::AudioSystem::DEVICE_OUT_WIRED_HEADPHONE :
        {
            LOG_HARDWARE("close, N/R, HEADSET, HEADPHONE ");
            //SetSpeakerStatus(false);
           // EnableSpeaker();
            break;
        }
        case android_audio_legacy::AudioSystem::DEVICE_OUT_BLUETOOTH_SCO:
        case android_audio_legacy::AudioSystem::DEVICE_OUT_BLUETOOTH_SCO_HEADSET:
        case android_audio_legacy::AudioSystem::DEVICE_OUT_BLUETOOTH_SCO_CARKIT:
        {
            pLad->LAD_SetSpeechMode(SPH_MODE_NORMAL);
            break;
        }
        case android_audio_legacy::AudioSystem::DEVICE_OUT_SPEAKER:
        {
            LOG_HARDWARE("close, N/R, SPEAKER ");
            SetSpeakerStatus(false);
            if(!(mRoutingDevices & android_audio_legacy::AudioSystem::DEVICE_OUT_SPEAKER))
            {
                EnableSpeaker();
            }
#ifdef HEADPHONE_CONTROL
#endif
            break;
        }
        case android_audio_legacy::AudioSystem::DEVICE_OUT_EARPIECE:
        {
            LOGD("close, N/R, EARPIECE");
#ifdef EARPIECE_CONTROL
            SetEarpieceStatus(false);
            EnableEarpiece();
#endif
            mAnaReg->AnalogAFE_Close(AUDIO_PATH);
            break;
        }
        case android_audio_legacy::AudioSystem::DEVICE_OUT_AUX_DIGITAL:
        {
            //FM TX Close
            LOGD("close, N, FM Tx");
            SetSpeakerStatus(false);
            EnableSpeaker();
            mAnaReg->AnalogAFE_Close(NONE_PATH);
            if(mStreamHandler->GetSrcBlockRunning ())
            {
                //connect dac
                mAfe_handle->Afe_Set_FmTx(false,1);
            }
            else
            {
                mAfe_handle->Afe_Set_FmTx(false,0);
            }
            break;
        }
        case android_audio_legacy::AudioSystem::DEVICE_OUT_HDMI_AUD:
        {
            LOGD("close, N, HDMI_Audio");
            SetSpeakerStatus(false);
            EnableSpeaker();
            mAnaReg->AnalogAFE_Close(NONE_PATH);
            SetHDMIAudioConnect(false);
            break;
        }
        default:
        {
            LOG_HARDWARE("close, N/R, Pre_dev not specified ");
        }
        }
        switch(Route_device)  // now  devices , open
        {
        case android_audio_legacy::AudioSystem::DEVICE_OUT_WIRED_HEADSET :
        {
            LOG_HARDWARE("open, N/R, HEADSET");
            SetInputSource(LADIN_Microphone2);
            mAfe_handle->Afe_Set_Stereo();
            if(GetAnalogLineinEnable())
            {
                mAnaReg->AnalogAFE_Open(FM_PATH_STEREO);
            }
            else if(mStreamHandler->GetSrcBlockRunning ()
                    && (PreRoute_device != android_audio_legacy::AudioSystem::DEVICE_OUT_SPEAKER))//When route from speaker, turn on audio path will cause one pop noise.
            {
                mAnaReg->AnalogAFE_Open(AUDIO_PATH);
            }
            SetFmSpeaker ();
            break;
        }
        case android_audio_legacy::AudioSystem::DEVICE_OUT_WIRED_HEADPHONE :
        {
            LOG_HARDWARE("open, N/R, HEADPHONE");
            SetInputSource(LADIN_Microphone1);
            mAfe_handle->Afe_Set_Stereo();
            if(GetAnalogLineinEnable())
            {
                mAnaReg->AnalogAFE_Open(FM_PATH_STEREO);
            }
            else if(mStreamHandler->GetSrcBlockRunning ()
                    && (PreRoute_device != android_audio_legacy::AudioSystem::DEVICE_OUT_SPEAKER))//When route from speaker, turn on audio path will cause one pop noise.
            {
                mAnaReg->AnalogAFE_Open(AUDIO_PATH);
            }
            SetFmSpeaker ();

            break;
        }
        case android_audio_legacy::AudioSystem::DEVICE_OUT_BLUETOOTH_SCO:
        case android_audio_legacy::AudioSystem::DEVICE_OUT_BLUETOOTH_SCO_HEADSET:
        case android_audio_legacy::AudioSystem::DEVICE_OUT_BLUETOOTH_SCO_CARKIT:
        {
            LOG_HARDWARE("open, N/R, BT_SCO");
            pLad->LAD_SetOutputDevice(LADOUT_BTOut);
            SetInputSource(LADIN_BTIn);
            pLad->LAD_SetSpeechMode(SPH_MODE_BT_EARPHONE);
            break;
        }
        case android_audio_legacy::AudioSystem::DEVICE_OUT_SPEAKER:
        {
            LOG_HARDWARE("open, N/R, SPEAKER");
#ifdef ENABLE_STEREO_SPEAKER
            mAfe_handle->Afe_Set_Stereo();
#else
            mAfe_handle->Afe_Set_Mono();
#endif
            SetSpeakerStatus(true); // set spaker status true
            SetInputSource(LADIN_Microphone1);
            if(GetAnalogLineinEnable() &&mOuputForceuse[mMode] == 0)
            {
                mAnaReg->AnalogAFE_Open(FM_PATH_STEREO);
            }
            else if(mStreamHandler->GetSrcBlockRunning ())
            {
                mAnaReg->AnalogAFE_Open(AUDIO_PATH);
            }
            if(mStreamHandler->GetSrcBlockRunning() > 0 || (GetAnalogLineinEnable()) )
            {
                /*if(mStreamHandler->GetSrcBlockRunning() > 0 && !GetMatvStatus() && !GetHDMIAudioStatus())
                {
                    usleep(300*1000);//to prevent TDD noise
                }*/
                EnableSpeaker();
            }
#ifdef HEADPHONE_CONTROL
#endif
            break;
        }
        case android_audio_legacy::AudioSystem::DEVICE_OUT_AUX_DIGITAL:
        {
            //FM TX Open
            LOGD("open, N/R, FM Tx");
            SetSpeakerStatus(false); // set spaker status false
            EnableSpeaker();
            if(mStreamHandler->GetSrcRunningNumber() > 0)
            {
#ifdef FM_ANALOG_OUT_SUPPORT
                mAnaReg->AnalogAFE_Open(AUDIO_PATH);
#endif
                mAfe_handle->Afe_Set_FmTx(mI2STxStatus,1);
            }
            else
            {
                mAfe_handle->Afe_Set_FmTx(mI2STxStatus,0);
            }
            break;
        }
        case android_audio_legacy::AudioSystem::DEVICE_OUT_EARPIECE:
        {
            LOGD("open, N/R, EARPIECE");
            mAfe_handle->Afe_Set_Mono();
            SetInputSource(LADIN_Microphone1);
            if(mStreamHandler->GetSrcBlockRunning ())
            {
                mAnaReg->AnalogAFE_Open(VOICE_PATH);
            }
#ifdef EARPIECE_CONTROL
            SetEarpieceStatus(true);
            EnableEarpiece();
#endif
			//ALPS00282730
			if(GetFmAnalogInEnable()&& (GetFmSpeakerStatus()||(!GetSpeakerStatus() && !IsHeadsetConnect())))
			{
				LOGD("for FM without headset case, enable FM");
				ForceEnableSpeaker();
			}

            break;
        }
        case android_audio_legacy::AudioSystem::DEVICE_OUT_HDMI_AUD:
        {
            LOGD("open, N/R, HDMI_Audio");
            SetSpeakerStatus(false);
            EnableSpeaker();
            SetHDMIAudioConnect(true);
            break;
        }
        default:
        {
            LOG_HARDWARE("doOutputDeviceRouting, N/R, Pre_dev not specified ");
            break;
        }
        }
        break;
    }
    case android_audio_legacy::AudioSystem::MODE_IN_CALL:  // this is for MODE_IN_CALL routing.
    {

// Modem side(MT657x) control limitation.
// If do output device change, need to speech off --> change device/mode --> speech on.
// --> Add for device routing path control.
//         SwitchAudioClock(true);
#if 1
        // need configure BGS off before speech off
        if(mMode == android_audio_legacy::AudioSystem::MODE_IN_CALL)
        {
//            mStreamHandler->ResetLadPlayer();
            mStreamHandler->ForceResetLadPlayer(mMode);
        }

#ifdef ENABLE_EXT_DAC
        mAudioCustomFunction->EXT_DAC_TurnOffSpeaker (EXT_DAC_LINEIN,AUDIO_EXP_DAC_BUF0);
        mAudioCustomFunction->EXT_DAC_TurnOffSpeaker (EXT_DAC_LINEIN,AUDIO_EXP_DAC_BUF1);
#endif

        if(mPcm2Way_Enable == true)
        {
            pLad->LAD_PCM2WayOff();
            mPcm2Way_Enable = false;
        }
        if(pLad->mTty_used_in_phone_call == true)
        {
            LOG_HARDWARE("doOutputDeviceRouting, LAD_TtyCtmOff()");
            //===> by
            pLad->LAD_TtyCtmOff();
            this->Set_Recovery_Tty(AUD_TTY_OFF);
            //<===
            //mTty_Ctm = AUD_TTY_OFF;
            //SetTtyCtmMode(AUD_TTY_OFF); //pLad->LAD_TtyCtmOff(); // turn off TTY-CTM
        }
        LOG_HARDWARE("doOutputDeviceRouting, LAD_Speech_Off()");
        pLad->LAD_Speech_Off();   // turn off SPEECH
#endif
// Add for device routing path control. <--

        switch(PreRoute_device)  // previous device , closeInputStream
        {
        case android_audio_legacy::AudioSystem::DEVICE_OUT_WIRED_HEADSET :
        case android_audio_legacy::AudioSystem::DEVICE_OUT_WIRED_HEADPHONE :
        {
            LOG_HARDWARE("close, C, DEVICE_OUT_WIRED_HEADSET");
            EnableSpeaker();
            break;
        }
        case android_audio_legacy::AudioSystem::DEVICE_OUT_BLUETOOTH_SCO:
        case android_audio_legacy::AudioSystem::DEVICE_OUT_BLUETOOTH_SCO_HEADSET:
        case android_audio_legacy::AudioSystem::DEVICE_OUT_BLUETOOTH_SCO_CARKIT:
        {
            LOG_HARDWARE("close, C, BT_SCO  ");
            pLad->LAD_SetSpeechMode(SPH_MODE_NORMAL);
            break;
        }
        case android_audio_legacy::AudioSystem::DEVICE_OUT_SPEAKER:
        {
            LOG_HARDWARE("close, C, DEVICE_OUT_SPEAKER  ");
            SetSpeakerStatus(false); // set spaker status true
            EnableSpeaker();
            break;
        }
        case android_audio_legacy::AudioSystem::DEVICE_OUT_EARPIECE:
        {
            LOG_HARDWARE("close, C, DEVICE_OUT_EARPIECE ");
#ifdef EARPIECE_CONTROL
            SetEarpieceStatus(false);
            EnableEarpiece();
#endif
            break;
        }
        case android_audio_legacy::AudioSystem::DEVICE_OUT_AUX_DIGITAL:
        {
            LOGD("close, C, FM Tx");
            SetSpeakerStatus(false);
            EnableSpeaker();
            mAnaReg->AnalogAFE_Close(NONE_PATH);
            if(mStreamHandler->GetSrcBlockRunning ())
            {
                //connect dac
                mAfe_handle->Afe_Set_FmTx(false,1);
            }
            else
            {
                mAfe_handle->Afe_Set_FmTx(false,0);
            }
            break;
        }
        case android_audio_legacy::AudioSystem::DEVICE_OUT_HDMI_AUD:
        {
            LOGD("close, C, HDMI_Audio");
            SetSpeakerStatus(false);
            EnableSpeaker();
            mAnaReg->AnalogAFE_Close(NONE_PATH);
            SetHDMIAudioConnect(false);
            break;
        }
        default:
        {
            LOG_HARDWARE("IN_CALL PreRoute_device not specified");
            break;
        }
        }

        switch(Route_device)  // now devices , openInputStream
        {
        case android_audio_legacy::AudioSystem::DEVICE_OUT_WIRED_HEADSET :
        {
            LOG_HARDWARE("open, IN_CALL, HEADSET");
            mAfe_handle->Afe_Set_Stereo();
            SetSpeakerStatus(false); // disable speaker

            if(mTty_Ctm == AUD_TTY_VCO)
            {
                LOG_HARDWARE("open, IN_CALL, HEADSET, TTY_VCO");
#ifdef ENABLE_EXT_DAC
                pLad->LAD_SetOutputDevice(LADOUT_SPEAKER1);
#else
                pLad->LAD_SetOutputDevice(LADOUT_SPEAKER2);
#endif
                SetInputSource(LADIN_Microphone1);
                pLad->LAD_SetSpeechMode(SPH_MODE_NORMAL);
                mVolumeController->SetLadMicGain(Normal_Mic);

                if(pLad->mTty_used_in_phone_call == true)
                {
                    // first: disable TTY-CTM
                    pLad->LAD_TtyCtmOff();
                    this->Set_Recovery_Tty(AUD_TTY_OFF);
                    // second: enable TTY-CTM again
                    pLad->LAD_TtyCtmOn(AUD_TTY_VCO);
                    this->Set_Recovery_Tty(AUD_TTY_VCO);
                }
            }
            else if(mTty_Ctm == AUD_TTY_HCO)
            {
                LOG_HARDWARE("open, IN_CALL, HEADSET, TTY_HCO");
                pLad->LAD_SetOutputDevice(LADOUT_SPEAKER1);
                SetInputSource(LADIN_Microphone2);
                pLad->LAD_SetSpeechMode(SPH_MODE_NORMAL);
                mVolumeController->SetLadMicGain(TTY_CTM_Mic);

                if(pLad->mTty_used_in_phone_call == true)
                {
                    // first: disable TTY-CTM
                    pLad->LAD_TtyCtmOff();
                    this->Set_Recovery_Tty(AUD_TTY_OFF);
                    // second: enable TTY-CTM again
                    pLad->LAD_TtyCtmOn(AUD_TTY_HCO);
                    this->Set_Recovery_Tty(AUD_TTY_HCO);
                }
            }
            else if(mTty_Ctm == AUD_TTY_FULL)
            {
                LOG_HARDWARE("open, IN_CALL, HEADSET, TTY_FULL");
#ifdef ENABLE_EXT_DAC
                pLad->LAD_SetOutputDevice(LADOUT_SPEAKER1);
#else
                pLad->LAD_SetOutputDevice(LADOUT_SPEAKER2);
#endif
                SetInputSource(LADIN_Microphone2);
                pLad->LAD_SetSpeechMode(SPH_MODE_EARPHONE);
                mVolumeController->SetLadMicGain(TTY_CTM_Mic);

                if(pLad->mTty_used_in_phone_call == true)
                {
                    // first: disable TTY-CTM
                    pLad->LAD_TtyCtmOff();
                    this->Set_Recovery_Tty(AUD_TTY_OFF);
                    // second: enable TTY-CTM again
                    pLad->LAD_TtyCtmOn(AUD_TTY_FULL);
                    this->Set_Recovery_Tty(AUD_TTY_FULL);
                }
            }
            else
            {
#ifdef ALL_USING_VOICEBUFFER_INCALL
                pLad->LAD_SetOutputDevice(LADOUT_SPEAKER1);
#else
                pLad->LAD_SetOutputDevice(LADOUT_SPEAKER2);
#endif
                SetInputSource(LADIN_Microphone2);
                pLad->LAD_SetSpeechMode(SPH_MODE_EARPHONE);
                mVolumeController->SetLadMicGain(Headset_Mic);
            }
            break;
        }
        case android_audio_legacy::AudioSystem::DEVICE_OUT_WIRED_HEADPHONE:
        {
            LOG_HARDWARE("open, IN_CALL, HEADPHONE");

            if(mTty_Ctm == AUD_TTY_VCO)
            {
                LOG_HARDWARE("open, IN_CALL, HEADPHONE, TTY_VCO");
#ifdef ENABLE_EXT_DAC
                pLad->LAD_SetOutputDevice(LADOUT_SPEAKER1);
#else
#ifdef ALL_USING_VOICEBUFFER_INCALL
                pLad->LAD_SetOutputDevice(LADOUT_SPEAKER1);
#else
                pLad->LAD_SetOutputDevice(LADOUT_SPEAKER2);
#endif
#endif
                SetInputSource(LADIN_Microphone1);
                pLad->LAD_SetSpeechMode(SPH_MODE_NORMAL);
                mVolumeController->SetLadMicGain(Normal_Mic);

                if(pLad->mTty_used_in_phone_call == true)
                {
                    // first: disable TTY-CTM
                    pLad->LAD_TtyCtmOff();
                    this->Set_Recovery_Tty(AUD_TTY_OFF);
                    // second: enable TTY-CTM again
                    pLad->LAD_TtyCtmOn(AUD_TTY_VCO);
                    this->Set_Recovery_Tty(AUD_TTY_VCO);
                }
            }
            else if(mTty_Ctm == AUD_TTY_HCO)
            {
                LOG_HARDWARE("open, IN_CALL, HEADPHONE, TTY_HCO");
                pLad->LAD_SetOutputDevice(LADOUT_SPEAKER1);
                SetInputSource(LADIN_Microphone2);
                pLad->LAD_SetSpeechMode(SPH_MODE_NORMAL);
                mVolumeController->SetLadMicGain(TTY_CTM_Mic);

                if(pLad->mTty_used_in_phone_call == true)
                {
                    // first: disable TTY-CTM
                    pLad->LAD_TtyCtmOff();
                    this->Set_Recovery_Tty(AUD_TTY_OFF);
                    // second: enable TTY-CTM again
                    pLad->LAD_TtyCtmOn(AUD_TTY_HCO);
                    this->Set_Recovery_Tty(AUD_TTY_HCO);
                }
            }
            else if(mTty_Ctm == AUD_TTY_FULL)
            {
                LOG_HARDWARE("open, IN_CALL, HEADPHONE, TTY_FULL");
#ifdef ENABLE_EXT_DAC
                pLad->LAD_SetOutputDevice(LADOUT_SPEAKER1);
#else
#ifdef ALL_USING_VOICEBUFFER_INCALL
                pLad->LAD_SetOutputDevice(LADOUT_SPEAKER1);
#else
                pLad->LAD_SetOutputDevice(LADOUT_SPEAKER2);
#endif
#endif
                SetInputSource(LADIN_Microphone2);
                pLad->LAD_SetSpeechMode(SPH_MODE_EARPHONE);
                mVolumeController->SetLadMicGain(TTY_CTM_Mic);
                if(pLad->mTty_used_in_phone_call == true)
                {
                    // first: disable TTY-CTM
                    pLad->LAD_TtyCtmOff();
                    this->Set_Recovery_Tty(AUD_TTY_OFF);
                    // second: enable TTY-CTM again
                    pLad->LAD_TtyCtmOn(AUD_TTY_FULL);
                    this->Set_Recovery_Tty(AUD_TTY_FULL);
                }
            }
            else
            {
                mAfe_handle->Afe_Set_Stereo( );
#ifdef ENABLE_EXT_DAC
                pLad->LAD_SetOutputDevice(LADOUT_SPEAKER1);
#else
#ifdef ALL_USING_VOICEBUFFER_INCALL
                pLad->LAD_SetOutputDevice(LADOUT_SPEAKER1);
#else
                pLad->LAD_SetOutputDevice(LADOUT_SPEAKER2);
#endif
#endif
                SetInputSource(LADIN_Microphone1);
                pLad->LAD_SetSpeechMode(SPH_MODE_EARPHONE);
                mVolumeController->SetLadMicGain(Handfree_Mic);
            }
            SetSpeakerStatus(false); // set spaker status false
            EnableSpeaker();
            break;
        }
        case android_audio_legacy::AudioSystem::DEVICE_OUT_BLUETOOTH_SCO:
        case android_audio_legacy::AudioSystem::DEVICE_OUT_BLUETOOTH_SCO_HEADSET:
        case android_audio_legacy::AudioSystem::DEVICE_OUT_BLUETOOTH_SCO_CARKIT:
        {
            LOG_HARDWARE("open, IN_CALL, BT_SCO");
            pLad->LAD_SetOutputDevice(LADOUT_BTOut);
            SetInputSource(LADIN_BTIn);
            pLad->LAD_SetSpeechMode(SPH_MODE_BT_EARPHONE);
            break;
        }
        case android_audio_legacy::AudioSystem::DEVICE_OUT_SPEAKER:
        {
            LOG_HARDWARE("open, IN_CALL, SPEAKER");

            // if pre_dev = HEADSET --> In-Call and switch from headset to loud speaker
            if(PreRoute_device == android_audio_legacy::AudioSystem::DEVICE_OUT_WIRED_HEADSET)
            {
                if(mTty_Ctm == AUD_TTY_VCO)
                {
                    LOG_HARDWARE("open, IN_CALL, SPEAKER, TTY_VCO");
                    mAfe_handle->Afe_Set_Mono();
#ifdef ENABLE_EXT_DAC
                    pLad->LAD_SetOutputDevice(LADOUT_SPEAKER1);
#else
#ifdef ALL_USING_VOICEBUFFER_INCALL
                    pLad->LAD_SetOutputDevice(LADOUT_SPEAKER1);
#else
                    pLad->LAD_SetOutputDevice(LADOUT_SPEAKER2);
#endif
#endif
                    SetInputSource(LADIN_Microphone1);
                    pLad->LAD_SetSpeechMode(SPH_MODE_NORMAL);
                    mVolumeController->SetLadMicGain(Handfree_Mic);

                    if(pLad->mTty_used_in_phone_call == true)
                    {
                        // first: disable TTY-CTM
                        pLad->LAD_TtyCtmOff();
                        this->Set_Recovery_Tty(AUD_TTY_OFF);
                        // second: enable TTY-CTM again
                        pLad->LAD_TtyCtmOn(AUD_TTY_VCO);
                        this->Set_Recovery_Tty(AUD_TTY_VCO);
                    }
                }
                else if(mTty_Ctm == AUD_TTY_HCO)
                {
                    LOG_HARDWARE("open, IN_CALL, SPEAKER, TTY_HCO");
                    mAfe_handle->Afe_Set_Mono();
#ifdef ENABLE_EXT_DAC
                    pLad->LAD_SetOutputDevice(LADOUT_SPEAKER1);
#else
#ifdef ALL_USING_VOICEBUFFER_INCALL
                    pLad->LAD_SetOutputDevice(LADOUT_SPEAKER1);
#else
                    pLad->LAD_SetOutputDevice(LADOUT_LOUDSPEAKER);
#endif
#endif
                    SetInputSource(LADIN_Microphone2);
                    pLad->LAD_SetSpeechMode(SPH_MODE_LOUDSPK);
                    mVolumeController->SetLadMicGain(TTY_CTM_Mic);
                    SetSpeakerStatus(true); // set spaker status true
                    EnableSpeaker();

                    if(pLad->mTty_used_in_phone_call == true)
                    {
                        // first: disable TTY-CTM
                        pLad->LAD_TtyCtmOff();
                        this->Set_Recovery_Tty(AUD_TTY_OFF);
                        // second: enable TTY-CTM again
                        pLad->LAD_TtyCtmOn(AUD_TTY_HCO);
                        this->Set_Recovery_Tty(AUD_TTY_HCO);
                    }
                }
                else if(mTty_Ctm == AUD_TTY_FULL)
                {
                    LOG_HARDWARE("open, IN_CALL, SPEAKER, TTY_FULL");
                    mAfe_handle->Afe_Set_Mono();
#ifdef ENABLE_EXT_DAC
                    pLad->LAD_SetOutputDevice(LADOUT_SPEAKER1);
#else
#ifdef ALL_USING_VOICEBUFFER_INCALL
                    pLad->LAD_SetOutputDevice(LADOUT_SPEAKER1);
#else
                    pLad->LAD_SetOutputDevice(LADOUT_SPEAKER2);
#endif
#endif
                    SetInputSource(LADIN_Microphone2);
                    pLad->LAD_SetSpeechMode(SPH_MODE_EARPHONE);
                    mVolumeController->SetLadMicGain(TTY_CTM_Mic);

                    if(pLad->mTty_used_in_phone_call == true)
                    {
                        // first: disable TTY-CTM
                        pLad->LAD_TtyCtmOff();
                        this->Set_Recovery_Tty(AUD_TTY_OFF);
                        // second: enable TTY-CTM again
                        pLad->LAD_TtyCtmOn(AUD_TTY_FULL);
                        this->Set_Recovery_Tty(AUD_TTY_FULL);
                    }
                }
                else
                {
                    mAfe_handle->Afe_Set_Mono();
#ifdef ENABLE_EXT_DAC
                    pLad->LAD_SetOutputDevice(LADOUT_SPEAKER1);
#else
#ifdef ALL_USING_VOICEBUFFER_INCALL
                    pLad->LAD_SetOutputDevice(LADOUT_SPEAKER1);
#else
                    pLad->LAD_SetOutputDevice(LADOUT_LOUDSPEAKER);
#endif
#endif
                    SetInputSource(LADIN_Microphone1);
                    pLad->LAD_SetSpeechMode(SPH_MODE_LOUDSPK);
                    mVolumeController->SetLadMicGain(Handfree_Mic);
                    SetSpeakerStatus(true); // set spaker status true
                    EnableSpeaker();
                }
            }
            else
            {
                mAfe_handle->Afe_Set_Mono();
#ifdef ENABLE_EXT_DAC
                pLad->LAD_SetOutputDevice(LADOUT_SPEAKER1);
#else
#ifdef ALL_USING_VOICEBUFFER_INCALL
                pLad->LAD_SetOutputDevice(LADOUT_SPEAKER1);
#else
                pLad->LAD_SetOutputDevice(LADOUT_LOUDSPEAKER);
#endif
#endif
                SetInputSource(LADIN_Microphone1);
                pLad->LAD_SetSpeechMode(SPH_MODE_LOUDSPK);
                mVolumeController->SetLadMicGain(Handfree_Mic);
                /*
                SetSpeakerStatus(true); // set spaker status true
                EnableSpeaker();
                */
                //Move to ccci read thread.
                SetSpeakerStatus(false); // set spaker status true
                EnableSpeaker();
                m_Speaker_Enable_during_InCall = true;

            }
            break;
        }
        case android_audio_legacy::AudioSystem::DEVICE_OUT_EARPIECE:
        {
            LOG_HARDWARE("open, IN_CALL, EARPIECE");
            pLad->LAD_SetOutputDevice(LADOUT_SPEAKER1);
            SetInputSource(LADIN_Microphone1);
            pLad->LAD_SetSpeechMode(SPH_MODE_NORMAL);
            mVolumeController->SetLadMicGain(Normal_Mic);

            if(this->GetAnalogLineinEnable() == true)
            {
                LOGD("open, IN_CALL, EARPIECE, AnalogAFE_Open(VOICE_PATH)");
                mAnaReg->AnalogAFE_Open(VOICE_PATH);
            }
            SetSpeakerStatus(false); // set spaker status false
#ifdef EARPIECE_CONTROL
            SetEarpieceStatus(true);
            EnableEarpiece();
#endif
            break;
        }
        case android_audio_legacy::AudioSystem::DEVICE_OUT_AUX_DIGITAL:
        {
            LOGW("open, C, FM Tx ");
            //FM TX Open
            SetSpeakerStatus(false); // set spaker status false
            EnableSpeaker();
            if(mStreamHandler->GetSrcRunningNumber() > 0)
            {
#ifdef FM_ANALOG_OUT_SUPPORT
                mAnaReg->AnalogAFE_Open(AUDIO_PATH);
#endif
                mAfe_handle->Afe_Set_FmTx(mI2STxStatus,1);
            }
            else
            {
                mAfe_handle->Afe_Set_FmTx(mI2STxStatus,0);
            }
            break;
        }
        case android_audio_legacy::AudioSystem::DEVICE_OUT_HDMI_AUD:
        {
            LOGD("open, C, HDMI_Audio(disable)");
            SetHDMIAudioConnect(false);
            break;
        }
        default:
        {
            LOG_HARDWARE("doOutputDeviceRouting, IN_CALL, Route_device not specified");
            break;
        }
        }

// Modem side(MT657x) control limitation.
// If do output device change, need to speech off --> change device/mode --> speech on
// --> Add for device routing path control.
        LOG_HARDWARE("doOutputDeviceRouting, LAD_Speech_On");  // open sppech function
#if 1
        if(mVT_Speech == true)
        {
            this->Set_Recovery_VT(true);
            // if this is VT phone call, set the flag=1
            pLad->LAD_Speech_On(2);
            mMicMute_ModemStatus = false;
            mPcm2Way_Enable = true;
            pLad->LAD_PCM2WayOn();
            setMicMute(mMicMute);
        }
        else
        {
            this->Set_Recovery_Speech(true);
            pLad->LAD_Speech_On(0);
            mMicMute_ModemStatus = false;
            setMicMute(mMicMute);
        }

#ifdef ENABLE_EXT_DAC
        mAudioCustomFunction->EXT_DAC_TurnOnSpeaker (EXT_DAC_LINEIN,AUDIO_EXP_DAC_BUF0);
        mAudioCustomFunction->EXT_DAC_TurnOnSpeaker (EXT_DAC_LINEIN,AUDIO_EXP_DAC_BUF1);
#endif

        if(mMode == android_audio_legacy::AudioSystem::MODE_IN_CALL)
        {
            mStreamHandler->SetLadPlayer(mMode);
        }

        if( (pLad->mTty_used_in_phone_call == false) && (mTty_Ctm != 0) )
        {
            // Unlock ModeMutex here, because the SetTtyCtmMode() would call doOutputDeviceRouting() function and
            // doOutputDeviceRouting() function also lock ModeMutex. It would cause deadlock.
            //pthread_mutex_unlock(&ModeMutex);
            //SetTtyCtmMode(mTty_Ctm);
            //pthread_mutex_lock(&ModeMutex);
            //===> by
            pLad->LAD_TtyCtmOn(mTty_Ctm);
            this->Set_Recovery_Tty(mTty_Ctm);
            //if(GetCtm4WayFlag() && VmThreadExit) Enable_Modem_Record(true); //: enable ctm-4way record after CTM on, also VM thread is no running

            //<===
        }
#endif
//         SwitchAudioClock(false);
// Add for device routing path control. <--
        break;
    }
    case android_audio_legacy::AudioSystem::MODE_IN_COMMUNICATION:
    {
        LOGD("doOutputDeviceRouting MODE_IN_COMMUNICATION");
        switch(PreRoute_device)  // previous devices ,close
        {
        case android_audio_legacy::AudioSystem::DEVICE_OUT_WIRED_HEADSET :
        case android_audio_legacy::AudioSystem::DEVICE_OUT_WIRED_HEADPHONE :
        {
            LOG_HARDWARE("close, CM, HEADSET, HEADPHONE ");
            SetSpeakerStatus(false);
            EnableSpeaker();
            break;
        }
        case android_audio_legacy::AudioSystem::DEVICE_OUT_BLUETOOTH_SCO:
        case android_audio_legacy::AudioSystem::DEVICE_OUT_BLUETOOTH_SCO_HEADSET:
        case android_audio_legacy::AudioSystem::DEVICE_OUT_BLUETOOTH_SCO_CARKIT:
        {
#ifdef ENABLE_VOIP_BTSCO
            mStreamHandler->InputStreamLock();
            pLad->LAD_SetSpeechMode(SPH_MODE_NORMAL);
            mStreamHandler->RestoreOutputStream();
            mStreamHandler->InputStreamUnLock();
#endif
            break;
        }
        case android_audio_legacy::AudioSystem::DEVICE_OUT_SPEAKER:
        {
            LOG_HARDWARE("close, CM, SPEAKER ");
            SetSpeakerStatus(false);
            EnableSpeaker();
            break;
        }
        case android_audio_legacy::AudioSystem::DEVICE_OUT_EARPIECE:
        {
            LOG_HARDWARE("close, CM, DEVICE_OUT_EARPIECE ");
#ifdef EARPIECE_CONTROL
            SetEarpieceStatus(false);
            EnableEarpiece();
#endif
            mAnaReg->AnalogAFE_Close(NONE_PATH);
            break;
        }
        case android_audio_legacy::AudioSystem::DEVICE_OUT_AUX_DIGITAL:
        {
            //FM TX Close
            LOGD("close, CM, FM Tx");
            SetSpeakerStatus(false);
            EnableSpeaker();
            mAnaReg->AnalogAFE_Close(NONE_PATH);
            if(mStreamHandler->GetSrcBlockRunning ())
            {
                //connect dac
                mAfe_handle->Afe_Set_FmTx(false,1);
            }
            else
            {
                mAfe_handle->Afe_Set_FmTx(false,0);
            }
            break;
        }
        case android_audio_legacy::AudioSystem::DEVICE_OUT_HDMI_AUD:
        {
            LOGD("close, CM, HDMI_Audio");
            SetSpeakerStatus(false);
            EnableSpeaker();
            mAnaReg->AnalogAFE_Close(NONE_PATH);
            SetHDMIAudioConnect(false);
            break;
        }
        default:
        {
            LOG_HARDWARE("close, CM, Pre_dev not specified ");
        }
        }

        switch(Route_device)  // now  devices , open
        {
        case android_audio_legacy::AudioSystem::DEVICE_OUT_WIRED_HEADSET :
        {
            LOG_HARDWARE("open, CM, HEADSET");
            SetInputSource(LADIN_Microphone2);
            mAfe_handle->Afe_Set_Stereo();
            if(mStreamHandler->GetSrcBlockRunning ())
                mAnaReg->AnalogAFE_Open(AUDIO_PATH);
            break;
        }
        case android_audio_legacy::AudioSystem::DEVICE_OUT_WIRED_HEADPHONE :
        {
            LOG_HARDWARE("open, CM, HEADPHONE");
            SetInputSource(LADIN_Microphone1);
            mAfe_handle->Afe_Set_Stereo();
            if(mStreamHandler->GetSrcBlockRunning ())
                mAnaReg->AnalogAFE_Open(AUDIO_PATH);
            break;
        }
        case android_audio_legacy::AudioSystem::DEVICE_OUT_BLUETOOTH_SCO:
        case android_audio_legacy::AudioSystem::DEVICE_OUT_BLUETOOTH_SCO_HEADSET:
        case android_audio_legacy::AudioSystem::DEVICE_OUT_BLUETOOTH_SCO_CARKIT:
        {
            LOG_HARDWARE("open, CM, BT_SCO");
#ifdef ENABLE_VOIP_BTSCO
            mStreamHandler->InputStreamLock();
            pLad->LAD_SetSpeechMode(SPH_MODE_BT_EARPHONE);
            SetInputSource(LADIN_BTIn);
            mStreamHandler->InputStreamUnLock();
            usleep(10*1000);
            LOGD("mStreamHandler->SetOutputStreamToBT()");
            mStreamHandler->SetOutputStreamToBT();
#endif
            break;
        }
        case android_audio_legacy::AudioSystem::DEVICE_OUT_SPEAKER:
        {
            LOG_HARDWARE("open, CM, SPEAKER");
#ifdef ENABLE_STEREO_SPEAKER
            mAfe_handle->Afe_Set_Stereo();
#else
            mAfe_handle->Afe_Set_Mono();
#endif
            SetSpeakerStatus(true); // set spaker status true
            SetInputSource(LADIN_Microphone1);
            if(mStreamHandler->GetSrcBlockRunning ())
                mAnaReg->AnalogAFE_Open(AUDIO_PATH);
            if(mStreamHandler->GetSrcBlockRunning())
            {
                EnableSpeaker();
            }
            break;
        }
        case android_audio_legacy::AudioSystem::DEVICE_OUT_AUX_DIGITAL:
        {
            //FM TX
            LOGD("open, CM, FM Tx");
            SetSpeakerStatus(false); // set spaker status false
            EnableSpeaker();
            if(mStreamHandler->GetSrcRunningNumber() > 0)
            {
#ifdef FM_ANALOG_OUT_SUPPORT
                mAnaReg->AnalogAFE_Open(AUDIO_PATH);
#endif
                mAfe_handle->Afe_Set_FmTx(mI2STxStatus,1);
            }
            else
            {
                mAfe_handle->Afe_Set_FmTx(mI2STxStatus,0);
            }
            break;
        }
        case android_audio_legacy::AudioSystem::DEVICE_OUT_EARPIECE:
        {
            LOG_HARDWARE("open, CM, DEVICE_OUT_EARPIECE ");
            mAfe_handle->Afe_Set_Mono();
            mAnaReg->AnalogAFE_Open(VOICE_PATH);
            SetSpeakerStatus(false); // set spaker status false
#ifdef EARPIECE_CONTROL
            SetEarpieceStatus(true);
            EnableEarpiece();
#endif
            break;
        }
        case android_audio_legacy::AudioSystem::DEVICE_OUT_HDMI_AUD:
        {
            LOGD("open, CM, HDMI_Audio(disable)");
            SetHDMIAudioConnect(false);
            break;
        }
        default:
        {
            LOG_HARDWARE("doOutputDeviceRouting, N/R, Pre_dev not specified ");
            break;
        }
        }
        // here to set mic state
        setMicMute(mMicMute);
        break;
    }
    default:
    {
        LOG_HARDWARE("doOutputDeviceRouting, default no route match");
        break;
    }
    }

NOT_ROUTING:

    if(mMode == android_audio_legacy::AudioSystem::MODE_IN_CALL)
    {
        LOG_HARDWARE("doOutputDeviceRouting, setVoiceVolume");

        mVolumeController->setVoiceVolume(mVolumeController->getVoiceVolume(),mMode,Route_device);

        if (m_Speaker_Enable_during_InCall == true)
        {
            m_Speaker_Enable_during_InCall = false;
            /* This sleep time is experienced value for skip pop noise during modem open ANA/AFE hw. */
            //usleep(300*1000);
            SetSpeakerStatus(true); // set spaker status true
            //EnableSpeaker();  // Move to ccco read thread.
        }
    }
    else if(mMode == android_audio_legacy::AudioSystem::MODE_IN_COMMUNICATION)
    {
        mVolumeController->setVoiceVolume(mVolumeController->getVoiceVolume(),mMode,GetCurrentDevice ());
    }
    else
    {
        //LOG_HARDWARE("doOutputDeviceRouting, setMasterVolume");
        mVolumeController->setMasterVolume(mVolumeController->getMasterVolume(),mMode,GetCurrentDevice());
    }
   #ifdef MTK_AUDIO_GAIN_TABLE_SUPPORT
   mAudioUCM->SetOutputDevice (Route_device);
   #endif
#ifdef HEADPHONE_CONTROL
	if((Route_device == android_audio_legacy::AudioSystem::DEVICE_OUT_WIRED_HEADSET ||
		Route_device == android_audio_legacy::AudioSystem::DEVICE_OUT_WIRED_HEADPHONE) &&
		(!GetFmSpeakerStatus() || (GetFmSpeakerStatus() && GetFmRxStatus()==false))) // fm not force to speaker or  force to speaker
	{                                                                              // but fm not start
		SetHeadPhoneStatus(true); //noisy after  hung up,but what i do?
		EnableHeadPhone();
	}
	LOGD("Route_device=%d,mMode=%d,mOuputForceuse[mMode]=%d,IsHeadsetPlugin(mMode)=%d",Route_device,mMode,mOuputForceuse[mMode],IsHeadsetPlugin(mMode));
	if(Route_device ==  android_audio_legacy::AudioSystem::DEVICE_OUT_SPEAKER &&
		(mOuputForceuse[mMode]== android_audio_legacy::AudioSystem::FORCE_SPEAKER ||mMode == android_audio_legacy::AudioSystem::MODE_RINGTONE) && //ringtong for ringtong,plug in headset
		(mMode != android_audio_legacy::AudioSystem::MODE_IN_CALL && mMode != android_audio_legacy::AudioSystem::MODE_IN_COMMUNICATION) && // not voice force to speaker
		IsHeadsetPlugin(mMode))
	{
		LOG_HARDWARE("open speaker and headphone");
		SetHeadPhoneStatus(true);
		EnableHeadPhone();
	}
#endif
    pthread_mutex_unlock(&ModeMutex);
    LOG_HARDWARE("-doOutputDeviceRouting");
    ioctl(mFd,AUD_SET_CLOCK,0);
    return NO_ERROR;
}


status_t AudioYusuHardware::doInputDeviceRouting()
{
    LOG_HARDWARE("+doInputDeviceRouting, mInputAvailable:0x%x ", mInputAvailable);
    int Route_device = -1;
    Route_device = Audio_Match_Input_device(mInputAvailable,true); // turn on this deivce
    LOG_HARDWARE("doInputDeviceRouting Route_device=0x%x, InputDevice=0x%x", Route_device,mInputAvailable);
    switch(Route_device)
    {
    case android_audio_legacy::AudioSystem::DEVICE_IN_COMMUNICATION:
    {
        LOGD("doInputDeviceRouting, Communication (Mic1) ");
        SetInputSource(LADIN_Microphone1);
        break;
    }
    case android_audio_legacy::AudioSystem::DEVICE_IN_BUILTIN_MIC:
    {
        LOGD("doInputDeviceRouting, BUILTIN_MIC (Mic1) ");
        SetInputSource(LADIN_Microphone1);
        break;
    }
    case android_audio_legacy::AudioSystem::DEVICE_IN_BLUETOOTH_SCO_HEADSET:
    {
        LOGD("doInputDeviceRouting, SCO_HEADSET (BT-SCO-Headset) ");
        pLad->LAD_SetSpeechMode(SPH_MODE_BT_EARPHONE);
        break;
    }
    case android_audio_legacy::AudioSystem::DEVICE_IN_WIRED_HEADSET:
    {
        LOGD("doInputDeviceRouting, HEADSET (Mic2) ");
        SetInputSource(LADIN_Microphone2);
        break;
    }
    case android_audio_legacy::AudioSystem::DEVICE_IN_BACK_MIC:
    {
        SetInputSource(LADIN_Microphone1);
        LOGD("DEVICE_IN_BUILTIN_MIC2 current not use ");
        break;
    }
    case android_audio_legacy::AudioSystem::DEVICE_IN_AMBIENT:
    {
        LOGD("DEVICE_IN_AMBIENT current not use  ");
        break;
    }
    case android_audio_legacy::AudioSystem::DEVICE_IN_AUX_DIGITAL:
    {
        SetInputSource(LADIN_SingleDigitalMic);
        LOGD("DEVICE_IN_AUX_DIGITAL current not use  ");
        break;
    }
    case android_audio_legacy::AudioSystem::DEVICE_IN_VOICE_CALL:
    {
        LOGD("DEVICE_IN_MIC_ARRAY current not use  ");
        break;
    }
    case android_audio_legacy::AudioSystem::DEVICE_IN_DEFAULT:
    {
        LOGD("DEVICE_IN_DEFAULT current not use  ");
        break;
    }
    }
   #ifdef MTK_AUDIO_GAIN_TABLE_SUPPORT
   mAudioUCM->SetInputDevice (Route_device);
   #endif
    LOG_HARDWARE("-doInputDeviceRouting ");
    return NO_ERROR;
}

status_t AudioYusuHardware::dumpInternals(int fd, const Vector<String16>& args)
{
    const size_t SIZE = 256;
    char buffer[SIZE];
    String8 result;
    result.append("AudioYusuHardware::dumpInternals\n");
    snprintf(buffer, SIZE, "\tmFd: %d mMicMute: %s\n",  mFd, mMicMute? "true": "false");
    result.append(buffer);
    ::write(fd, result.string(), result.size());
    return NO_ERROR;
}

status_t AudioYusuHardware::dump(int fd, const Vector<String16>& args)
{

    dumpInternals(fd, args);
    if (mStreamHandler->mInput[0] != NULL)
    {
        mStreamHandler->mInput[0]->dump(fd, args);
    }
    if (mStreamHandler->mOutput[0] != NULL)
    {
        mStreamHandler->mOutput[0]->dump(fd, args);
    }
    return NO_ERROR;

}

void AudioYusuHardware::SetModeToAMP(int mode)
{
    LOG_HARDWARE("SetModeToAMP(mode=%d)",mode);
    if(!mAuioDevice)
    {
        return;
    }
#ifdef ALL_USING_VOICEBUFFER_INCALL
    mAuioDevice->setMode(mode);
#endif
}

////////<earpiece

void AudioYusuHardware::SetEarpieceStatus(bool on)
{
    LOG_HARDWARE("SetEarpieceStatus(%d)",on);
    mEarpieceStatus = on;
}

bool AudioYusuHardware::GetEarpieceStatus(void)
{
    return mEarpieceStatus;
}

void AudioYusuHardware::EnableEarpiece()
{

    LOG_HARDWARE("EnableEarpiece(enable=%d)",GetEarpieceStatus());
    if(!mAuioDevice)
    {
        return;
    }
    if (GetEarpieceStatus() > 0)
    {
        mAuioDevice->setReceiver(true);// open earpiece
    }
    else
    {
        mAuioDevice->setReceiver(false);// disable earpiece
    }
}

void AudioYusuHardware::ForceDisableEarpiece()
{
    LOG_HARDWARE("ForceDisableEarpiece");
    mAuioDevice->setReceiver(false);// disable earpiece
}

////////earpiece>

////////<headphone
void AudioYusuHardware::SetHeadPhoneStatus(bool on)
{
    LOG_HARDWARE("SetHeadPhoneStatus(%d)", on);
    mHeadPhoneStatus = on;
}

bool AudioYusuHardware::GetHeadPhoneStatus(void)
{
    return mHeadPhoneStatus;
}

void AudioYusuHardware::EnableHeadPhone()
{
    LOG_HARDWARE("EnableHeadPhone(enable=%d) ",GetHeadPhoneStatus());
    if(!mAuioDevice)
    {
        return;
    }
    if (GetHeadPhoneStatus() > 0)
    {

        mAuioDevice->setHeadphone(true);// openheadphone

    }
    else
    {
        mAuioDevice->setHeadphone(false);// disable headphone

    }

}

void AudioYusuHardware::ForceEnableHeadPhone()
{
    LOG_HARDWARE("ForceEnableHeadPhone");
    mAuioDevice->setHeadphone(true);// enable headphone
}

void AudioYusuHardware::ForceDisableHeadPhone()
{
    LOG_HARDWARE("ForceDisableHeadPhone");
    mAuioDevice->setHeadphone(false);// disable headphone
}

bool AudioYusuHardware::IsHeadsetPlugin(int mode)
{
    if(mOutputAvailable[mode] &
            ( android_audio_legacy::AudioSystem::DEVICE_OUT_WIRED_HEADPHONE | android_audio_legacy::AudioSystem::DEVICE_OUT_WIRED_HEADSET)
      )
    {
        return true;
    }
    return false;
}

////////headphone>

void AudioYusuHardware::SetSpeakerStatus(bool enable)
{
    mSpeakerStatus = enable;
}

bool AudioYusuHardware::GetSpeakerStatus(void)
{
    return mSpeakerStatus;
}

void AudioYusuHardware::EnableSpeaker()
{
    LOG_HARDWARE("EnableSpeaker (enable %d)",GetSpeakerStatus());
    if(!mAuioDevice)
    {
        return;
    }
    if (GetSpeakerStatus() > 0)
    {
        mAuioDevice->setSpeaker(true);
    }
    else
    {
        mAuioDevice->setSpeaker(false);
    }
}
void AudioYusuHardware::ForceDisableSpeaker()
{
    LOG_HARDWARE("ForceDisableSpeaker");
    mAuioDevice->setSpeaker(false);
}
void AudioYusuHardware::ForceEnableSpeaker()
{
    LOG_HARDWARE("ForceEnableSpeaker");
    mAuioDevice->setSpeaker(true);

}
bool AudioYusuHardware::GetForceSpeakerStatus(void)
{
    return mForceSpeakerStatus;
}

void AudioYusuHardware::SwitchAudioClock(bool enable)
{
    if(enable)
    {
        ::ioctl(mFd, AUD_SET_ADC_CLOCK, 1);// turn on audio clock
    }
    else
    {
        ::ioctl(mFd, AUD_SET_ADC_CLOCK, 0);// turn off audio clock
    }
}

////////////////////////////////////////////////////////
// FM Control
////////////////////////////////////////////////////////
status_t AudioYusuHardware::SetFmEnable(bool enable)
{
    LOG_HARDWARE("+SetFmEnable(Analog) enable(%d), mFmStatus(%d) ",enable,mFmStatus);

    if(mFmStatus == enable)
    {
        LOGD("SetFmEnable is already set");
        return NO_ERROR;
    }

    pthread_mutex_lock(&ModeMutex);

    if(enable == true )  // enable FM neccessary function
    {
        //Add to enable fm's analog record
        mRecordFM = enable;

        ::ioctl(mFd, AUD_SET_LINE_IN_CLOCK, 1);// turn on audio clock

        mFmStatus = enable;
        //move to only adjust volume , open fm
        //mVolumeController->HandleFmFd (enable);

        if(mMode > android_audio_legacy::AudioSystem::MODE_NORMAL)
        {
            LOGD("SetFmEnable but mode is not correct!!");
            pthread_mutex_unlock(&ModeMutex);
            return NO_ERROR;
        }
        if (GetSpeakerStatus() > 0)
        {
            ForceDisableSpeaker();//Disable speaker to prevent pop noise.
        }

        int FmVolume = mVolumeController->GetFmVolume();

        pthread_mutex_lock(&MasterVolumeMutex);
        float MasterVolume = mVolumeController->getMasterVolume();
        setMasterVolume (0.0);
        mVolumeController->SetFmVolume(0);
        mVolumeController->SetLineInGainByMode(LINE_GAIN_FM);

        //mode is ringtone or incall
        //no stream is running
        if(mStreamHandler->GetSrcBlockRunning() == false )
        {
            LOG_HARDWARE("SetFmEnable AnalogAFE_Open(FM_PATH_STEREO) ");
            mAnaReg->AnalogAFE_Open(FM_PATH_STEREO);
        }
        // stream is running
        else if(mStreamHandler->GetSrcBlockRunning() == true)
        {
            LOG_HARDWARE("SetFmEnable GetCurrentDevice():0x%x ", GetCurrentDevice());
            mAnaReg->AnalogAFE_Depop(AUDIO_PATH,false);
            mAnaReg->AnalogAFE_Depop(FM_STEREO_AUDIO,true);
        }
        // stream is running, audio dac is turn on already , set pinmux
        else
        {
            LOG_HARDWARE("!SetFmEnable =AnalogAFE_Open(FM_PATH_STEREO) ");
            mAnaReg->AnalogAFE_Open(FM_PATH_STEREO);
        }
        setMasterVolume (MasterVolume);
        pthread_mutex_unlock(&MasterVolumeMutex);
        // remove ramp volume

        mVolumeController->SetFmVolume(FmVolume);
        SetFmSpeaker();
        ::ioctl(mFd, AUDDRV_FM_ANALOG_PATH, 1); // ask driver that it is FM analog path play

    }
    else if(enable == false )
    {
        ::ioctl(mFd, AUDDRV_FM_ANALOG_PATH, 0); // ask driver that it is FM analog path play

        //Add to enable fm's analog record
        mRecordFM = enable;
        mFmStatus = enable;
        // mode is ringtone or incall
        if(mMode > android_audio_legacy::AudioSystem::MODE_NORMAL)
        {
            pthread_mutex_unlock(&ModeMutex);
            return NO_ERROR;
        }

        if(mStreamHandler->GetSrcBlockRunning() == false)//audio is off
        {
            SetFmSpeaker();
            ForceDisableSpeaker();//Disable speaker first.
            pthread_mutex_lock(&MasterVolumeMutex);
            float MasterVolume = mVolumeController->getMasterVolume();
            setMasterVolume (0.0);
            mAnaReg->AnalogAFE_Close(FM_PATH_STEREO);
            setMasterVolume (MasterVolume);
            pthread_mutex_unlock(&MasterVolumeMutex);
            LOG_HARDWARE("SetFmEnable=%d, Ana_Close_Analog ",enable);
        }
        else if(mStreamHandler->GetSrcBlockRunning())// audio is on
        {
            SetFmSpeaker();
            mAnaReg->AnalogAFE_Close(FM_PATH_STEREO);
            mAnaReg->AnalogAFE_Open(AUDIO_PATH);
            EnableSpeaker();
        }
        ::ioctl(mFd, AUD_SET_LINE_IN_CLOCK, 0);// turn off audio clock
    }
    else  // Fm state not correct warning
    {
        LOGE("AudioYusuHardware::SetFmStatus error = %d",enable);
    }

    LOG_HARDWARE("-SetFmEnable(Analog)");
    pthread_mutex_unlock(&ModeMutex);
    return NO_ERROR;
}

bool AudioYusuHardware::GetFmRxStatus(void)
{
//   return GetAnalogLineinEnable() || GetFmDigitalInEnable();
    return GetFmAnalogInEnable() || GetFmDigitalInEnable();
}

bool AudioYusuHardware::GetFmAnalogInEnable()
{
    return mFmStatus;
}

bool AudioYusuHardware::GetAnalogLineinEnable(void)
{
    return GetMatvAnalogEnable() || GetFmAnalogInEnable();
}


bool AudioYusuHardware::GetFmDigitalInEnable(void)
{
    return mFmDigitalInStatus;
}

bool AudioYusuHardware::GetFmSpeakerStatus(void)
{
    return mFmSpeakerStatus;
}

status_t AudioYusuHardware::SetFmDigitalInEnable(bool enable)
{
    LOGD("SetFmDigitalInEnable, enable=%d",enable);
    mFmDigitalInStatus = enable;

    if(enable == true)
    {
        LOGD("SetFmDigitalInEnable AUDDRV_SET_FM_I2S_GPIO");
        if(mStreamHandler->GetSrcBlockRunning()&& GetFmSpeakerStatus() && IsHeadsetPlugin(mMode))
        {
        	usleep(150*1000); //sleep 150ms for music output from speaker
        }
        ::ioctl(mFd,AUDDRV_SET_FM_I2S_GPIO);// enable FM use I2S
        SetFmSpeaker();  //enable speaker when mFmSpeakerStatus is '1'. FM Apk will not keep the speaker status, it is kept in audio driver
    }
    else
    {
        LOGD("SetFmDigitalInEnable AUDDRV_RESET_BT_FM_GPIO");
        ::ioctl(mFd,AUDDRV_RESET_BT_FM_GPIO);// Reset GPIO pin mux
        EnableSpeaker();
#ifdef HEADPHONE_CONTROL
		if(GetFmSpeakerStatus() && IsHeadsetPlugin(mMode))
		{
			SetHeadPhoneStatus(true);
		}
		if(mStreamHandler->GetSrcBlockRunning() && IsHeadsetPlugin(mMode))
		{
			usleep(100*1000);  // fm sound leak out.
			EnableHeadPhone();
		}
#endif
    }
    return NO_ERROR;
}

status_t AudioYusuHardware::SetFmDigitalOutEnable(bool enable)
{
    LOGD("mFmDigitalOutStatus, enable=%d",enable);
    mFmDigitalOutStatus = enable;

    if(enable == true)
    {
        LOGD("SetFmDigitalOutEnable AUDDRV_SET_FM_I2S_GPIO");
        ::ioctl(mFd,AUDDRV_SET_FM_I2S_GPIO);// enable FM use I2S
    }
    else
    {
        LOGD("SetFmDigitalInEnable AUDDRV_RESET_BT_FM_GPIO");
        ::ioctl(mFd,AUDDRV_RESET_BT_FM_GPIO);// Reset GPIO pin mux
    }
    return NO_ERROR;
}


bool AudioYusuHardware::GetFmTxStatus(void)
{
    return (bool)mI2STxStatus;
}

bool AudioYusuHardware::GetRecordFM(void)
{
    return mRecordFM;
}
bool AudioYusuHardware::GetHaveExtDac(void)
{
#ifdef ENABLE_EXT_DAC
    LOGV("GetHaveExtDac");
    return true;
#endif
    LOGV("not GetHaveExtDac");
    return false;
}


////////////////////////////////////////////////////////
// mATV Control
////////////////////////////////////////////////////////
void AudioYusuHardware::SetMatvDigitalEnable(bool enable)
{
    mMatvDigitalstatus = enable;
    if(enable == true)
    {
        LOGD("SetMatvDigitalEnable, AUDDRV_ENABLE_ATV_I2S_GPIO");
        ::ioctl(mFd,AUDDRV_ENABLE_ATV_I2S_GPIO);  // Set ATV I2S path
    }
    else
    {
        LOGD("SetMatvDigitalEnable, AUDDRV_DISABLE_ATV_I2S_GPIO");
        ::ioctl(mFd,AUDDRV_DISABLE_ATV_I2S_GPIO);
    }
}

bool AudioYusuHardware::GetMatvAnalogEnable()
{
    return mMatvAnalogstatus;
}
bool AudioYusuHardware::GetMatvDigitalEnable()
{
    return mMatvDigitalstatus;
}
bool AudioYusuHardware::GetMatvStatus()
{
    LOGD("mMatvAnalogstatus = %d mMatvDigitalstatus = %d",mMatvAnalogstatus,mMatvDigitalstatus);
    return (mMatvAnalogstatus||mMatvDigitalstatus);
}

//MATV relate function
void  AudioYusuHardware::SetMatvAnalogEnable(bool enable)
{
    LOG_HARDWARE("SetMatvAnalogEnable: enable(%d), mMatvAnalogstatus(%d) ",enable,mMatvAnalogstatus);

    // (Analog line in) Matv maybe set during InCall mode.
    if(mMode > android_audio_legacy::AudioSystem::MODE_NORMAL)
    {
        LOGD("SetMatvAnalogEnable, Not in Normal Mode (%d)",mMode);
        m_AnalogIn_ATV_Enable_during_InCall = true;
        return ;
    }

    if(mMatvAnalogstatus == enable)
    {
        return;
    }

    m_AnalogIn_ATV_Enable_during_InCall = false;
    mMatvAnalogstatus = enable;
    if(enable == true )  // enable FM neccessary function
    {
        //Add to enable atv's analog record
        mRecordFM = enable;
        ::ioctl(mFd, AUD_SET_LINE_IN_CLOCK, 1);// turn on audio clock

        pthread_mutex_lock(&MasterVolumeMutex);
        float MasterVolume = mVolumeController->getMasterVolume();
        setMasterVolume (0.0);
        mVolumeController->SetLineInGainByMode(LINE_GAIN_FM);

        if(mStreamHandler->GetSrcBlockRunning() == false )
        {
            mAnaReg->AnalogAFE_Open(FM_PATH_STEREO);
        }
        // stream is running
        else if(mStreamHandler->GetSrcBlockRunning() == true)
        {
            mAnaReg->AnalogAFE_Open(FM_PATH_STEREO);
        }
        // stream is running, audio dac is turn on already , set pinmux
        else
        {
            LOG_HARDWARE("!SetMatvAnalogEnable =AnalogAFE_Open(FM_PATH_STEREO) ");
            mAnaReg->AnalogAFE_Open(FM_PATH_STEREO);
        }
        setMasterVolume (MasterVolume);
        pthread_mutex_unlock(&MasterVolumeMutex);
        // remove ramp volume

        SetMatvSpeaker();
    }
    else if(enable == false )
    {
        //Add to enable atv's analog record
        mRecordFM = enable;

        LOG_HARDWARE("SetMatvAnalogEnable, GetSrcBlockRunning(%d) ",mStreamHandler->GetSrcBlockRunning());
        if(mStreamHandler->GetSrcBlockRunning() == false)//audio is off
        {
            SetMatvSpeaker();
            pthread_mutex_lock(&MasterVolumeMutex);
            float MasterVolume = mVolumeController->getMasterVolume();
            setMasterVolume (0.0);
            mAnaReg->AnalogAFE_Close(FM_PATH_STEREO);
            setMasterVolume (MasterVolume);
            pthread_mutex_unlock(&MasterVolumeMutex);
            LOG_HARDWARE("SetMatvAnalogEnable=%d, Ana_Close_Analog ",enable);
        }
        else if(mStreamHandler->GetSrcBlockRunning())// audio is on
        {
            SetMatvSpeaker();
            mAnaReg->AnalogAFE_Close(FM_PATH_STEREO);
            mAnaReg->AnalogAFE_Open(AUDIO_PATH);
            EnableSpeaker();
        }

        ::ioctl(mFd, AUD_SET_LINE_IN_CLOCK, 0);// turn off audio clock
    }
    else  // Fm state not correct warning
    {
        LOGE("AudioYusuHardware::SetMatvStatus error = %d",enable);
    }

    return;
}

void AudioYusuHardware::SetBgsStatus(bool enable)
{
    mBgsStatus = enable;
}

bool AudioYusuHardware::GetBgsStatus(void)
{
    return mBgsStatus;
}

void  AudioYusuHardware::SetVoiceStatus(bool Enable)
{
    mVoiceStatus = Enable;
}

bool AudioYusuHardware::GetVoiceStatus(void)
{
    return mVoiceStatus;
}

void AudioYusuHardware::SetCommunicationStatus(bool Enable)
{
    mCommunicationStatus = Enable;
}

bool AudioYusuHardware::GetCommunicationStatus(void)
{
    return mCommunicationStatus;
}

LadInPutDevice_Line AudioYusuHardware::ChooseInputSource(int LadSource)
{
    if(LadSource > NUM_OF_InputDevice || LadSource < LADIN_FM_Radio)
    {
        LOGE("LadSource error!!");
        LadSource = LADIN_Microphone1;
        return LADIN_Microphone1;
    }
    else if(GetAnalogLineinEnable() == true && mMode == android_audio_legacy::AudioSystem::MODE_NORMAL && GetRecordFM() )
    {
        return LADIN_FM_Radio;
    }
    else
    {
        return (LadInPutDevice_Line)LadSource;
    }
}

//SetinputSource , set modem and AP side mictype
void AudioYusuHardware::SetInputSource(int LadSource)
{
    #ifdef MTK_DIGITAL_MIC_SUPPORT
       if(LadSource == LADIN_Microphone1)
       {
          LadSource = LADIN_SingleDigitalMic;//Modified by jrd.lipeng for digital dual mic
       }
       else if(LadSource == LADIN_DualAnalogMic)
       {
          LadSource = LADIN_DualDigitalMic;
       }
    #endif


    SetApMicType (LadSource);
    mStreamHandler->SetMicType(LadSource);
    if(mMode == android_audio_legacy::AudioSystem::MODE_IN_CALL)
    {
        pLad->LAD_SetInputSource(ChooseInputSource(LadSource));
    }
}

void AudioYusuHardware::CheckPinmux(int mPreviousMode,int mMode)
{

    if(mMode == android_audio_legacy::AudioSystem::MODE_NORMAL)
    {
        if(GetAnalogLineinEnable ()== false)
        {
            LOG_HARDWARE("CheckPinmux AnalogAFE_ChangeMux(AUDIO_PATH)");
            mAnaReg->AnalogAFE_ChangeMux(AUDIO_PATH);
        }
        else
        {
            LOG_HARDWARE("CheckPinmux AnalogAFE_ChangeMux(EXTERNAL_FM_R)");
            mAnaReg->AnalogAFE_ChangeMux(FM_PATH_STEREO);
        }
    }
    else if (mMode == android_audio_legacy::AudioSystem::MODE_RINGTONE)
    {
        LOG_HARDWARE("CheckPinmux AnalogAFE_ChangeMux(AUDIO_PATH)");
        mAnaReg->AnalogAFE_ChangeMux(AUDIO_PATH);
    }
    else if(mMode == android_audio_legacy::AudioSystem::MODE_IN_CALL)
    {
        LOG_HARDWARE("CheckPinmux AnalogAFE_ChangeMux(VOICE_PATH)");
        mAnaReg->AnalogAFE_ChangeMux(VOICE_PATH);
    }
    else if(mMode == android_audio_legacy::AudioSystem::MODE_IN_COMMUNICATION)
    {
        LOG_HARDWARE("CheckPinmux AnalogAFE_ChangeMux(AUDIO_PATH)");
        mAnaReg->AnalogAFE_ChangeMux(VOICE_PATH);
    }
    else
    {
        LOGE("CheckPinmux no route");
    }

}

bool AudioYusuHardware:: AddOutputDevices(int device)
{
    LOG_HARDWARE("AddOutputDevices device=0x%x", device);
    mOutputAvailable[android_audio_legacy::AudioSystem::MODE_NORMAL]    |= device;
    mOutputAvailable[android_audio_legacy::AudioSystem::MODE_RINGTONE] |= device;
    mOutputAvailable[android_audio_legacy::AudioSystem::MODE_IN_CALL]|= device;
    mOutputAvailable[android_audio_legacy::AudioSystem::MODE_IN_COMMUNICATION]|= device;
    return true;
}


bool AudioYusuHardware:: RemoveOutputDevices(int device)
{
    LOG_HARDWARE("RemoveOutputDevices device=0x%x", device);
    mOutputAvailable[android_audio_legacy::AudioSystem::MODE_NORMAL]    &= (~device);
    mOutputAvailable[android_audio_legacy::AudioSystem::MODE_RINGTONE] &= (~device);
    mOutputAvailable[android_audio_legacy::AudioSystem::MODE_IN_CALL ]   &= (~device);
    mOutputAvailable[android_audio_legacy::AudioSystem::MODE_IN_COMMUNICATION ]   &= (~device);
    return true;
}


bool AudioYusuHardware:: AddInputDevices(int device)
{
    LOG_HARDWARE("AddInputDevices device=0x%x", device);
    mInputAvailable |= device;
    return true;
}


bool AudioYusuHardware:: RemoveInputDevices(int device)
{
    LOG_HARDWARE("mInputAvailable=0x%x", mInputAvailable);
    LOG_HARDWARE("RemoveInputDevices device=0x%x", device);
    mInputAvailable &= (~device);
    return true;
}

int AudioYusuHardware::GetForceUseOutputDevice(uint32_t config)
{
    switch(config)
    {
    case android_audio_legacy::AudioSystem::FORCE_NONE:
    {
        // setforceuse to none , use outputavailable device
        return Audio_Match_Output_device(mOutputAvailable[mMode],mMode,false);
    }
    case android_audio_legacy::AudioSystem::FORCE_SPEAKER:
    {
        return android_audio_legacy::AudioSystem::DEVICE_OUT_SPEAKER;
    }
    case android_audio_legacy::AudioSystem::FORCE_HEADPHONES:
    {
        return android_audio_legacy::AudioSystem::DEVICE_OUT_WIRED_HEADPHONE;
    }
    case android_audio_legacy::AudioSystem::FORCE_BT_SCO:
    {
        return android_audio_legacy::AudioSystem::DEVICE_OUT_BLUETOOTH_SCO;
    }
    case android_audio_legacy::AudioSystem::FORCE_BT_A2DP:
    {
        return android_audio_legacy::AudioSystem::DEVICE_OUT_BLUETOOTH_A2DP;
    }
    case android_audio_legacy::AudioSystem::FORCE_WIRED_ACCESSORY:
    {
        return android_audio_legacy::AudioSystem::DEVICE_OUT_AUX_DIGITAL;
    }
    case android_audio_legacy::AudioSystem::FORCE_BT_CAR_DOCK:
    case android_audio_legacy::AudioSystem::FORCE_BT_DESK_DOCK:
    default:
    {
        LOGE("GetForceUseOutputDevice with no choose" );
        break;
    }
    }
    return 0;
}


int AudioYusuHardware::GetForceUseInputDevice(uint32_t config)
{
    LOGD("GetForceUseInputDevice config:%d",config);
    switch(config)
    {
    case android_audio_legacy::AudioSystem::FORCE_NONE:
    {
        return Audio_Match_Input_device(mInputAvailable,false);
    }
    case android_audio_legacy::AudioSystem::FORCE_SPEAKER:
    {
        return android_audio_legacy::AudioSystem::DEVICE_IN_BUILTIN_MIC;
    }
    case android_audio_legacy::AudioSystem::FORCE_HEADPHONES:
    {
        return android_audio_legacy::AudioSystem::DEVICE_IN_BUILTIN_MIC;
    }
    case android_audio_legacy::AudioSystem::FORCE_BT_SCO:
    {
        return android_audio_legacy::AudioSystem::DEVICE_IN_BLUETOOTH_SCO_HEADSET;
    }
    case android_audio_legacy::AudioSystem::FORCE_BT_A2DP:
    {
        return android_audio_legacy::AudioSystem::DEVICE_IN_BUILTIN_MIC;
    }
    case android_audio_legacy::AudioSystem::FORCE_WIRED_ACCESSORY:
    {
        return android_audio_legacy::AudioSystem::DEVICE_IN_WIRED_HEADSET;
    }
    default:
    {
        LOGE("GetForceUseOutputDevice with no choose" );
        break;
    }
    }
    return 0;
}

bool AudioYusuHardware::SetForceUse(uint32_t usage,uint32_t config)
{
    LOG_HARDWARE("SetForceUse usage=%d, config=%d mMode = %d",usage,config,mMode);
    bool checkdevice = false;
    switch(usage)
    {
    case android_audio_legacy::AudioSystem::FOR_COMMUNICATION:
    {
        mPredevice = GetForceUseOutputDevice(mOuputForceuse[mMode]);
        mOuputForceuse[android_audio_legacy::AudioSystem::MODE_IN_CALL] = config;
        mOuputForceuse[android_audio_legacy::AudioSystem::MODE_IN_COMMUNICATION] = config;
        mDevice = GetForceUseOutputDevice(mOuputForceuse[mMode]);
        if(mPredevice !=mDevice )
        {
            checkdevice = (config == android_audio_legacy::AudioSystem::FORCE_NONE);
            doOutputDeviceRouting(checkdevice,!checkdevice);
        }
        break;
    }
    case android_audio_legacy::AudioSystem::FOR_MEDIA:
    {
        if(mMode == android_audio_legacy::AudioSystem::MODE_NORMAL || mMode ==android_audio_legacy::AudioSystem::MODE_RINGTONE)
        {
            mPredevice = GetForceUseOutputDevice(mOuputForceuse[mMode]);
            mOuputForceuse[android_audio_legacy::AudioSystem::MODE_NORMAL] = config;
            mOuputForceuse[android_audio_legacy::AudioSystem::MODE_RINGTONE] = config;
            mDevice    = GetForceUseOutputDevice(mOuputForceuse[mMode]);
            if(mPredevice != mDevice)
            {
                checkdevice = (config == android_audio_legacy::AudioSystem::FORCE_NONE);
                doOutputDeviceRouting(checkdevice,!checkdevice);
            }
            else
            {
                if(mStreamHandler->GetSrcBlockRunning () == true || (GetAnalogLineinEnable()&&GetFmSpeakerStatus ()))
                {
                    EnableSpeaker();
                }
                else
                {
                    checkdevice = (config == android_audio_legacy::AudioSystem::FORCE_NONE);
                    doOutputDeviceRouting(checkdevice,!checkdevice);
                }
            }
        }
        else
        {
            LOG_HARDWARE("SetForceUse android_audio_legacy::AudioSystem::FOR_MEDIA: but in INCALL_MODE");
            // mode routing should take care of force use
            mOuputForceuse[android_audio_legacy::AudioSystem::MODE_NORMAL] = config;
            mOuputForceuse[android_audio_legacy::AudioSystem::MODE_RINGTONE] = config;
        }
        break;
    }
    case android_audio_legacy::AudioSystem::FOR_RECORD:
    {
        uint32 inputdevice = Audio_Match_Input_device(mInputAvailable,true);
        if(inputdevice == android_audio_legacy::AudioSystem::DEVICE_IN_BLUETOOTH_SCO_HEADSET || config == android_audio_legacy::AudioSystem::FORCE_BT_SCO)
        {
            LOGD("setforce FOR_RECORD with BT_device change");
            mInputForceuse = config;
            mStreamHandler->InputStreamLock();
            mStreamHandler->RecordClose();
            doInputDeviceRouting();
            mStreamHandler->RecordOpen();
            mStreamHandler->InputStreamUnLock();
        }
        else
        {
            mInputForceuse = config;
            doInputDeviceRouting();
        }
        break;
    }
    case android_audio_legacy::AudioSystem::FOR_DOCK:
    {
        break;
    }
    default:
    {
        LOGE("SetForceUse with no usage = %d",usage);
        return false;
    }
    }
    return true;
}

int AudioYusuHardware::SetTtyCtmMode(int tty_mode)
{
    LOG_HARDWARE("+SetTtyCtmMode:%d, mPreviousMode:%d, mMode:%d ",tty_mode,mPreviousMode,mMode);
    pthread_mutex_lock(&ttyMutex);

    // disable TTY-CTM before Speech Off
    if(mPreviousMode == android_audio_legacy::AudioSystem::MODE_IN_CALL)
    {
        LOG_HARDWARE("SetTtyCtmMode:%d, mPreviousMode:%d, previous mode is In-Call mode",tty_mode,mPreviousMode);
        if( (pLad->mTty_used_in_phone_call == true) && (tty_mode == AUD_TTY_OFF) )
        {
            if(GetCtm4WayFlag()) Enable_Modem_Record(false); //: disable ctm-4way record before CTM off

            pLad->LAD_TtyCtmOff();
            this->Set_Recovery_Tty(AUD_TTY_OFF);
            doOutputDeviceRouting();
            pthread_mutex_unlock(&ttyMutex);
            LOG_HARDWARE("-SetTtyCtmMode");
            return NO_ERROR;
        }
    }

    // Current mode must be in "MODE_IN_CALL" mode if user set tty-ctm.
    if(mMode != android_audio_legacy::AudioSystem::MODE_IN_CALL)
    {
        LOGD("SetTtyCtmMode:%d, mMode:%d, mTty_Ctm=%d. Not in call mode ",tty_mode,mMode,mTty_Ctm);
        pthread_mutex_unlock(&ttyMutex);
        LOG_HARDWARE("-SetTtyCtmMode");
        return NO_ERROR;
    }

    LOGD("SetTtyCtmMode:%d, mTty_used_in_phone_call:%d ",tty_mode,pLad->mTty_used_in_phone_call);
    if( (pLad->mTty_used_in_phone_call == true) && (tty_mode == AUD_TTY_OFF) )
    {
        if(GetCtm4WayFlag()) Enable_Modem_Record(false); //: disable ctm-4way record before CTM off

        pLad->LAD_TtyCtmOff();
        this->Set_Recovery_Tty(AUD_TTY_OFF);
        doOutputDeviceRouting();
    }
    else
    {
        if(pLad->mTty_used_in_phone_call == false)
        {
            pLad->LAD_TtyCtmOn(tty_mode);
            this->Set_Recovery_Tty(tty_mode);
        }
        else // tty-ctm already enable, must with different TTT_MODE, bcs we prevent same mode enter when using this function
        {

            // first: disable TTY-CTM
            pLad->LAD_TtyCtmOff();
            this->Set_Recovery_Tty(AUD_TTY_OFF);
            // second: enable TTY-CTM again
            pLad->LAD_TtyCtmOn(tty_mode);
            this->Set_Recovery_Tty(tty_mode);
            //doOutputDeviceRouting();
        }
        // if switch to VCO/HCO mode, need to do Output Device Routing again.
        doOutputDeviceRouting();
        if(GetCtm4WayFlag() && VmThreadExit) Enable_Modem_Record(true); //: enable ctm-4way record after CTM on, also VM thread is no running
    }

    pthread_mutex_unlock(&ttyMutex);
    LOG_HARDWARE("-SetTtyCtmMode");
    return NO_ERROR;
}

int AudioYusuHardware::Set_VTSpeechCall(int vt_mode)
{
    LOGD("Set_VTSpeechCall, vt_mode:%d",vt_mode);
    int sp_flag;

    sp_flag = Get_Recovery_Speech();

    // if current mode == MODE_IN_CALL
    if( (mMode == android_audio_legacy::AudioSystem::MODE_IN_CALL) && (sp_flag == true) )
    {
        LOGD("!! Set_VTSpeechCall in-call mode , vt_mode:%d",vt_mode);
    }

    if(vt_mode == true)
    {
        mVT_Speech = true;
    }
    else
    {
        mVT_Speech = false;
    }

    return true;
}

status_t  AudioYusuHardware::setParameters(const String8& keyValuePairs)
{
    status_t status = NO_ERROR;
    int value = 0;
    String8 value_str;
    pthread_mutex_lock(&ParametersMutex);
    AudioParameter param = AudioParameter(keyValuePairs);

    LOG_HARDWARE("+setParameters(): %s ", keyValuePairs.string());

    // get mAvailableOutputDevices , now device = mAvailableOutputDevices
    if (param.getInt(key1, value) == NO_ERROR)
    {
        LOG_HARDWARE("setParameters addoutputdevice=0x%x", value);
        param.remove(key1);
        if(mHeadsetDetect !=0 && mHeadsetDetect->isPlaying())
        {
           // this indicates that java  is alive, so headset dectect thread is no longer needed.
            mHeadsetDetect->stop();
            LOG_HARDWARE("headset detect thread exit");
        }
        if(mOuputForceuse[mMode] !=0 )
        {
            LOG_HARDWARE("AddOutputDevices with mOuputForceuse!=0, mMode=%d, mOuputForceuse=%d",mMode,mOuputForceuse[mMode]);
            AddOutputDevices(value);
        }
        else
        {
            mPredevice = mOutputAvailable[mMode];
            AddOutputDevices(value);
            mDevice = mOutputAvailable[mMode];
            if(value == android_audio_legacy::AudioSystem::DEVICE_OUT_BLUETOOTH_A2DP ||
               value == android_audio_legacy::AudioSystem::DEVICE_OUT_BLUETOOTH_SCO_HEADSET)
            {
                //ignore  routing in normal mode when a2dp or sco is connected.
                // playing music in background, then connect a2dp, the music sounds discontinuity.
                // because device like speake will be closed and then open.
                LOG_HARDWARE("add a2dp & SCO ,not routing device");
            }
            else
            {
                doOutputDeviceRouting();
            }
        }
        goto EXIT_SETPARAMETERS;
    }
    if (param.getInt(key2, value) == NO_ERROR)
    {
        LOG_HARDWARE("setParameters Removeoutputdevice=0x%x", value);
        param.remove(key2);
        if(mOuputForceuse[mMode] !=0)
        {
            LOG_HARDWARE("Removeoutputdevice with mOuputForceuse!=0 mMode=%d, mOuputForceuse=%d",mMode,mOuputForceuse[mMode]);
            RemoveOutputDevices(value);
        }
        else
        {
            mPredevice=mOutputAvailable[mMode];
            RemoveOutputDevices(value);
            mDevice =mOutputAvailable[mMode];
            doOutputDeviceRouting ();
        }
        goto EXIT_SETPARAMETERS;
    }
    // get mAvailableOutputDevices , now device = mAvailableIntputDevices
    if (param.getInt(key3, value) == NO_ERROR)
    {
        LOGD("setParameters AddInputDevices=0x%x", value);
        param.remove(key3);
        if(mInputForceuse !=0)
        {
            LOGD("AddInputDevices with mInputForceuse!=0, mMode=%d, mOuputForceuse=%d",mMode,mOuputForceuse[mMode]);
            AddInputDevices(value);
        }
        else
        {
            LOGD("AddInputDevices with doInputDeviceRouting");
            AddInputDevices(value);
            doInputDeviceRouting();
        }
        goto EXIT_SETPARAMETERS;
    }

    if (param.getInt(key4, value) == NO_ERROR)
    {
        LOGD("setParameters Removeoutputdevice=0x%x", value);
        param.remove(key4);
        if(mInputForceuse !=0)
        {
            LOGD("Removeoutputdevice with mOuputForceuse!=0, mMode=%d, mOuputForceuse=%d",mMode,mOuputForceuse[mMode]);
            RemoveInputDevices(value);
        }
        else
        {
            LOGD("Removeoutputdevice with doInputDeviceRouting");
            RemoveInputDevices(value);
            doInputDeviceRouting();
        }
        goto EXIT_SETPARAMETERS;
    }
    //exit headset detect thread
   if (param.getInt(keyHeadsetDetect, value) == NO_ERROR)
   {
       // this indicates that java  is alive, so headset dectect thread is no longer needed.
        if(mHeadsetDetect != 0 && mHeadsetDetect->isPlaying())
        {
           mHeadsetDetect->stop();
        }
        param.remove(keyHeadsetDetect);
        goto EXIT_SETPARAMETERS;
   }
   // keyrouting now only used for earpiece in normalmode
   if (param.getInt(key22, value) == NO_ERROR) {
       LOG_HARDWARE("KeyRouting device = 0x%x,mode=%d,mFakeMode=%d", value,mMode,mFakeMode);
       mRoutingDevices = value;
	   if(mMode == android_audio_legacy::AudioSystem::MODE_NORMAL && value == android_audio_legacy::AudioSystem::DEVICE_OUT_EARPIECE && !mFakeMode)
	   {
		     mFakeMode = true;
			 if(mOuputForceuse[mMode] !=0 ){
				 LOG_HARDWARE("AddOutputDevices with mOuputForceuse!=0, mMode=%d, mOuputForceuse=%d",mMode,mOuputForceuse[mMode]);
				 //AddOutputDevices(value);
				 mOutputAvailable[android_audio_legacy::AudioSystem::MODE_NORMAL]|= value;
			  }
			  else{
				 mPredevice = mOutputAvailable[mMode];
				 //AddOutputDevices(value);
				 mOutputAvailable[android_audio_legacy::AudioSystem::MODE_NORMAL] |= value;
				 mDevice = mOutputAvailable[mMode];
				 doOutputDeviceRouting();
			  }
	   }
	   if(mMode == android_audio_legacy::AudioSystem::MODE_NORMAL && mFakeMode && value != android_audio_legacy::AudioSystem::DEVICE_OUT_EARPIECE)
	   {
	   	   mFakeMode = false;
		   value = android_audio_legacy::AudioSystem::DEVICE_OUT_EARPIECE;
		   LOG_HARDWARE("setParameters Removeoutputdevice=0x%x", value);
		   if(mOuputForceuse[mMode] !=0){
			  LOG_HARDWARE("Removeoutputdevice with mOuputForceuse!=0 mMode=%d, mOuputForceuse=%d",mMode,mOuputForceuse[mMode]);
			  //RemoveOutputDevices(value);
			  mOutputAvailable[android_audio_legacy::AudioSystem::MODE_NORMAL] &= (~value);
		   }
		   else{
			  mPredevice=mOutputAvailable[mMode];
			  //RemoveOutputDevices(value);
			  mOutputAvailable[android_audio_legacy::AudioSystem::MODE_NORMAL] &= (~value);
			  mDevice =mOutputAvailable[mMode];
			  doOutputDeviceRouting ();
		   }
	   }
      param.remove(key22);
      goto EXIT_SETPARAMETERS;
   }
    // get previous mode and mode
    if (param.getInt(key5, value) == NO_ERROR)
    {
        param.remove(key5);
        goto EXIT_SETPARAMETERS;
    }
    //forceuse
    if (param.getInt(key6, value) == NO_ERROR)
    {
        uint32_t usage =(uint32)value>>16;
        uint32_t config = (uint32)value& 0xffff;
        LOG_HARDWARE("keyAddtForceuse usage = 0x%x config = 0x%x", usage, config);
        if(SetForceUse(usage,config) == 0 )
        {
            LOGE("AudioYusuHardware::setParameters SetForceUse return false usage = %d config = %d",usage,config);
        }
        param.remove(key6);
        goto EXIT_SETPARAMETERS;
    }
    //forceusePhone
    if (param.getInt(Key20, value) == NO_ERROR)
    {
        uint32_t usage =(uint32)value>>16;
        uint32_t config = (uint32)value& 0xffff;
        LOG_HARDWARE("keyAddtForceuse usage = 0x%x config = 0x%x", usage, config);
        if(SetForceUse(usage,config) == 0 )
        {
            LOGE("AudioYusuHardware::setParameters SetForceUse return false usage = %d config = %d",usage,config);
        }
        param.remove(Key20);
        goto EXIT_SETPARAMETERS;
    }
    // add setsamplerate , set to 44100 or 48000
    if (param.getInt(key7, value) == NO_ERROR)
    {
        //LOGD("AudioSetSampleRate = %d",value);
        mStreamHandler->SetOutputSamplerate(value);
        param.remove(key7);
        goto EXIT_SETPARAMETERS;
    }
    // add SetFmEnable
    if (param.getInt(key8, value) == NO_ERROR)
    {
        LOG_HARDWARE("SetFmEnable (Analog) = %d",value);
        SetFmEnable(value);
        param.remove(key8);
        goto EXIT_SETPARAMETERS;
    }
    // add SetFmForceLoudspk
    if (param.getInt(key9, value) == NO_ERROR)
    {
        LOG_HARDWARE("SetFmForceLoudspk = %d",value);
        mStreamHandler->OutputStreamLock();
        mFmSpeakerStatus = value;
        SetFmSpeaker();
        mStreamHandler->OutputStreamUnLock();
        param.remove(key9);
        goto EXIT_SETPARAMETERS;
    }
    // add SetLienInEnable
    if (param.getInt(key13, value) == NO_ERROR)
    {
        LOG_HARDWARE("SetMatvLineInEnable = %d",value);
        if(value)
        {
            mVolumeController->setMatvVolume (mVolumeController->GetMatvVolume ());
        }
        SetMatvAnalogEnable((bool)value);
        SetMatvSpeaker();
        param.remove(key13);
        goto EXIT_SETPARAMETERS;
    }

    if (param.getInt(keySetMatvMute, value) == NO_ERROR)
    {
        LOG_HARDWARE("SetMatvMute=%d",value);
#ifdef MATV_AUDIO_LINEIN_PATH
        mVolumeController->SetMatvMute((bool)value);
#endif
        param.remove(keySetMatvMute);
        goto EXIT_SETPARAMETERS;
    }
    if (param.getInt(key14, value) == NO_ERROR)
    {
        LOG_HARDWARE("SetFmDigitalEnable = %d",value);
        SetFmDigitalInEnable((bool)value);
        param.remove(key14);
        goto EXIT_SETPARAMETERS;
    }
    if (param.getInt(key16, value) == NO_ERROR)
    {
        LOG_HARDWARE("keySetMatvDigitalEnable = %d",value);
        SetMatvDigitalEnable((bool)value);
        param.remove(key16);
        goto EXIT_SETPARAMETERS;
    }
    if (param.getInt(Key17, value) == NO_ERROR)
    {
        LOG_HARDWARE("SetMatvVolume = %d",value);
#ifdef MATV_AUDIO_LINEIN_PATH
        mVolumeController->setMatvVolume(value);
#endif
        param.remove(Key17);
        goto EXIT_SETPARAMETERS;
    }
    // add SetFmVolume
    if (param.getInt(key10, value) == NO_ERROR)
    {
        LOG_HARDWARE("SetFmVolume(Analog)=%d",value);
#ifdef FM_ANALOG_INPUT
        mVolumeController->SetFmVolume(value);
#endif
        param.remove(key10);
        goto EXIT_SETPARAMETERS;
    }
    if (param.getInt(key11, value) == NO_ERROR)
    {
        LOG_HARDWARE("keySetI2SOutputEnable = %d",value);
        LOGD("keySetI2SOutputEnable = %d, mFMRXForceDisableFMTX = %d", value, mFMRXForceDisableFMTX);
        if(value)
        {
            mI2STxStatus ++;
        }
        else if(value ==0)
        {
            mI2STxStatus--;
        }
        else
        {
            LOGW("keySetI2SOutputEnable value = %d mI2STxStatus = %d",value,mI2STxStatus);
        }
        if(mI2STxStatus ==1)
        {
            if(mOuputForceuse[mMode] !=0 )
            {
                LOG_HARDWARE("AddOutputDevices with mOuputForceuse!=0 mMode = %d mOuputForceuse = %d",mMode,mOuputForceuse[mMode]);
                AddOutputDevices(android_audio_legacy::AudioSystem::DEVICE_OUT_AUX_DIGITAL);
            }
            else
            {
                mPredevice=mOutputAvailable[mMode] ;
                AddOutputDevices(android_audio_legacy::AudioSystem::DEVICE_OUT_AUX_DIGITAL);
                mDevice =mOutputAvailable[mMode] ;
                doOutputDeviceRouting();
            }
        }
        else if(mI2STxStatus ==0)
        {
            if(mOuputForceuse[mMode] !=0)
            {
                LOG_HARDWARE("Removeoutputdevice with mOuputForceuse!=0 mMode = %d mOuputForceuse = %d",mMode,mOuputForceuse[mMode]);
                RemoveOutputDevices(android_audio_legacy::AudioSystem::DEVICE_OUT_AUX_DIGITAL);
                if(mFMRXForceDisableFMTX)
                {
                    LOGD("FM TX close delay 180ms by mFMRXForceDisableFMTX=%d", mFMRXForceDisableFMTX);
                    usleep(180*1000); //ALPS00282348 sleep 180ms for TX->(SPK)->RX case, avoid little 180ms audio playback by spk
                }
            }
            else
            {
                mPredevice=mOutputAvailable[mMode];
                RemoveOutputDevices(android_audio_legacy::AudioSystem::DEVICE_OUT_AUX_DIGITAL);
                mDevice =mOutputAvailable[mMode];
                if(mFMRXForceDisableFMTX)
                {
                    LOGD("FM TX close delay 180ms by mFMRXForceDisableFMTX=%d", mFMRXForceDisableFMTX);
                    usleep(180*1000); //ALPS00282348 sleep 180ms for TX->(SPK)->RX case, avoid little 180ms audio playback by spk
                }
                doOutputDeviceRouting ();
            }
            mFMRXForceDisableFMTX = false;
        }
        else
        {
            LOGD("Enable TX with count = %d",mI2STxStatus);
        }

        param.remove(key11);
        goto EXIT_SETPARAMETERS;
    }

    if (param.getInt(keyFMRXForceDisableFMTX, value) == NO_ERROR)
    {
        LOGD("keyFMRXForceDisableFMTX = %d",value);
        if(mI2STxStatus>0)
        {
            mFMRXForceDisableFMTX = true;
        }

        param.remove(keyFMRXForceDisableFMTX);
        goto EXIT_SETPARAMETERS;
    }
    // add  keySetTtyMode
    // if (param.get(key12, value_str) == NO_ERROR &&
    //	(mMode == android_audio_legacy::AudioSystem::MODE_IN_CALL)) // : to make sure TTY set only on phone call
    if (param.get(key12, value_str) == NO_ERROR  )
    {
        int tty_mode;

        if(value_str == "tty_full")
        {
            tty_mode = AUD_TTY_FULL;
        }
        else if(value_str == "tty_vco")
        {
            tty_mode = AUD_TTY_VCO;
        }
        else if(value_str == "tty_hco")
        {
            tty_mode = AUD_TTY_HCO;
        }
        else if(value_str == "tty_off")
        {
            tty_mode = AUD_TTY_OFF;
        }
        else
        {
            LOGD("setParameters tty_mode error !!");
            tty_mode = AUD_TTY_ERR;
        }
        LOGD("setParameters, mTty_Ctm=%d, tty_mode=%d",mTty_Ctm,tty_mode);

        // Set by phone apk            : tty_mode
        // Current mode in audio driver: mTty_Ctm
        if(mTty_Ctm != tty_mode)
        {
            mTty_Ctm = tty_mode;
            SetTtyCtmMode(mTty_Ctm);
        }
        param.remove(key12);
        goto EXIT_SETPARAMETERS;
    }

    if (param.getInt(key15, value) == NO_ERROR)
    {
        LOG_HARDWARE("SetVTSpeechCall=%d",value);
        Set_VTSpeechCall(value);
        param.remove(key15);
        goto EXIT_SETPARAMETERS;
    }

    if (param.getInt(key31, value) == NO_ERROR)
    {
        LOG_HARDWARE("SetPhoneMode = %d",value);
        pLad->mHeadSetMessager->SetHeadSetState (value); // tell headset driver status
        mPhoneMode = value;
        param.remove(key31);
        goto EXIT_SETPARAMETERS;
    }

    if (param.getInt(key41, value) == NO_ERROR)
    {
        LOG_HARDWARE("SetForceSpeaker=%d",value);
        ForceSpeaker((bool)value);
        param.remove(key41);
        goto EXIT_SETPARAMETERS;
    }

    // HDMI setting
    if (param.getInt(key_SET_HDMI_AUDIO_ENABLE, value) == NO_ERROR)
    {
        LOG_HARDWARE("SetHDMIAudioEnable=%d",value);
        SetHDMIAudioStatus(value);
        param.remove(key_SET_HDMI_AUDIO_ENABLE);
        goto EXIT_SETPARAMETERS;
    }

   if (param.getInt(keyHwInitStreamVoume, value) == NO_ERROR) {
      LOG_HARDWARE("keyHwInitStreamVoume=%x",value);
      #ifdef MTK_AUDIO_GAIN_TABLE_SUPPORT
      int stream = value>>8;
      int indexMin = (value >>4)&0xf;
      value= value&0xf ;
      mAudioUCM->InitStreamLevel ((android_audio_legacy::AudioSystem::stream_type)stream, indexMin, value);
      #endif
      param.remove(keyHwInitStreamVoume);
      goto EXIT_SETPARAMETERS;
   }
   if (param.getInt(keyHwSetVoumeIndex, value) == NO_ERROR) {
      LOG_HARDWARE("keyHwSetVoumeIndex=%x",value);

      #ifdef MTK_AUDIO_GAIN_TABLE_SUPPORT
      int stream = value>>4;
      value= value&0xf ;
      mAudioUCM->SetStreamLevel ((android_audio_legacy::AudioSystem::stream_type)stream, value);
      #endif

      param.remove(keyHwSetVoumeIndex);
      goto EXIT_SETPARAMETERS;
   }
   if (param.getInt(keyHwSetStreamStart, value) == NO_ERROR) {
      LOG_HARDWARE("keyHwSetStreamStart=%x",value);
      #ifdef MTK_AUDIO_GAIN_TABLE_SUPPORT
      mAudioUCM->StreamStart ((android_audio_legacy::AudioSystem::stream_type)value);
      #endif
      param.remove(keyHwSetStreamStart);
      goto EXIT_SETPARAMETERS;
   }
   if (param.getInt(keyHwSetStreamStop, value) == NO_ERROR) {
      LOG_HARDWARE("keyHwSetStreamStop=%x",value);
      #ifdef MTK_AUDIO_GAIN_TABLE_SUPPORT
      mAudioUCM->StreamStop ((android_audio_legacy::AudioSystem::stream_type)value);
      #endif
      param.remove(keyHwSetStreamStop);
      goto EXIT_SETPARAMETERS;
   }
   if (param.getInt(keyHwSetRecordStreamStart, value) == NO_ERROR) {
      LOG_HARDWARE("keyHwSetRecordStreamStart=%x",value);
      #ifdef MTK_AUDIO_GAIN_TABLE_SUPPORT
      mAudioUCM->StreamStart ((android_audio_legacy::AudioSystem::stream_type)value);
      #endif
      param.remove(keyHwSetStreamStart);
      goto EXIT_SETPARAMETERS;
   }
   if (param.getInt(keyHwSetRecordStreamStop, value) == NO_ERROR) {
      LOG_HARDWARE("keyHwSetRecordStreamStop=%x",value);
      #ifdef MTK_AUDIO_GAIN_TABLE_SUPPORT
      mAudioUCM->StreamStop ((android_audio_legacy::AudioSystem::stream_type)value);
      #endif
      param.remove(keyHwSetStreamStop);
      goto EXIT_SETPARAMETERS;
   }

    if (param.getInt(keyEnableStereoOutput, value) == NO_ERROR) {
        LOG_HARDWARE("keyEnableStereoOutput=%d",value);
        EnableStereoOutput(value);
        param.remove(keyEnableStereoOutput);
        goto EXIT_SETPARAMETERS;
    }

#if defined(MTK_DUAL_MIC_SUPPORT)
// Use for MMI Setting
    if (param.getInt(keyEnable_Dual_Mic_Setting, value) == NO_ERROR)
    {
        LOGD("Enable_Dual_Mic_Setting=%d",value);
        MMI_Set_Dual_Mic_Enable(value);
        param.remove(keyEnable_Dual_Mic_Setting);
        goto EXIT_SETPARAMETERS;
    }
#endif

#if defined(MTK_DUAL_MIC_SUPPORT)||defined(MTK_AUDIO_HD_REC_SUPPORT)

// For META Dual-mic special format recording

// step1: Set META_SET_DUAL_MIC_FLAG=1 --> Enable dual-mic test.
// step2: Set META_DUAL_MIC_FILE_NAME=donglei_In.vm -->set input fine name
//            Set META_DUAL_MIC_OUT_FILE_NAME=donglei_Out.vm  --> set output name
// step3: Set META_SET_DUAL_MIC_WB=1/0 --> Enable/Disable WB calibration if necessary, default value base on WB feature support
// step4: Set META_DUAL_MIC_RECORD=1/0   --> Start/Stop dual-mic recording.
//        Set META_DUAL_MIC_REC_PLAY=1/0 --> Start/Stop dual-mic playback+recording.
//        Set META_DUAL_MIC_REC_PLAY_HS=1/0 --> Start/Stop dual-mic playback+recording from headset

    if (param.getInt(keyMETA_SET_DUAL_MIC_FLAG, value) == NO_ERROR)
    {
        LOG_HARDWARE("META_SET_DUAL_MIC_FLAG=%d",value);
        META_Set_DualMic_Test_Mode((int)value);
        param.remove(keyMETA_SET_DUAL_MIC_FLAG);
        goto EXIT_SETPARAMETERS;
    }
    if (param.getInt(keyMETA_SET_DUAL_MIC_WB, value) == NO_ERROR)
    {
        LOG_HARDWARE("META_SET_DUAL_MIC_WB=%d",value);
        //When WB option is ont present, Setting to WB mode has no effect
        //Will always set to NB calibraion. Error message will present
        //See the implementation of META_Set_DualMic_WB()
        META_Set_DualMic_WB((int)value);
        param.remove(keyMETA_SET_DUAL_MIC_WB);
        goto EXIT_SETPARAMETERS;
    }

    if (param.get(keyMETA_DUAL_MIC_OUT_FILE_NAME, value_str) == NO_ERROR)
    {
        mDmTmpOutStr8 = value_str;
        m_str_pOutFile = NULL;
        m_str_pOutFile = mDmTmpOutStr8.string();
        LOG_HARDWARE("META_DUAL_MIC_OUT_FILE_NAME=%s",m_str_pOutFile);
        param.remove(keyMETA_DUAL_MIC_OUT_FILE_NAME);
        goto EXIT_SETPARAMETERS;
    }

    if (param.get(keyMETA_DUAL_MIC_IN_FILE_NAME, value_str) == NO_ERROR)
    {
        mDmTmpInStr8 = value_str;
        m_str_pInFile = NULL;
        m_str_pInFile = mDmTmpInStr8.string();
        LOG_HARDWARE("META_DUAL_MIC_IN_FILE_NAME=%s",m_str_pInFile);
        param.remove(keyMETA_DUAL_MIC_IN_FILE_NAME);
        goto EXIT_SETPARAMETERS;
    }

    if (param.getInt(keyMETA_DUAL_MIC_RECORD, value) == NO_ERROR)
    {
        LOG_HARDWARE("META_DUAL_MIC_RECORD=%d",value);
        /*
        if(value==0){
           Enable_Modem_Dual_Mic_Rec(FALSE);
        }
        else{
           Enable_Modem_Dual_Mic_Rec(TRUE);
        }
        */
        if(value==0)
        {
            Enable_Modem_2Way_Dual_Mic(FALSE, DMMETA_RECEIVER_OUT, mDualMicMetaWB, DMMETA_2WAY_RECONLY);
        }
        else
        {
            Enable_Modem_2Way_Dual_Mic(TRUE, DMMETA_RECEIVER_OUT, mDualMicMetaWB, DMMETA_2WAY_RECONLY);
        }
        param.remove(keyMETA_DUAL_MIC_RECORD);
        goto EXIT_SETPARAMETERS;
    }

    if (param.getInt(keyMETA_DUAL_MIC_REC_PLAY, value) == NO_ERROR)
    {
        LOG_HARDWARE("META_DUAL_MIC_REC_PLAY=%d",value);
        if(value==0)
        {
            Enable_Modem_2Way_Dual_Mic(FALSE, DMMETA_RECEIVER_OUT, mDualMicMetaWB, DMMETA_2WAY_NORMAL);
        }
        else
        {
            Enable_Modem_2Way_Dual_Mic(TRUE, DMMETA_RECEIVER_OUT, mDualMicMetaWB, DMMETA_2WAY_NORMAL);
        }
        param.remove(keyMETA_DUAL_MIC_REC_PLAY);
        goto EXIT_SETPARAMETERS;
    }

    if (param.getInt(keyMETA_DUAL_MIC_REC_PLAY_HS, value) == NO_ERROR)
    {
        LOG_HARDWARE("META_DUAL_MIC_REC_PLAY_HS=%d",value);
        if(value==0)
        {
            Enable_Modem_2Way_Dual_Mic(FALSE, DMMETA_HEADSET_OUT, mDualMicMetaWB, DMMETA_2WAY_NORMAL);
        }
        else
        {
            Enable_Modem_2Way_Dual_Mic(TRUE, DMMETA_HEADSET_OUT, mDualMicMetaWB, DMMETA_2WAY_NORMAL);
        }
        param.remove(keyMETA_DUAL_MIC_REC_PLAY_HS);
        goto EXIT_SETPARAMETERS;
    }

    if (param.getInt(keyMETA_DUAL_MIC_SET_UL_GAIN, value) == NO_ERROR)
    {
        LOG_HARDWARE("META_DUAL_MIC_SET_UL_GAIN=%d",value);
        SetDualMicToolMicGain(value);
        param.remove(keyMETA_DUAL_MIC_SET_UL_GAIN);
        goto EXIT_SETPARAMETERS;
    }

    if (param.getInt(keyMETA_DUAL_MIC_SET_DL_GAIN, value) == NO_ERROR)
    {
        LOG_HARDWARE("META_DUAL_MIC_SET_DL_GAIN=%d",value);
        SetDualMicToolReceiverGain(value);
        param.remove(keyMETA_DUAL_MIC_SET_DL_GAIN);
        goto EXIT_SETPARAMETERS;
    }

    if (param.getInt(keyMETA_DUAL_MIC_SET_HSDL_GAIN, value) == NO_ERROR)
    {
        LOG_HARDWARE("META_DUAL_MIC_SET_HSDL_GAIN=%d",value);
        SetDualMicToolHeadsetGain(value);
        param.remove(keyMETA_DUAL_MIC_SET_HSDL_GAIN);
        goto EXIT_SETPARAMETERS;
    }

#endif

#if defined(MTK_SPH_EHN_CTRL_SUPPORT)
    if (param.getInt(key_SET_VCE_ENABLE, value) == NO_ERROR) {
        param.remove(key_SET_VCE_ENABLE);
        MMI_Set_VCE_Enable(value);
        goto EXIT_SETPARAMETERS;
    }
#endif


    // Set Loopback Type
    if (param.getInt(key_SET_LOOPBACK_TYPE, value) == NO_ERROR) {
        param.remove(key_SET_LOOPBACK_TYPE);
        if (value == NO_LOOPBACK) {
            mAudFtm->SetLoopbackOff();
        }
        else
        {
            uint32_t u4CurDev = GetCurrentDevice();
            bool bEarphoneInserted =
                 (u4CurDev == android_audio_legacy::AudioSystem::DEVICE_OUT_WIRED_HEADSET ||
                  u4CurDev == android_audio_legacy::AudioSystem::DEVICE_OUT_WIRED_HEADPHONE) ? true : false;

            mAudFtm->SetLoopbackOn((ENUM_LOOPBACK_TYPES)value, bEarphoneInserted);
        }
        goto EXIT_SETPARAMETERS;
    }

/// ---> RILSetHeadset start
if (param.getInt(keyRILSetHeadsetState, value) == NO_ERROR)
{
    LOG_HARDWARE("RILSetHeadsetState=%d",value);
    if((pLad!=NULL) && (pLad->mHeadSetMessager!=NULL))
    {
        pLad->mHeadSetMessager->SetHeadSetState((int)value); // tell headset driver status
    }
    else
    {
        LOG_HARDWARE("RILSetHeadsetState(%d) error",value);
    }
    param.remove(keyRILSetHeadsetState);
    goto EXIT_SETPARAMETERS;
}
/// ---> RILSetHeadset end
    //<---for speech parameters calibration
    if (param.getInt(keySpeechParams_Update, value) == NO_ERROR) {
        LOGD("setParameters Update Speech Parames");
        int ret = 0;
        if (value ==0) {
            AUDIO_CUSTOM_PARAM_STRUCT pCustomPara;
            GetCustParamFromNV(&pCustomPara);
            // set to modem side
            ret = pLad->LAD_SetSpeechParameters(&pCustomPara);
            LOGD("LAD_SetSpeechParameters ret=%d",ret);
        }else if(value==1){
#if defined(MTK_WB_SPEECH_SUPPORT)
            AUDIO_CUSTOM_WB_PARAM_STRUCT pCustomWbParam;
            GetCustWBParamFromNV(&pCustomWbParam);
            ret = pLad->LAD_SetWBSpeechParameters(&pCustomWbParam);
            LOGD("LAD_SetWBSpeechParameters ret=%d",ret);
#endif
        }

        if (mMode == android_audio_legacy::AudioSystem::MODE_IN_CALL) {
            int outputDevice = Audio_Match_Output_device(mDevice,mMode,true);
            switch(outputDevice)
            {
              case android_audio_legacy::AudioSystem::DEVICE_OUT_WIRED_HEADSET :
              case android_audio_legacy::AudioSystem::DEVICE_OUT_WIRED_HEADPHONE :
              {
          	      if(mTty_Ctm == AUD_TTY_VCO || mTty_Ctm == AUD_TTY_HCO) {
          	  	      pLad->LAD_SetSpeechMode(SPH_MODE_NORMAL);
          	      }else {
                      pLad->LAD_SetSpeechMode(SPH_MODE_EARPHONE);
                  }
                  break;
              }
              case android_audio_legacy::AudioSystem::DEVICE_OUT_BLUETOOTH_SCO:
              case android_audio_legacy::AudioSystem::DEVICE_OUT_BLUETOOTH_SCO_HEADSET:
              case android_audio_legacy::AudioSystem::DEVICE_OUT_BLUETOOTH_SCO_CARKIT:
              {
                  pLad->LAD_SetSpeechMode(SPH_MODE_BT_EARPHONE);
                  break;
              }
              case android_audio_legacy::AudioSystem::DEVICE_OUT_SPEAKER:
              {
                  pLad->LAD_SetSpeechMode(SPH_MODE_LOUDSPK);
                  break;
              }
              case android_audio_legacy::AudioSystem::DEVICE_OUT_EARPIECE:
              {
                  pLad->LAD_SetSpeechMode(SPH_MODE_NORMAL);
                  break;
              }
              default:
              {
                  pLad->LAD_SetSpeechMode(SPH_MODE_NORMAL);
                  break;
              }
            }
		    }
		    param.remove(keySpeechParams_Update);
		    goto EXIT_SETPARAMETERS;
    }

    if (param.getInt(keySpeechVolume_Update, value) == NO_ERROR) {
        LOGD("setParameters Update Speech volume");
        mVolumeController->InitVolumeController ();
        if (mMode == android_audio_legacy::AudioSystem::MODE_IN_CALL) {
            int outputDevice = Audio_Match_Output_device(mDevice,mMode,true);
            mVolumeController->setVoiceVolume(mVolumeController->getVoiceVolume(), mMode, (uint32)outputDevice);
            switch(outputDevice)
            {
              case android_audio_legacy::AudioSystem::DEVICE_OUT_WIRED_HEADSET :
              {
                  if(mTty_Ctm == AUD_TTY_VCO) {
          	  	      mVolumeController->SetLadMicGain(Normal_Mic);
          	      }else if(mTty_Ctm == AUD_TTY_HCO || mTty_Ctm == AUD_TTY_FULL) {
                      mVolumeController->SetLadMicGain(TTY_CTM_Mic);
                  }else {
                      mVolumeController->SetLadMicGain(Headset_Mic);
                  }
                  break;
              }
              case android_audio_legacy::AudioSystem::DEVICE_OUT_WIRED_HEADPHONE :
              {
          	      if(mTty_Ctm == AUD_TTY_VCO) {
          	  	      mVolumeController->SetLadMicGain(Normal_Mic);
          	      }else if(mTty_Ctm == AUD_TTY_HCO || mTty_Ctm == AUD_TTY_FULL) {
                      mVolumeController->SetLadMicGain(TTY_CTM_Mic);
                  }else {
                      mVolumeController->SetLadMicGain(Handfree_Mic);
                  }
                  break;
              }
              case android_audio_legacy::AudioSystem::DEVICE_OUT_SPEAKER:
              {
                  mVolumeController->SetLadMicGain(Handfree_Mic);
                  break;
              }
              case android_audio_legacy::AudioSystem::DEVICE_OUT_EARPIECE:
              {
                  mVolumeController->SetLadMicGain(Normal_Mic);
                  break;
              }
              default:
              {
                  break;
              }
            }
        }else {
            setMasterVolume(mVolumeController->getMasterVolume ());
        }
        param.remove(keySpeechVolume_Update);

        goto EXIT_SETPARAMETERS;
    }

    if (param.getInt(keyACFHCF_Update, value) == NO_ERROR) {
        int len = 0;
        AUDIO_ACF_CUSTOM_PARAM_STRUCT pCustomPara;
        if (value==0) {
#if defined(ENABLE_AUDIO_COMPENSATION_FILTER) || defined(ENABLE_AUDIO_DRC_SPEAKER)
            LOGD("setParameters Update ACF Parames");
            GetAudioCompFltCustParamFromNV(&pCustomPara);
            mStreamHandler->mOutput[0]->SetACFPreviewParameter((void *)(&pCustomPara),len);
            mStreamHandler->mOutput[0]->startACFProcess(false);
            mStreamHandler->mOutput[0]->startACFProcess(true);
#endif
        }else if(value==1) {
#ifdef ENABLE_HEADPHONE_COMPENSATION_FILTER
            LOGD("setParameters Update HCF Parames");
            GetHeadphoneCompFltCustParamFromNV(&pCustomPara);
            mStreamHandler->mOutput[0]->SetHCFPreviewParameter((void *)(&pCustomPara),len);
            mStreamHandler->mOutput[0]->startHCFProcess(false);
            mStreamHandler->mOutput[0]->startHCFProcess(true);
#endif
        }
        param.remove(keyACFHCF_Update);
        goto EXIT_SETPARAMETERS;
    }
    //--->
#if defined(MTK_AUDIO_HD_REC_SUPPORT)
    if(param.getInt(keyHDREC_SET_VOICE_MODE, value) == NO_ERROR)
    {
        LOG_HARDWARE("HDREC_SET_VOICE_MODE=%d",value); // Normal, Indoor, Outdoor,
        param.remove(keyHDREC_SET_VOICE_MODE);
        //Get and Check Voice/Video Mode Offset
        AUDIO_HD_RECORD_SCENE_TABLE_STRUCT hdRecordSceneTable;
        GetHdRecordSceneTableFromNV(&hdRecordSceneTable);
        if (value < hdRecordSceneTable.num_voice_rec_scenes)
        {
            mHdRecScene = value + 1;//1:cts verifier offset
        }
        else
        {
            LOGE("HDREC_SET_VOICE_MODE=%d exceed max value(%d)\n",value, hdRecordSceneTable.num_voice_rec_scenes);
        }
        goto EXIT_SETPARAMETERS;
    }
    if(param.getInt(keyHDREC_SET_VIDEO_MODE, value) == NO_ERROR)
    {
        LOG_HARDWARE("HDREC_SET_VIDEO_MODE=%d",value); // Normal, Indoor, Outdoor,
        param.remove(keyHDREC_SET_VIDEO_MODE);
        //Get and Check Voice/Video Mode Offset
        AUDIO_HD_RECORD_SCENE_TABLE_STRUCT hdRecordSceneTable;
        GetHdRecordSceneTableFromNV(&hdRecordSceneTable);
        if (value < hdRecordSceneTable.num_video_rec_scenes)
        {
            uint32 offset = hdRecordSceneTable.num_voice_rec_scenes + 1;//1:cts verifier offset
            mHdRecScene = value + offset;
        }
        else
        {
            LOGE("HDREC_SET_VIDEO_MODE=%d exceed max value(%d)\n",value, hdRecordSceneTable.num_video_rec_scenes);
        }
        goto EXIT_SETPARAMETERS;
    }
#endif

EXIT_SETPARAMETERS:
    pthread_mutex_unlock(&ParametersMutex);

    if (param.size())
    {
        LOGE("setParameters still have param.size() = %d",param.size());
        status = BAD_VALUE;
    }
    LOG_HARDWARE("-setParameters() ");
    return status;
}

void  AudioYusuHardware::ForceSpeaker(bool bEnable)
{
    LOGD("ForceSpeaker bEnable = %d",bEnable);
    mForceSpeakerStatus = bEnable;
    usleep(1500*1000);
    if(mForceSpeakerStatus)
    {
        ForceEnableSpeaker();
    }
    else
    {
        ForceDisableSpeaker();
    }
}

//static
void AudioYusuHardware::AudioEarphoneCallback(void * user,bool bEarphonetEnable)
{
    LOG_HARDWARE("+AudioHardwareCallback EarphonetEnable=%d",bEarphonetEnable);
    AudioYusuHardware * hw = (AudioYusuHardware * )user;
    hw->SetSpeakerStatus(!bEarphonetEnable);
    if(hw->mStreamHandler->GetSrcBlockRunning())
    {
        hw->EnableSpeaker();
    }
    LOG_HARDWARE("-AudioHardwareCallback EarphonetEnable=%d",bEarphonetEnable);
}


bool AudioYusuHardware::SetFmSpeaker()
{
    if(mMode > android_audio_legacy::AudioSystem::MODE_NORMAL)
        return false;

    LOG_HARDWARE("SetFmSpeaker mFmSpeakerStatus:%d, mFmStatus:%d, mFmDigitalInStatus:%d",mFmSpeakerStatus,mFmStatus,mFmDigitalInStatus);
#ifdef HEADPHONE_CONTROL
	if(GetFmSpeakerStatus() && (GetAnalogLineinEnable() || GetFmDigitalInEnable()) )
	{
		ForceDisableHeadPhone();
	}
#endif
    if( (GetFmSpeakerStatus() && GetAnalogLineinEnable()) )
    {
        LOGD("1SetFmSpeaker mFmSpeakerStatus:%d, GetAnalogLineinEnable:%d",mFmSpeakerStatus,GetAnalogLineinEnable() );
        /* //ALPS00065106 to skip sound is turned off in AnalogAFE_Open().
        mAnaReg->AnalogAFE_Close(AUDIO_PATH);
        mAnaReg->AnalogAFE_Open(FM_PATH_STEREO);
        */
        ForceEnableSpeaker ();
        //return false;
    }
    else if( (GetFmSpeakerStatus()) && (GetFmDigitalInEnable()) )
    {
        LOGD("SetFmSpeaker mFmSpeakerStatus:%d, mFmDigitalInStatus:%d",mFmSpeakerStatus,mFmDigitalInStatus);
        ForceEnableSpeaker();
        //return false;
    }
    else if( (GetFmSpeakerStatus()==false) && (GetFmDigitalInEnable()) )
    {
        LOGD("SetFmSpeaker mFmSpeakerStatus:%d, mFmDigitalInStatus:%d",mFmSpeakerStatus,mFmDigitalInStatus);
        ForceDisableSpeaker();
        if(mMode <= android_audio_legacy::AudioSystem::MODE_NORMAL && mOuputForceuse[android_audio_legacy::AudioSystem::MODE_NORMAL] != android_audio_legacy::AudioSystem::FORCE_SPEAKER )
        {
            if(GetCurrentDevice() == android_audio_legacy::AudioSystem::DEVICE_OUT_SPEAKER)
            {
                ForceEnableSpeaker();
            }
            else
            {
                SetSpeakerStatus(false);
                EnableSpeaker();
            }
        }

        //return false;
    }
    // fm not enable or speaker is not set
    else if( GetFmSpeakerStatus()==false && GetAnalogLineinEnable() )
    {
        // only in normal mode and not in force to speaker can close speaker
        if(mMode <= android_audio_legacy::AudioSystem::MODE_NORMAL && mOuputForceuse[android_audio_legacy::AudioSystem::MODE_NORMAL] != android_audio_legacy::AudioSystem::FORCE_SPEAKER )
        {
            if(GetFmRxStatus() == true)
            {
                if(!IsHeadsetConnect())
                {
                    if(mOutputAvailable[android_audio_legacy::AudioSystem::MODE_NORMAL] & android_audio_legacy::AudioSystem::DEVICE_OUT_ALL_A2DP)
                    {   // cr ALPS00253033. play fm, connect bt, MT call, take on and then hung up, close bt.
                        mOutputAvailable[android_audio_legacy::AudioSystem::MODE_NORMAL] &= (~android_audio_legacy::AudioSystem::DEVICE_OUT_EARPIECE);
                    }
                    ForceEnableSpeaker ();
                }
                else
                {
                    EnableSpeaker();
                }
            }
            else
            {
                EnableSpeaker();
            }
        }
    }
#ifdef HEADPHONE_CONTROL
	if(GetFmSpeakerStatus()==false && (GetAnalogLineinEnable() || GetFmDigitalInEnable()) )
	{
		ForceEnableHeadPhone();
	}
#endif
    return false;
}

bool AudioYusuHardware::SetMatvSpeaker()
{
    if(mMode > android_audio_legacy::AudioSystem::MODE_NORMAL)
        return false;

    LOG_HARDWARE("SetMatvSpeaker, GetMatvAnalogEnable=%d ",GetMatvAnalogEnable());
    if(mMode <= android_audio_legacy::AudioSystem::MODE_NORMAL&&mOuputForceuse[android_audio_legacy::AudioSystem::MODE_NORMAL] != android_audio_legacy::AudioSystem::FORCE_SPEAKER )
    {
        if(GetMatvAnalogEnable () == true && GetCurrentDevice () == android_audio_legacy::AudioSystem::DEVICE_OUT_SPEAKER )
        {
            LOG_HARDWARE("SetMatvSpeaker, Device:Speaker ");
            ForceEnableSpeaker ();
        }
        else
        {
            LOG_HARDWARE("SetMatvSpeaker, GetSrcBlockRunning(%d) ",mStreamHandler->GetSrcBlockRunning());
            if(mStreamHandler->GetSrcBlockRunning())
            {
                EnableSpeaker();
            }
            else
            {
                ForceDisableSpeaker();
            }
        }
    }
    return false;
}


//static functin to get FM power state
#define BUF_LEN 1
static char rbuf[BUF_LEN] = {'\0'};
static char wbuf[BUF_LEN] = {'1'};
static const char *FM_POWER_STAUTS_PATH ="/proc/fm";
static const char *FM_DEVICE_PATH = "dev/fm";

// FM Chip MT519x_FM/MT6620_FM
bool AudioYusuHardware::Get_FMPower_info(void)
{
    int FMstatusFd = -1;
    int ret =-1;

#if defined(MT6620_FM) || defined(MT6626_FM)
    /*
       //open fm dev
       FMstatusFd = open("/dev/fm", O_RDWR);
       uint32_t pwredup = 0;
       if(FMstatusFd < 0){
         LOGE("cannot open FM deivce");
         return false ;//error message
       }

       ret = ioctl(FMstatusFd, FM_IOCTL_IS_FM_POWERED_UP, &pwredup);
       if(ret < 0){
         LOGE("ioctl to FM device error");
       }
       LOGD("Get_FMPower_info pwredup = %d",pwredup);
       close(FMstatusFd);
       return (bool)pwredup;
    */

    LOGD("MT6620 Get_FMPower_info");
    // comment by FM driver owner:
    // the ioctl to get FM power up information would spend a long time (700ms ~ 2000ms)
    // FM owner suggest to get the power information via /proc/fm
    FMstatusFd = open(FM_POWER_STAUTS_PATH, O_RDONLY,0);
    if(FMstatusFd <0)
    {
        LOGE("open %s error fd = %d",FM_POWER_STAUTS_PATH,FMstatusFd);
        return false;
    }
    if (read(FMstatusFd, rbuf, BUF_LEN) == -1)
    {
        LOGD("FMstatusFd Can't read headset");
        close(FMstatusFd);
        return false;
    }
    if (!strncmp(wbuf, rbuf, BUF_LEN))
    {
        LOGD( "FMstatusFd  state  == 1" );
        close(FMstatusFd);
        return  true;
    }
    else
    {
        LOGD("FMstatusFd return  false" );
        close(FMstatusFd);
        return  false;
    }

#elif defined(MT5192_FM) || defined(MT5193_FM)
    LOGD("MT519x_FM Get_FMPower_info (%d)",GetFmRxStatus());
    return GetFmRxStatus();
#else
    LOGD("No this FM chip ");
    return true;
#endif

}

String8 AudioYusuHardware::getParameters(const String8& keys)
{
    AudioParameter param = AudioParameter(keys);
    AudioParameter a2dpParam = AudioParameter();
    String8 value;
    String8 key;

    LOGD("+getParameters ");

    key = "GetFmEnable";
    if (param.get(key, value) == NO_ERROR)
    {
        LOGD("+getParameters mFmStatus:%d, mFmDigitalInStatus:%d ",mFmStatus,mFmDigitalInStatus);
        LOGD("+getParameters Get_FMPower_info():%d ",Get_FMPower_info());
        bool rx_status = GetFmRxStatus();
        bool fm_power_status = Get_FMPower_info();
        value = ( (rx_status && fm_power_status) ) ? "true" : "false";
        a2dpParam.add(key, value);
        param.remove(key);
    }
    key = "GetFmTxEnable";
    if (param.get(key, value) == NO_ERROR)
    {
        value = mI2STxStatus? "true" : "false";
        a2dpParam.add(key, value);
        param.remove(key);
    }
    key = "GetForceSpeakerEnable";
    if (param.get(key, value) == NO_ERROR)
    {
        value = GetForceSpeakerStatus () ? "true" : "false";
        a2dpParam.add(key, value);
        param.remove(key);
    }

    key = key_GET_HDMI_AUDIO_STATUS;
    if (param.get(key, value) == NO_ERROR)
    {
        value = (GetHDMIAudioStatus() > 0) ? "true" : "false";
        a2dpParam.add(key, value);
        param.remove(key);
    }

#if defined(MTK_DUAL_MIC_SUPPORT)
    key = keyMETA_DUAL_MIC_GET_UL_GAIN;
    if (param.get(key, value) == NO_ERROR)
    {
        char buf[32];
        int gain = 0;
        GetDualMicToolMicGain(&gain);
        sprintf(buf, "%d",gain);
        value = String8(buf);
        a2dpParam.add(key, value);
        param.remove(key);
    }
    key = keyMETA_DUAL_MIC_GET_DL_GAIN;
    if (param.get(key, value) == NO_ERROR)
    {
        char buf[32];
        int gain = 0;
        GetDualMicToolReceiverGain(&gain);
        sprintf(buf, "%d",gain);
        value = String8(buf);
        a2dpParam.add(key, value);
        param.remove(key);
    }
    key = keyMETA_DUAL_MIC_GET_HSDL_GAIN;
    if (param.get(key, value) == NO_ERROR)
    {
        char buf[32];
        int gain = 0;
        GetDualMicToolHeadsetGain(&gain);
        sprintf(buf, "%d",gain);
        value = String8(buf);
        a2dpParam.add(key, value);
        param.remove(key);
    }
    key = keyMETA_DUAL_MIC_GET_WB_SUPPORT;
    if (param.get(key, value) == NO_ERROR)
    {
        char buf[32];
        int support;
#if defined(MTK_WB_SPEECH_SUPPORT)
        support = 1;
#else
        support = 0;
#endif
        sprintf(buf, "%d",support);
        value = String8(buf);
        a2dpParam.add(key, value);
        param.remove(key);
    }

#endif

    key = keyEnableStereoOutput;
    if (param.get(key, value) == NO_ERROR) {
       value = mEnableStereoOutput? "1" : "0";
       a2dpParam.add(key, value);
       param.remove(key);
    }

#if defined(MTK_SPH_EHN_CTRL_SUPPORT)
    if (param.get(key_GET_VCE_STATUS, value) == NO_ERROR) {
       value = (mVceIsEnable == true) ? "1" : "0";
       a2dpParam.add(key_GET_VCE_STATUS, value);
       param.remove(key_GET_VCE_STATUS);
    }
#endif

    String8 keyValuePairs  = a2dpParam.toString();
    LOGD("-getParameters() %s", keyValuePairs.string());
    return keyValuePairs;
}

status_t AudioYusuHardware::getMode(int* mode)
{
    *mode = mMode;
    return NO_ERROR;
}

int AudioYusuHardware::getPreviousMode(void)
{
    return mPreviousMode;
}


// add , support EM mode setting
status_t AudioYusuHardware::GetEMParameter(void *ptr, int len)
{
    LOG_HARDWARE("GetEMParameter ");
    GetCustParamFromNV((AUDIO_CUSTOM_PARAM_STRUCT*)ptr);
    return NO_ERROR;
}

void AudioYusuHardware::DumpSpeechNBParameters(AUDIO_CUSTOM_PARAM_STRUCT sndParam)
{
    LOGD("DumpSpeechNBParameters ");
    unsigned short *temp = &sndParam.speech_common_para[0];
    LOGD("speech_common_para 0 %d %d %d %d %d %d ",
        sndParam.speech_common_para[0],*(temp+1),*(temp+2),*(temp+3),*(temp+4),*(temp+5));
    LOGD("speech_common_para 6 %d %d %d %d  %d %d ",
                *(temp+6),*(temp+7),*(temp+8),*(temp+9),*(temp+10),*(temp+11));

    for(int i=0 ; i < SPEECH_PARA_MODE_NUM ; i++){
        temp = &sndParam.speech_mode_para[i][0];
        LOGD("speech mode %d ",i);
        LOGD("mode para 0 %d %d %d %d %d %d %d %d",
            *temp,*(temp+1),*(temp+2),*(temp+3),*(temp+4),*(temp+5),*(temp+6),*(temp+7));
        LOGD("mode para 8 %d %d %d %d %d %d %d %d",
            *(temp+8),*(temp+9),*(temp+10),*(temp+11),*(temp+12),*(temp+13),*(temp+14),*(temp+15));
    }

    temp = &sndParam.debug_info[0];
    LOGD("debug_info 0 %d %d %d %d %d %d %d %d ",
        *temp,*(temp+1),*(temp+2),*(temp+3),*(temp+4),*(temp+5) ,*(temp+6),*(temp+7));
    LOGD("debug_info 8 %d %d %d %d %d %d %d %d",
        *(temp+8),*(temp+9),*(temp+10),*(temp+11),*(temp+12),*(temp+13),*(temp+14),*(temp+15));

    for (int i=0 ; i < NB_FIR_INDEX_NUM ; i++){
        short *temp = &sndParam.sph_in_fir[i][0];
        LOGD("sph_in_fir mode %d",i);
        LOGD("sph_in_fir 0 %d %d %d %d %d %d %d %d %d %d ",
            *temp,*(temp+1),*(temp+2),*(temp+3),*(temp+4),*(temp+5) ,*(temp+6),*(temp+7),*(temp+8),*(temp+9));
        LOGD("sph_in_fir 10 %d %d %d %d %d %d %d %d %d %d ",
            *(temp+10),*(temp+11),*(temp+12),*(temp+13),*(temp+14),*(temp+15) ,*(temp+16),*(temp+17),*(temp+18),*(temp+19));
        LOGD("sph_in_fir 20 %d %d %d %d %d %d %d %d %d %d ",
            *(temp+20),*(temp+21),*(temp+22),*(temp+23),*(temp+24),*(temp+25) ,*(temp+26),*(temp+27),*(temp+28),*(temp+29));
        LOGD("sph_in_fir 30 %d %d %d %d %d %d %d %d %d %d ",
            *(temp+30),*(temp+31),*(temp+32),*(temp+33),*(temp+34),*(temp+35) ,*(temp+36),*(temp+37),*(temp+38),*(temp+39));
        LOGD("sph_in_fir 40 %d %d %d %d %d",
            *(temp+40),*(temp+41),*(temp+42),*(temp+43),*(temp+44));
    }

    for (int i=0 ; i < NB_FIR_INDEX_NUM ; i++){
        short *temp = &sndParam.sph_out_fir[i][0];
        LOGD("sph_out_fir mode %d",i);
        LOGD("sph_out_fir 0 %d %d %d %d %d %d %d %d %d %d ",
            *temp,*(temp+1),*(temp+2),*(temp+3),*(temp+4),*(temp+5) ,*(temp+6),*(temp+7),*(temp+8),*(temp+9));
        LOGD("sph_out_fir 10 %d %d %d %d %d %d %d %d %d %d ",
            *(temp+10),*(temp+11),*(temp+12),*(temp+13),*(temp+14),*(temp+15) ,*(temp+16),*(temp+17),*(temp+18),*(temp+19));
        LOGD("sph_out_fir 20 %d %d %d %d %d %d %d %d %d %d ",
            *(temp+20),*(temp+21),*(temp+22),*(temp+23),*(temp+24),*(temp+25) ,*(temp+26),*(temp+27),*(temp+28),*(temp+29));
        LOGD("sph_out_fir 30 %d %d %d %d %d %d %d %d %d %d ",
            *(temp+30),*(temp+31),*(temp+32),*(temp+33),*(temp+34),*(temp+35) ,*(temp+36),*(temp+37),*(temp+38),*(temp+39));
        LOGD("sph_out_fir 40 %d %d %d %d %d",
            *(temp+40),*(temp+41),*(temp+42),*(temp+43),*(temp+44));

    }
}

void AudioYusuHardware::DumpSpeechWBParameters(AUDIO_CUSTOM_WB_PARAM_STRUCT sndWbParam )
{
    LOGD("DumpSpeechWBParameters ");
    unsigned short *temp = &sndWbParam.speech_mode_wb_para[0][0];

    for(int i=0 ; i < SPEECH_PARA_MODE_NUM ; i++){
        temp = &sndWbParam.speech_mode_wb_para[i][0];
        LOGD("sndWbParam mode %d ",i);
        LOGD("sndWbParam para 0 %d %d %d %d %d %d %d %d",
            *temp,*(temp+1),*(temp+2),*(temp+3),*(temp+4),*(temp+5),*(temp+6),*(temp+7));
        LOGD("sndWbParam para 8 %d %d %d %d %d %d %d %d",
            *(temp+8),*(temp+9),*(temp+10),*(temp+11),*(temp+12),*(temp+13),*(temp+14),*(temp+15));
    }


    for (int i=0 ; i < WB_FIR_INDEX_NUM ; i++){
        short *temp = &sndWbParam.sph_wb_in_fir[i][0];
        LOGD("sph_wb_in_fir mode %d",i);
        LOGD("sph_wb_in_fir 0 %d %d %d %d %d %d %d %d %d %d ",
            *temp,*(temp+1),*(temp+2),*(temp+3),*(temp+4),*(temp+5) ,*(temp+6),*(temp+7),*(temp+8),*(temp+9));
        LOGD("sph_wb_in_fir 10 %d %d %d %d %d %d %d %d %d %d ",
            *(temp+10),*(temp+11),*(temp+12),*(temp+13),*(temp+14),*(temp+15) ,*(temp+16),*(temp+17),*(temp+18),*(temp+19));
        LOGD("sph_wb_in_fir 20 %d %d %d %d %d %d %d %d %d %d ",
            *(temp+20),*(temp+21),*(temp+22),*(temp+23),*(temp+24),*(temp+25) ,*(temp+26),*(temp+27),*(temp+28),*(temp+29));
        LOGD("sph_wb_in_fir 30 %d %d %d %d %d %d %d %d %d %d ",
            *(temp+30),*(temp+31),*(temp+32),*(temp+33),*(temp+34),*(temp+35) ,*(temp+36),*(temp+37),*(temp+38),*(temp+39));
        LOGD("sph_wb_in_fir 40 %d %d %d %d %d %d %d %d %d %d ",
            *(temp+40),*(temp+41),*(temp+42),*(temp+43),*(temp+44),*(temp+45) ,*(temp+46),*(temp+47),*(temp+48),*(temp+49));
        LOGD("sph_wb_in_fir 50 %d %d %d %d %d %d %d %d %d %d ",
            *(temp+50),*(temp+51),*(temp+52),*(temp+53),*(temp+54),*(temp+55) ,*(temp+56),*(temp+57),*(temp+58),*(temp+59));
        LOGD("sph_wb_in_fir 60 %d %d %d %d %d %d %d %d %d %d ",
            *(temp+60),*(temp+61),*(temp+62),*(temp+63),*(temp+64),*(temp+65) ,*(temp+66),*(temp+67),*(temp+68),*(temp+69));
        LOGD("sph_wb_in_fir 70 %d %d %d %d %d %d %d %d %d %d ",
            *(temp+70),*(temp+71),*(temp+72),*(temp+73),*(temp+74),*(temp+75) ,*(temp+76),*(temp+77),*(temp+78),*(temp+79));
        LOGD("sph_wb_in_fir 80 %d %d %d %d %d %d %d %d %d %d ",
            *(temp+80),*(temp+81),*(temp+82),*(temp+83),*(temp+84),*(temp+85) ,*(temp+86),*(temp+87),*(temp+88),*(temp+89));
    }

    for (int i=0 ; i < WB_FIR_INDEX_NUM ; i++){
        short *temp = &sndWbParam.sph_wb_out_fir[i][0];
        LOGD("sph_wb_out_fir mode %d",i);
        LOGD("sph_wb_out_fir 0 %d %d %d %d %d %d %d %d %d %d ",
            *temp,*(temp+1),*(temp+2),*(temp+3),*(temp+4),*(temp+5) ,*(temp+6),*(temp+7),*(temp+8),*(temp+9));
        LOGD("sph_wb_out_fir 10 %d %d %d %d %d %d %d %d %d %d ",
            *(temp+10),*(temp+11),*(temp+12),*(temp+13),*(temp+14),*(temp+15) ,*(temp+16),*(temp+17),*(temp+18),*(temp+19));
        LOGD("sph_wb_out_fir 20 %d %d %d %d %d %d %d %d %d %d ",
            *(temp+20),*(temp+21),*(temp+22),*(temp+23),*(temp+24),*(temp+25) ,*(temp+26),*(temp+27),*(temp+28),*(temp+29));
        LOGD("sph_wb_out_fir 30 %d %d %d %d %d %d %d %d %d %d ",
            *(temp+30),*(temp+31),*(temp+32),*(temp+33),*(temp+34),*(temp+35) ,*(temp+36),*(temp+37),*(temp+38),*(temp+39));
        LOGD("sph_wb_out_fir 40 %d %d %d %d %d %d %d %d %d %d ",
            *(temp+40),*(temp+41),*(temp+42),*(temp+43),*(temp+44),*(temp+45) ,*(temp+46),*(temp+47),*(temp+48),*(temp+49));
        LOGD("sph_wb_out_fir 50 %d %d %d %d %d %d %d %d %d %d ",
            *(temp+50),*(temp+51),*(temp+52),*(temp+53),*(temp+54),*(temp+55) ,*(temp+56),*(temp+57),*(temp+58),*(temp+59));
        LOGD("sph_wb_out_fir 60 %d %d %d %d %d %d %d %d %d %d ",
            *(temp+60),*(temp+61),*(temp+62),*(temp+63),*(temp+64),*(temp+65) ,*(temp+66),*(temp+67),*(temp+68),*(temp+69));
        LOGD("sph_wb_out_fir 70 %d %d %d %d %d %d %d %d %d %d ",
            *(temp+70),*(temp+71),*(temp+72),*(temp+73),*(temp+74),*(temp+75) ,*(temp+76),*(temp+77),*(temp+78),*(temp+79));
        LOGD("sph_wb_out_fir 80 %d %d %d %d %d %d %d %d %d %d ",
            *(temp+80),*(temp+81),*(temp+82),*(temp+83),*(temp+84),*(temp+85) ,*(temp+86),*(temp+87),*(temp+88),*(temp+89));
    }

}

void AudioYusuHardware::DumpSpeechCoefficient()
{
    AUDIO_CUSTOM_PARAM_STRUCT sndParam;
    #if defined(MTK_WB_SPEECH_SUPPORT)
    AUDIO_CUSTOM_WB_PARAM_STRUCT sndWbParam;
    #endif

    GetCustParamFromNV(&sndParam);
    DumpSpeechNBParameters(sndParam);

#if defined(MTK_WB_SPEECH_SUPPORT)
    GetCustWBParamFromNV(&sndWbParam);
    DumpSpeechWBParameters(sndWbParam);
#endif
    if(GetVmFlag ()){
        DumpSpeechParatoFile();
    }
}

void AudioYusuHardware::DumpSpeechParatoFile()
{
    return;

    AUDIO_CUSTOM_PARAM_STRUCT sndParam;
    #if defined(MTK_WB_SPEECH_SUPPORT)
    AUDIO_CUSTOM_WB_PARAM_STRUCT sndWbParam;
    #endif

    GetCustParamFromNV(&sndParam);
    DumpSpeechNBParameters(sndParam);

#if defined(MTK_WB_SPEECH_SUPPORT)
    GetCustWBParamFromNV(&sndWbParam);
    DumpSpeechWBParameters(sndWbParam);
#endif

}


status_t AudioYusuHardware::SetEMParameter(void *ptr, int len)
{
    LOG_HARDWARE("SetEMParameter ");
    SetCustParamToNV((AUDIO_CUSTOM_PARAM_STRUCT*)ptr);
    mVolumeController->InitVolumeController();

    LOGD("+LAD_SetSpeechParameters to modem");
    pLad->LAD_SetSpeechParameters((AUDIO_CUSTOM_PARAM_STRUCT*)ptr);
    LOGD("-LAD_SetSpeechParameters to modem");

#if defined(MTK_DUAL_MIC_SUPPORT)
      AUDIO_CUSTOM_EXTRA_PARAM_STRUCT dualMicParam;
#endif
#if defined(MTK_WB_SPEECH_SUPPORT)
      AUDIO_CUSTOM_WB_PARAM_STRUCT sndWbParam;
#endif

#if defined(MTK_DUAL_MIC_SUPPORT)
      LOGD("SendSphParaThread, M2A_MD_Reset_Notify, Dual Mic");
      Read_DualMic_CustomParam_From_NVRAM(&dualMicParam);
      pLad->LAD_SetDualMicParameters(&(dualMicParam));
#endif
#if defined(MTK_WB_SPEECH_SUPPORT)
      LOGD("SendSphParaThread, M2A_MD_Reset_Notify, WB SPH");
      GetCustWBParamFromNV(&sndWbParam);
      pLad->LAD_SetWBSpeechParameters(&(sndWbParam));
#endif
    DumpSpeechCoefficient();
    return NO_ERROR;
}
status_t AudioYusuHardware::SetFMTX_SineWave(bool Enable, uint16 Freq)
{
    mAudFtm->WavGen_SW_SineWave(Enable, Freq, 0);

    return NO_ERROR;
}

bool AudioYusuHardware::UpdateOutputFIR(int mode , int index)
{
    LOGD("UpdateOutputFIR  mode = %d index = %d",mode,index);
    AUDIO_CUSTOM_PARAM_STRUCT pCustomPara;
    AUDIO_PARAM_MED_STRUCT pMedPara;

    GetCustParamFromNV(&pCustomPara);
    GetMedParamFromNV(&pMedPara);
    //save  index to MED with different mode.
    pMedPara.select_FIR_output_index[mode]= index;
    for(int i=0; i < NB_FIR_NUM ;  i++)
    {
        LOGD("GetCustParamFromNV [%d][%d] = %d",mode,index,pCustomPara.sph_out_fir[mode][i]);
    }
    for(int i=0; i < NB_FIR_NUM ;  i++)
    {
        LOGD("GetCustParamFromNV [%d][%d][%d] = %d",mode,index,i,pMedPara.speech_output_FIR_coeffs[mode][index][i]);
    }
    //copy med data into audio_custom param
    memcpy((void*)&pCustomPara.sph_out_fir[mode][0],(void*)&pMedPara.speech_output_FIR_coeffs[mode][index][Normal_Coef_Index],sizeof(pCustomPara.sph_out_fir[mode]));

    for(int i=0; i < NB_FIR_NUM ;  i++)
    {
        LOGD("GetCustParamFromNV [%d][%d] = %d",mode,i,pCustomPara.sph_out_fir[mode][i]);
    }
    for(int i=0; i < NB_FIR_NUM ;  i++)
    {
        LOGD("GetCustParamFromNV [%d][%d][%d] = %d",mode,index,i,pMedPara.speech_output_FIR_coeffs[mode][index][i]);
    }
    // set to modem side
    pLad->LAD_SetSpeechParameters(&pCustomPara);
    SetCustParamToNV(&pCustomPara);
    SetMedParamToNV(&pMedPara);
    DumpSpeechCoefficient();
    return true;
}

status_t AudioYusuHardware::SetAudioCommand(int par1, int par2)
{
    LOGD("SetAudioCommand par1 = %d, par2 = %d",par1,par2);
    switch(par1)
    {
    case SETOUTPUTFIRINDEX:
    {
        UpdateOutputFIR(Normal_Coef_Index,par2);
        break;
    }
    case START_FMTX_SINEWAVE:
    {
        mAnaReg->AnalogAFE_Open(AUDIO_PATH);
        return SetFMTX_SineWave(true, par2);
    }
    case STOP_FMTX_SINEWAVE:
    {
        mAnaReg->AnalogAFE_Close(AUDIO_PATH);
        return SetFMTX_SineWave(false, 0);
    }
    case SETNORMALOUTPUTFIRINDEX:
    {
        //=========================================
        //==  Test Code
#if defined(MTK_DUAL_MIC_SUPPORT)||defined(MTK_AUDIO_HD_REC_SUPPORT)
        LOGD(" Test Code:%d", par2);
        if ( par2 == 1 )
        {
            mDualMicMetaWB = TRUE;
            Enable_Modem_2Way_Dual_Mic(TRUE, DMMETA_RECEIVER_OUT, mDualMicMetaWB, DMMETA_2WAY_RECONLY);
        }
        else if ( par2 == 0 )
        {
            Enable_Modem_2Way_Dual_Mic(FALSE, DMMETA_RECEIVER_OUT, FALSE, DMMETA_2WAY_RECONLY);
            mDualMicMetaWB = FALSE;
        }
        else if ( par2 == 3 )
        {
            mDualMicMetaWB = TRUE;
            Enable_Modem_2Way_Dual_Mic(TRUE, DMMETA_HEADSET_OUT, mDualMicMetaWB, DMMETA_2WAY_NORMAL);
        }
        else if ( par2 == 2 )
        {
            Enable_Modem_2Way_Dual_Mic(FALSE, DMMETA_HEADSET_OUT, TRUE, DMMETA_2WAY_NORMAL);
            mDualMicMetaWB = FALSE;
        }
        else if ( par2 == 5 )
        {
            SetDualMicToolHeadsetGain(3);
        }
        else if ( par2 == 4 )
        {
            SetDualMicToolHeadsetGain(-32);
        }
#endif

        UpdateOutputFIR(Normal_Coef_Index,par2);

        break;
    }
    case SETHEADSETOUTPUTFIRINDEX:
    {
        UpdateOutputFIR(Headset_Coef_Index,par2);
        break;
    }
    case SETSPEAKEROUTPUTFIRINDEX:
    {
        UpdateOutputFIR(Handfree_Coef_Index,par2);
        break;
    }
    case SET_LOAD_VOLUME_SETTING:
    {
        const sp<IAudioPolicyService>& aps = AudioSystem::get_audio_policy_service();
        if (aps == 0) return PERMISSION_DENIED;
        aps->SetPolicyManagerParameters (POLICY_LOAD_VOLUME,0, 0,0);
        mVolumeController->InitVolumeController ();
        setMasterVolume (mVolumeController->getMasterVolume ());
        break;
    }
    default:
        break;
    }
    return NO_ERROR;
}

status_t AudioYusuHardware::GetAudioCommand(int parameters1)
{
    LOGD("GetAudioCommand parameters1 = %d ",parameters1);
    int result = 0 ;
    switch(parameters1)
    {
    case GETOUTPUTFIRINDEX:
    {
        AUDIO_PARAM_MED_STRUCT pMedPara;
        GetMedParamFromNV(&pMedPara);
        result = pMedPara.select_FIR_output_index[Normal_Coef_Index];
        break;
    }
    case GETAUDIOCUSTOMDATASIZE:
    {
        int AudioCustomDataSize = sizeof(AUDIO_VOLUME_CUSTOM_STRUCT);
        LOGD("GETAUDIOCUSTOMDATASIZE  AudioCustomDataSize = %d",AudioCustomDataSize);
        return AudioCustomDataSize;
    }
    case GETNORMALOUTPUTFIRINDEX:
    {
        AUDIO_PARAM_MED_STRUCT pMedPara;
        GetMedParamFromNV(&pMedPara);
        result = pMedPara.select_FIR_output_index[Normal_Coef_Index];
        break;
    }
    case GETHEADSETOUTPUTFIRINDEX:
    {
        AUDIO_PARAM_MED_STRUCT pMedPara;
        GetMedParamFromNV(&pMedPara);
        result = pMedPara.select_FIR_output_index[Headset_Coef_Index];
        break;
    }
    case GETSPEAKEROUTPUTFIRINDEX:
    {
        AUDIO_PARAM_MED_STRUCT pMedPara;
        GetMedParamFromNV(&pMedPara);
        result = pMedPara.select_FIR_output_index[Handfree_Coef_Index];
        break;
    }
    default:
        break;
    }
    // call fucntion want to get status adn return it.
    return result;
}

status_t AudioYusuHardware::SetAudioData(int par1,size_t len,void *ptr)
{
    LOGD("SetAudioData par1=%d, len=%d",par1,len);
    switch(par1)
    {
    case SETMEDDATA:
    {
        AUDIO_PARAM_MED_STRUCT pMedPara;
        memcpy((void*)&pMedPara,ptr,sizeof(AUDIO_PARAM_MED_STRUCT));
        SetMedParamToNV(&pMedPara);
        break;
    }
    case SETAUDIOCUSTOMDATA:
    {
        AUDIO_VOLUME_CUSTOM_STRUCT pAudioCustomPara;
        memcpy((void*)&pAudioCustomPara,ptr,sizeof(AUDIO_VOLUME_CUSTOM_STRUCT));
        SetAudioCustomParamToNV(&pAudioCustomPara);
        mVolumeController->InitVolumeController();
        setMasterVolume (mVolumeController->getMasterVolume ());
        // fix me , another way to tell policymanager load volume.
        //AudioYusuPolicyManager::LoadCustomVolume ();

        const sp<IAudioPolicyService>& aps = AudioSystem::get_audio_policy_service();
        if (aps == 0) return PERMISSION_DENIED;
        aps->SetPolicyManagerParameters (POLICY_LOAD_VOLUME,0, 0,0);
        break;
    }
#if defined(MTK_DUAL_MIC_SUPPORT)
    case SET_DUAL_MIC_PARAMETER:
    {
        AUDIO_CUSTOM_EXTRA_PARAM_STRUCT dualMicParam;
        memcpy((void*)&dualMicParam,ptr,sizeof(AUDIO_CUSTOM_EXTRA_PARAM_STRUCT));
        Write_DualMic_CustomParam_To_NVRAM(&dualMicParam);
        mVolumeController->InitVolumeController();

        LOGD("+LAD_SetDualMicParameters to modem");
        pLad->LAD_SetDualMicParameters((AUDIO_CUSTOM_EXTRA_PARAM_STRUCT*)ptr);
        LOGD("-LAD_SetDualMicParameters to modem");
        break;
    }
#endif

#if defined(MTK_WB_SPEECH_SUPPORT)
    case SET_WB_SPEECH_PARAMETER:
    {
        AUDIO_CUSTOM_WB_PARAM_STRUCT wbParam;
        memcpy((void*)&wbParam,ptr,sizeof(AUDIO_CUSTOM_WB_PARAM_STRUCT));
        SetCustWBParamToNV(&wbParam);
        LOGD("+LAD_SetWBSpeechParameters to modem");
        pLad->LAD_SetWBSpeechParameters((AUDIO_CUSTOM_WB_PARAM_STRUCT*)ptr);
        LOGD("-LAD_SetWBSpeechParameters to modem");
        break;
    }
#endif

    default:
        break;
    }
    return NO_ERROR;
}

status_t AudioYusuHardware::GetAudioData(int par1,size_t len,void *ptr)
{
    LOGD("GetAudioData par1=%d, len=%d",par1,len);
    switch(par1)
    {
    case GETMEDDATA:
    {
        AUDIO_PARAM_MED_STRUCT pMedPara;
        GetMedParamFromNV(&pMedPara);
        memcpy(ptr,(void*)&pMedPara,sizeof(AUDIO_PARAM_MED_STRUCT));
        break;
    }
    case GETAUDIOCUSTOMDATA:
    {
        AUDIO_VOLUME_CUSTOM_STRUCT pAudioCustomPara;
        GetAudioCustomParamFromNV(&pAudioCustomPara);
        memcpy(ptr,(void*)&pAudioCustomPara,sizeof(AUDIO_VOLUME_CUSTOM_STRUCT));
        break;
    }
#if defined(MTK_DUAL_MIC_SUPPORT)
    case GET_DUAL_MIC_PARAMETER:
    {
        AUDIO_CUSTOM_EXTRA_PARAM_STRUCT dualMicParam;
        Read_DualMic_CustomParam_From_NVRAM(&dualMicParam);
        memcpy(ptr,(void*)&dualMicParam,sizeof(AUDIO_CUSTOM_EXTRA_PARAM_STRUCT));
        break;
    }
#endif
#if defined(MTK_WB_SPEECH_SUPPORT)
    case GET_WB_SPEECH_PARAMETER:
    {
        AUDIO_CUSTOM_WB_PARAM_STRUCT wbParam;
        GetCustWBParamFromNV(&wbParam);
        memcpy(ptr,(void*)&wbParam,sizeof(AUDIO_CUSTOM_WB_PARAM_STRUCT));
        break;
    }
#endif
    default:
        break;
    }
    return NO_ERROR;
}


status_t AudioYusuHardware::SetACFPreviewParameter(void *ptr, int len)
{
    LOG_HARDWARE("SetACFPreviewParameter ");
    LOGD("AudioYusuHardware::SetACFPreviewParameter!! ");
    mStreamHandler->mOutput[0]->SetACFPreviewParameter(ptr,len);
    return NO_ERROR;
}

status_t AudioYusuHardware::SetHCFPreviewParameter(void *ptr, int len)
{
    LOG_HARDWARE("SetHCFPreviewParameter ");
    LOGD("AudioYusuHardware::SetHCFPreviewParameter!! ");
    mStreamHandler->mOutput[0]->SetHCFPreviewParameter(ptr,len);
    return NO_ERROR;
}

bool AudioYusuHardware::GetVmFlag(void)
{
    //LOG_HARDWARE("GetVmFlag= %d",mVolumeController->sndParam.bAutoVM);
#if defined(FORCE_VM_ENABLE)
    return true;
#else
    //return mVolumeController->sndParam.bAutoVM;
    return (mVolumeController->sndParam.bAutoVM & MODEM_REC_VM_MASK);
#endif
}

bool AudioYusuHardware::GetCtm4WayFlag(void)
{
    LOG_HARDWARE("GetCtm4WayFlag= %d",mVolumeController->sndParam.bAutoVM);
    //return false;

    //cannot support VM and CTM4way record at the same time
    if (mVolumeController->sndParam.bAutoVM & MODEM_REC_VM_MASK)
        return false;

    return (mVolumeController->sndParam.bAutoVM & MODEM_REC_CTM4WAY_MASK);
}

#if defined(MTK_DUAL_MIC_SUPPORT)||defined(MTK_AUDIO_HD_REC_SUPPORT)

static void *Dual_Mic_SDCard_Routine(void *arg)
{
    AudioYusuHardware *mHw = (AudioYusuHardware*)arg;
    if(mHw == NULL)
    {
        LOGE("Dual_Mic_SDCard_Routine mHw = NULL arg = %x",arg);
        return 0;
    }

    // open AudioRecord
    pthread_mutex_lock(&mHw->mDmsdMutex);

    LOGD("+Dual_Mic_SDCard_Routine");


    // Adjust thread priority
    prctl(PR_SET_NAME, (unsigned long)"DualMicCalibrationRoutine", 0, 0, 0);
    setpriority(PRIO_PROCESS, 0, ANDROID_PRIORITY_AUDIO);
    //Prepare file pointer
    FILE* inFp = mHw->m_DualMic_pInFile;                 //file for input
    FILE* outFp = mHw->m_DualMic_pOutFile;                 //file for input

    // Prepare input & output memory
    rb* playDmsdBuf = &mHw->playDmsdBuf;
    playDmsdBuf->pBufBase = new char[DMSD_PLAYBUF_SIZE];
    playDmsdBuf->pRead    = playDmsdBuf->pBufBase;
    playDmsdBuf->pWrite   = playDmsdBuf->pBufBase;
    memset((void*)playDmsdBuf->pBufBase , 0, DMSD_PLAYBUF_SIZE);
    playDmsdBuf->bufLen = DMSD_PLAYBUF_SIZE;

    rb* recDmsdBuf = &mHw->recDmsdBuf;
    recDmsdBuf->pBufBase = new char[DMSD_RECBUF_SIZE];
    memset((void*)recDmsdBuf->pBufBase , 0, DMSD_RECBUF_SIZE);
    recDmsdBuf->pRead    = recDmsdBuf->pBufBase;
    recDmsdBuf->pWrite   = recDmsdBuf->pBufBase;
    recDmsdBuf->bufLen = DMSD_RECBUF_SIZE;

    // ----start the loop --------
    mHw->DmsdThreadExit = false;
    int numOfBytesPlay =0;
    int numOfBytesRec =0;
    int totalBytes =0;
    char* tmpOut;
    char* tmpIn;

    LOGD("pthread_cond_signal(&mHw->mMdExit_Cond)");
    pthread_cond_signal(&mHw->mDmsdExit_Cond); // wake all thread
    pthread_mutex_unlock(&mHw->mDmsdMutex);

    while((!mHw->DmsdThreadExit) && inFp && outFp)
    {

        pthread_mutex_lock(&mHw->mDmsdBufMutex);

        //handling playback buffer
        int playBufFreeCnt = playDmsdBuf->bufLen - rb_getDataCount(playDmsdBuf) -1;
        tmpOut = new char[playBufFreeCnt];
        int cntR = fread(tmpOut, sizeof(char), playBufFreeCnt, inFp);
        rb_copyFromLinear(playDmsdBuf, tmpOut, cntR);
        numOfBytesPlay += cntR;
        delete[] tmpOut;
        LOGD(" Playback buffer, free:%d, read from SD:%d, total play:%d", playBufFreeCnt, cntR, numOfBytesPlay);

        // handling record buffer
        int recBufDataCnt = rb_getDataCount(recDmsdBuf);
        tmpIn = new char[recBufDataCnt];
        rb_copyToLinear(tmpIn, recDmsdBuf, recBufDataCnt);
        int cntW = fwrite((void*)tmpIn,sizeof(char),recBufDataCnt,outFp);
        numOfBytesRec += cntW;
        delete[] tmpIn;
        LOGD(" Record buffer, available:%d, write SD:%d, total rec:%d", recBufDataCnt, cntW, numOfBytesRec);

        pthread_mutex_unlock(&mHw->mDmsdBufMutex);

        usleep(200*1000);

    }

    // free buffer
    pthread_mutex_lock(&mHw->mDmsdBufMutex);
    delete[] playDmsdBuf->pBufBase;
    delete[] recDmsdBuf->pBufBase;
    playDmsdBuf->pBufBase = NULL;
    recDmsdBuf->pBufBase = NULL;
    pthread_mutex_unlock(&mHw->mDmsdBufMutex);

    //exit thread
    LOGD( "VmRecordRoutine pthread_mutex_lock");
    pthread_mutex_lock(&mHw->mDmsdMutex);
    LOGD("pthread_cond_signal(&mDmsdExit_Cond)");
    pthread_cond_signal(&mHw->mDmsdExit_Cond); // wake all thread
    pthread_mutex_unlock(&mHw->mDmsdMutex);

    return 0;
}

int AudioYusuHardware::Get_NVRAM_DLGain_For_DualMic(int index)
{
    AUDIO_VOLUME_CUSTOM_STRUCT VolumeCustomParam;//volume custom data
    GetAudioCustomParamFromNV(&VolumeCustomParam);
    int gain, dbGain;
    float fdbGain;

    if ( index == 0 )    // earpiece
    {
        gain = VolumeCustomParam.audiovolume_sph[VOL_NORMAL][CUSTOM_VOLUME_STEP-1];
    }
    else if ( index == 1 )    //headset
    {
        gain = VolumeCustomParam.audiovolume_sph[VOL_HEADSET][3];
    }

    fdbGain = 11.5 - (176.0 - (float)gain) / 4;
    dbGain = (int)fdbGain;

    LOGD("Get_NVRAM_DLGain_For_DualMic: Index: %d,  gain: %d, fdbGain:%f, dbGain: %d", index, gain, fdbGain, dbGain);

    return dbGain;
}

int AudioYusuHardware::Get_NVRAM_ULGain_For_DualMic(int index)
{
    AUDIO_VOLUME_CUSTOM_STRUCT VolumeCustomParam;//volume custom data
    GetAudioCustomParamFromNV(&VolumeCustomParam);
    int gain, dbGain;
    float fdbGain;

    gain = VolumeCustomParam.audiovolume_mic[VOL_NORMAL][3];

    if(gain > UPLINK_GAIN_MAX){
        gain = UPLINK_GAIN_MAX;
    }else if(gain<0){
        gain = 0;
    }
    fdbGain = 49.0 - (float)(UPLINK_GAIN_MAX - gain)/4;  // how many dB degrade
    dbGain = (int)fdbGain;

    LOGD("Get_NVRAM_ULGain_For_DualMic: Index: %d,  gain: %d, fdbGain:%f, dbGain: %d", index, gain, fdbGain, dbGain);

    return dbGain;
}

void AudioYusuHardware::MMI_Set_Dual_Mic_Enable(int value)
{
    LOG_HARDWARE("MMI_Set_Dual_Mic_Enable(%d)",value);

    int sp_flag;
    sp_flag = Get_Recovery_Speech();
    // if user change DualMic on/off during phone call
    // 0: off
    // 1: on
    /*
    if((mMode == android_audio_legacy::AudioSystem::MODE_IN_CALL))
    {
       mMMI_DualMic_Setting = value;
    }
    else
    {
       mMMI_DualMic_Setting = value;
       mHAL_DualMic_Setting = value;
    }
    */

    mHAL_DualMic_Setting = value;
    if(mMode == android_audio_legacy::AudioSystem::MODE_IN_CALL)
    {
        doOutputDeviceRouting();   // enable the dual mic setting during call
    }

    // update the setting in file system
    FILE* dmFp;
    dmFp = fopen("/data/system/dualmiccfg","wb");
    if(dmFp) fwrite(&value, sizeof(int), 1, dmFp);
    else LOGE("cannot open file");
    if(dmFp) chmod("/data/system/dualmiccfg", 0770);
    if(dmFp) fclose(dmFp);
    //usleep(100*1000);
    LOG_HARDWARE("dual mic set to file: %d", value);

}

void AudioYusuHardware::META_Set_DualMic_Test_Mode(int value)
{
    LOG_HARDWARE("META_Set_DualMic_Test_Mode(%d)",value);
    mDualMic_Test = value;
}

bool AudioYusuHardware::META_Get_DualMic_Test_Mode(void)
{
    LOG_HARDWARE("META_Get_DualMic_Test_Mode(%d)",mDualMic_Test);
    return mDualMic_Test;
}

void AudioYusuHardware::META_Set_DualMic_WB(int value)
{
    LOG_HARDWARE("META_Set_DualMic_WB(%d)",value);
#if defined(MTK_WB_SPEECH_SUPPORT)
    mDualMicMetaWB = value;
#else
    LOGE("META_Set_DualMic_WB always set to false due to WB speech option not enable!!");
    mDualMicMetaWB = FALSE;
#endif
}

bool AudioYusuHardware::META_Get_DualMic_WB(void)
{
    LOG_HARDWARE("META_Get_DualMic_WB(%d)",mDualMic_Test);
    return mDualMicMetaWB;
}


void AudioYusuHardware::Enable_Modem_Dual_Mic_Rec(bool Enable)
{
    LOG_HARDWARE("Enable_Modem_Dual_Mic_Rec(%d) ",Enable);
    pthread_mutex_lock(&dmicMutex);

    if(TRUE==Enable)
    {
        META_Set_DualMic_Test_Mode(1);
        // open output file
        if(m_str_pOutFile != NULL)
        {
            LOG_HARDWARE("[Dual-Mic] open output file filename:%s ", m_str_pOutFile);
            m_DualMic_pOutFile = fopen(m_str_pOutFile,"wb");
        }
        else
        {
            char defaultOutFileName[] = "/sdcard/DualMic_Rec_default_out.vm";
            LOG_HARDWARE("[Dual-Mic] m_str_pOutFile is NULL, open with default filename:%s ", defaultOutFileName);
            m_DualMic_pOutFile = fopen(defaultOutFileName,"wb");
        }
        if( m_DualMic_pOutFile == NULL)
        {
            LOGE("[Dual-Mic] Fail to Open Output File");
            pthread_mutex_unlock(&dmicMutex);
            return;
        }

        //start record
        Enable_Modem_Record(TRUE);
    }
    else
    {
        //stop record
        Enable_Modem_Record(FALSE);

        //close output file
        if( m_DualMic_pOutFile ) fclose(m_DualMic_pOutFile);

        doOutputDeviceRouting();
        doInputDeviceRouting();

        META_Set_DualMic_Test_Mode(0);
    }
    pthread_mutex_unlock(&dmicMutex);

}

void AudioYusuHardware::Enable_Modem_2Way_Dual_Mic(bool Enable, int OutDevice, int WBMode, int WorkMode)
{
    LOG_HARDWARE("Enable_Modem_2Way_Dual_Mic(%d), Device:%d ", Enable, OutDevice);
#ifdef USE_DUALMIC_SDBUF
    // 3 sec for timeout
    struct timeval now;
    struct timespec timeout;
    gettimeofday(&now,NULL);
    timeout.tv_sec = now.tv_sec + 3;
    timeout.tv_nsec = now.tv_usec * 1000;
    int ret;
#endif
    pthread_mutex_lock(&dmicMutex);
    if(TRUE == Enable)
    {
        // open output file
        if(m_str_pOutFile != NULL)
        {
            LOG_HARDWARE("[Dual-Mic] open output file filename:%s ", m_str_pOutFile);
            m_DualMic_pOutFile = fopen(m_str_pOutFile,"wb");
        }
        else
        {
            char defaultOutFileName[] = "/sdcard/DualMic_PlayRec_default_out.vm";
            LOG_HARDWARE("[Dual-Mic] m_str_pOutFile is NULL, open with default filename:%s ", defaultOutFileName);
            m_DualMic_pOutFile = fopen(defaultOutFileName,"wb");
        }
        if( m_DualMic_pOutFile == NULL)
        {
            LOGE("[Dual-Mic] Fail to Open Output File");
            pthread_mutex_unlock(&dmicMutex);
            return;
        }

        //open input file
        if(m_str_pInFile != NULL)
        {
            m_DualMic_pInFile = fopen(m_str_pInFile,"rb");
            /*
              {
                  short tmp[20];
                  int rcnt;
                  fseek(m_DualMic_pInFile, 0, SEEK_SET);
                  rcnt = fread(tmp, sizeof(short), 20, m_DualMic_pInFile);
                  fseek(m_DualMic_pInFile, 0, SEEK_SET);
                  LOGD("%d: %x, %x, %x, %x, %x, %x, %x, %x", rcnt, tmp[0], tmp[1], tmp[2], tmp[3], tmp[4], tmp[5], tmp[6], tmp[7]);
                  LOGD("%x, %x, %x, %x, %x, %x, %x, %x", tmp[8], tmp[9], tmp[10], tmp[11], tmp[12], tmp[13], tmp[14], tmp[15]);
              }*/

            LOG_HARDWARE("[Dual-Mic] open input file filename:%s, fp:%p ", m_str_pInFile, m_DualMic_pInFile);

        }
        else
        {
            char defaultInFileName[] = "/sdcard/DualMic_PlayRec_default_in.pcm";
            LOG_HARDWARE("[Dual-Mic] m_str_pInFile is NULL, open or create with default filename:%s ", defaultInFileName);
            m_DualMic_pInFile = fopen(defaultInFileName,"ab+");
            LOGD("fp: %p", m_DualMic_pInFile);

        }
        if( m_DualMic_pInFile == NULL)
        {
            LOGE("[Dual-Mic] Fail to Open Input File");
            pthread_mutex_unlock(&dmicMutex);
            return;
        }
        mDualMic_Test = true;

        // do basic setting to modem side
        SwitchAudioClock(true);  // Enable the audio power
        pLad->LAD_SetInputSource(LADIN_DualAnalogMic);

        //set mic gain in dB
        uint32 ladLevel = 0xffffff - (MAX_MIC_GAIN_DB - mDualMicTool_micGain) * 2;                                                                                                           // in 0.5dB
        LOGD("Play+Rec set dual mic, mic gain: %d, ladlevel: 0x%x",mDualMicTool_micGain,ladLevel);
        pLad->LAD_SetMicrophoneVolume(ladLevel);

        //set output and output gain in dB
        if ( OutDevice == DMMETA_RECEIVER_OUT )
        {
            pLad->LAD_SetOutputDevice(LADOUT_SPEAKER1);
            ladLevel = 0xffffff - 90 * 2;
            pLad->LAD_SetSidetoneVolume(ladLevel);                         //set very low side tone
            ladLevel = 0xffffff - (MAX_RECEIVER_GAIN_DB - mDualMicTool_receiverGain) * 2;                                                                                                           // in 0.5dB
            pLad->LAD_SetOutputVolume(ladLevel);
            LOGD("Play+Rec set dual mic, receiver gain: %d, ladlevel: 0x%x",mDualMicTool_receiverGain,ladLevel);
        }
        else
        {
            pLad->LAD_SetOutputDevice(LADOUT_SPEAKER2);
            ladLevel = 0xffffff - 90 * 2;
            pLad->LAD_SetSidetoneVolume(ladLevel);                         //set very low side tone
            ladLevel = (MAX_HEADSET_GAIN_DB - mDualMicTool_headsetGain);                                                                                                           // in 0.5dB
            mAnaReg->SetAnaReg(AUDIO_CON1,ladLevel,0x3f); // left audio gain
            mAnaReg->SetAnaReg(AUDIO_CON2,ladLevel,0x3f); // right audio gain
            LOGD("Play+Rec set dual mic, headset gain: %d, ladlevel: 0x%x",mDualMicTool_headsetGain,ladLevel);
        }
#ifdef USE_DUALMIC_SDBUF
        // open buffer thread
        LOGD("open Dual_Mic_SDCard_Routine");
        pthread_mutex_lock(&mDmsdMutex);
        ret = pthread_create(&mDmsdThread, NULL,Dual_Mic_SDCard_Routine,(void*)this);
        if(ret !=0)
        {
            LOGE("Dual_Mic_SDCard_Routine pthread_create error!!");
        }
        LOGD("+mDmsdExit_Cond wait");
        ret = pthread_cond_timedwait(&mDmsdExit_Cond, &mDmsdMutex, &timeout);
        LOGD("-mDmsdExit_Cond receive ret=%d",ret);
        pthread_mutex_unlock(&mDmsdMutex);
#endif
        usleep(100*1000);

        // really enable the process
        pLad->LAD_DualMic_2WayOn(WBMode, WorkMode);
    }
    else
    {
        //pPlay2Way->Play2Way_Stop();
        //pRecord2Way->Record2Way_Stop();
#ifdef USE_DUALMIC_SDBUF
        //stop buffer thread
        LOGD("close Dual_Mic_SDCard_Routine");
        pthread_mutex_lock(&mDmsdMutex);
        if(DmsdThreadExit == true)
        {
            LOGD("VmThreadExit = %d",VmThreadExit);
        }
        else
        {
            DmsdThreadExit = true;
            LOGD("+mDmsdExit_Cond wait");
            ret = pthread_cond_timedwait(&mDmsdExit_Cond,&mDmsdMutex, &timeout);
            LOGD("-mDmsdExit_Cond receive ret=%d",ret);
        }
        pthread_mutex_unlock(&mDmsdMutex);
#endif
        pLad->LAD_DualMic_2WayOff();
        usleep(200*1000);                     //wait to make sure all message is processed

        mDualMic_Test = false;
        SwitchAudioClock(false);  // Disable the audio power

        doOutputDeviceRouting();
        doInputDeviceRouting();


        if (m_DualMic_pOutFile) fclose(m_DualMic_pOutFile);
        if (m_DualMic_pInFile) fclose(m_DualMic_pInFile);
        m_DualMic_pOutFile = NULL;
        m_DualMic_pInFile = NULL;
    }

    pthread_mutex_unlock(&dmicMutex);
    return;
}

void AudioYusuHardware::SetDualMicToolMicGain(int value)
{
    pthread_mutex_lock(&dmicMutex);
    LOG_HARDWARE("SetDualMicToolMicGain(%d) ",value);
    mDualMicTool_micGain = (value > MAX_MIC_GAIN_DB) ? MAX_MIC_GAIN_DB : value;
    mDualMicTool_micGain = (value < MIN_MIC_GAIN_DB) ? MIN_MIC_GAIN_DB : value;
    pthread_mutex_unlock(&dmicMutex);

}

void AudioYusuHardware::SetDualMicToolReceiverGain(int value)
{
    pthread_mutex_lock(&dmicMutex);
    LOG_HARDWARE("SetDualMicToolReceiverGain(%d) ",value);
    mDualMicTool_receiverGain = (value > MAX_RECEIVER_GAIN_DB) ? MAX_RECEIVER_GAIN_DB : value;
    mDualMicTool_receiverGain = (value < MIN_RECEIVER_GAIN_DB) ? MIN_RECEIVER_GAIN_DB : value;
    pthread_mutex_unlock(&dmicMutex);
}

void AudioYusuHardware::SetDualMicToolHeadsetGain(int value)
{
    pthread_mutex_lock(&dmicMutex);
    LOG_HARDWARE("SetDualMicToolHeadsetGain(%d) ",value);
    mDualMicTool_headsetGain = (value > MAX_HEADSET_GAIN_DB) ? MAX_HEADSET_GAIN_DB : value;
    mDualMicTool_headsetGain = (value < MIN_HEADSET_GAIN_DB) ? MIN_HEADSET_GAIN_DB : value;
    pthread_mutex_unlock(&dmicMutex);
}

void AudioYusuHardware::GetDualMicToolMicGain(int *value)
{
    pthread_mutex_lock(&dmicMutex);
    *value = mDualMicTool_micGain;
    LOG_HARDWARE("GetDualMicToolMicGain, got (%d) ",*value);
    pthread_mutex_unlock(&dmicMutex);
}

void AudioYusuHardware::GetDualMicToolReceiverGain(int *value)
{
    pthread_mutex_lock(&dmicMutex);
    *value = mDualMicTool_receiverGain;
    LOG_HARDWARE("GetDualMicToolReceiverGain, got (%d) ",*value);
    pthread_mutex_unlock(&dmicMutex);
}

void AudioYusuHardware::GetDualMicToolHeadsetGain(int *value)
{
    pthread_mutex_lock(&dmicMutex);
    *value = mDualMicTool_headsetGain;
    LOG_HARDWARE("GetDualMicToolHeadsetGain, got (%d) ",*value);
    pthread_mutex_unlock(&dmicMutex);
}


#endif

#if defined(MTK_SPH_EHN_CTRL_SUPPORT)
void AudioYusuHardware::MMI_Set_VCE_Enable(int value)
{
    if (value == mVceIsEnable)
    {
        LOG_HARDWARE("MMI_Set_VCE_Enable, value(%d) = mVceIsEnable(%d) return", value, mVceIsEnable);
        return;
    }

    LOGD("MMI_Set_VCE_Enable = %d", value);
    mVceIsEnable = value;

    // update VCE status
    SPH_ENH_INFO_T eSphEnhInfo = pLad->mSphEnhInfo;

    if (mVceIsEnable == true)
        eSphEnhInfo.spe_usr_subFunc_mask |= SPH_ENH_SUBFUNC_MASK_VCE;
    else
        eSphEnhInfo.spe_usr_subFunc_mask &= ~(SPH_ENH_SUBFUNC_MASK_VCE);

    pLad->LAD_Set_Speech_Enhancement_Info(eSphEnhInfo);

    // update the setting in file system
    FILE *f_vce = fopen(VCE_CFG_FILENAME, "wb");
    if (f_vce == NULL)
    {
        LOGE("cannot open VCE file %s in MMI_Set_VCE_Enable", VCE_CFG_FILENAME);
    }
    else
    {
        fwrite(&mVceIsEnable, sizeof(bool), 1, f_vce);
        fclose(f_vce);
    }
}
#endif

////////////////////////////////////////////////////////
// Record Special Format from Modem (Ex: VM, ABF(Dual-mic) )
////////////////////////////////////////////////////////
#ifdef TEMP_SOLUTION_VM
static void *Special_Format_Record_Routine(void *arg)
{
    // open AudioRecord
    pthread_mutex_lock(&mMdRecordMutex);

    LOGD("+Special_Format_Record_Routine");
	//adjust priority
    prctl(PR_SET_NAME, (unsigned long)"VMRecrdRoutine", 0, 0, 0);
    setpriority(PRIO_PROCESS, 0, ANDROID_PRIORITY_AUDIO);

    AudioYusuHardware *mHardware = (AudioYusuHardware*)arg;
    if(mHardware == NULL)
    {
        LOGE("Special_Format_Record_Routine mHardware = NULL arg = 0x%x", arg);
        return 0;
    }

    uint32_t pDevices  = 1;//AUDIO_SOURCE_MIC
    int format = android_audio_legacy::AudioSystem::VM_FMT;
    uint32_t channels = android_audio_legacy::AudioSystem::CHANNEL_IN_MONO;
    uint32_t samplingRate =8000;
    status_t status = NO_ERROR;
    uint32_t flags =    AudioRecord::RECORD_AGC_ENABLE |
                        AudioRecord::RECORD_NS_ENABLE |
                        AudioRecord::RECORD_IIR_ENABLE;


    android_audio_legacy::AudioStreamIn *input = mHardware->openInputStream(pDevices,
                           (int *)&format,
                           &channels,
                           &samplingRate,
                           &status,
                           (android_audio_legacy::AudioSystem::audio_in_acoustics)flags);

    if(input == NULL)
    {
        LOGE("open Audio record fail");
        pthread_cond_signal(&mMdExit_Cond);
        pthread_mutex_unlock(&mMdRecordMutex);
        return 0;
    }
    LOGD(" mHardware->openInputStream()");

    /*
    AudioRecord  * record = new AudioRecord(
                     1 , 8000,
                     android_audio_legacy::AudioSystem::PCM_16_BIT,
                     android_audio_legacy::AudioSystem::CHANNEL_IN_MONO,
                     4*VMRECORD_DATASIZE/sizeof(int16),0);

    status_t res = record->initCheck();
    if (res == NO_ERROR)
        res = record->start();
    else{
        if(record){
            delete record;
        }
        LOGE("open Audio record fail");
        pthread_cond_signal(&mMdExit_Cond);
        pthread_mutex_unlock(&mMdRecordMutex);
        return 0;
    }
    */
    char ReadData[VMRECORD_READSIZE];
    memset((void*)ReadData , 0,VMRECORD_READSIZE);

    FILE * pFile = NULL;
    struct tm *timeinfo;
    time_t rawtime;
    time(&rawtime);
    timeinfo=localtime(&rawtime);
    char path[80];

#if defined(MTK_DUAL_MIC_SUPPORT)||defined(MTK_AUDIO_HD_REC_SUPPORT)
    if (mHardware->mDualMic_Test)          // for dual mic case, use file opened outside
    {
        pFile = mHardware->m_DualMic_pOutFile;
        LOGD( "Dual mic case, use FP directly : %p", pFile);
    }
    else
#endif
    {
        memset((void*)path,0,80);
        strftime (path,80,"/sdcard/%a_%b_%Y__%H_%M_%S.vm", timeinfo);
        LOGD( "path is : %s", path);
        pFile = fopen(path,"w");
    }

    if(pFile == NULL)
    {
        LOGE("open File Error Try other place");
        memset((void*)path,0,80);
        strftime (path,80,"/data/%a_%b_%Y__%H_%M_%S.vm", timeinfo);
        LOGD("path is : %s", path);
        pFile = fopen(path,"w");
        if(pFile == NULL)
        {
            LOGE("open file error again");
            pthread_cond_signal(&mMdExit_Cond);
            pthread_mutex_unlock(&mMdRecordMutex);
            return 0;
        }
        else
        {
            LOGD("retry open file success");
        }
    }
    else
    {
        LOGD("open Special_Format_Record_Routine File success");
    }

    VmThreadExit = false;
    int numOfBytesRead =0;
    int numOfBytesWrite =0;
    int totalBytes =0;

    LOGD("pthread_cond_signal(&mMdExit_Cond)");
    pthread_cond_signal(&mMdExit_Cond); // wake all thread
    pthread_mutex_unlock(&mMdRecordMutex);

    while((!VmThreadExit)&&pFile)
    {
        // read 640 bytes one time
        numOfBytesRead = input->read ((void*) ReadData,VMRECORD_READSIZE);
        numOfBytesWrite = fwrite((void*)ReadData,sizeof(char),numOfBytesRead,pFile);
        totalBytes += numOfBytesWrite;
        memset((void*)ReadData , 0,VMRECORD_READSIZE);
        LOGD( "read %d bytes write %d bytes numOfBytesWrite = %d",numOfBytesRead,numOfBytesWrite,totalBytes);
        numOfBytesRead =0;
    }

    if(input)
    {
        input->standby ();
        usleep(3*1000);
        mHardware->closeInputStream(input);
        input = NULL;
        LOGD(" mHardware->closeInputStream()");
    }

    LOGD("pthread_cond_signal(&mMdExit_Cond)");
    pthread_cond_signal(&mMdExit_Cond); // wake all thread

    if(pFile)
    {
        fflush (pFile);
        fclose(pFile);
        LOGD("pFile close");
    }

    return 0;
}

void AudioYusuHardware::Enable_Modem_Record(bool Enable)
{
    int ret;
    // 3 sec for timeout
    struct timeval now;
    struct timespec timeout;
    gettimeofday(&now,NULL);
    timeout.tv_sec = now.tv_sec + 3;
    timeout.tv_nsec = now.tv_usec * 1000;

    if(true == Enable)
    {
        LOGD("Enable_Modem_Record hold wake lock");
        acquire_wake_lock(PARTIAL_WAKE_LOCK, VM_RECORD_WAKELOCK_NAME);
        pthread_mutex_lock(&mMdRecordMutex);
        ret = pthread_create(&mMdReadThread, NULL,Special_Format_Record_Routine,(void*)this);
        if(ret !=0)
        {
            LOGE("Enable_Modem_Record pthread_create error!!");
        }
        LOGD("+mMdExit_Cond wait");
        ret = pthread_cond_timedwait(&mMdExit_Cond,&mMdRecordMutex,&timeout);
        LOGD("-mMdExit_Cond receive ret=%d",ret);
        pthread_mutex_unlock(&mMdRecordMutex);
    }
    else if(false == Enable)
    {
        LOGD("close VM record");
        release_wake_lock(VM_RECORD_WAKELOCK_NAME);
        pthread_mutex_lock(&mMdRecordMutex);
        if(VmThreadExit == true)
        {
            LOGD("VmThreadExit = %d",VmThreadExit);
        }
        else
        {
            VmThreadExit = true;
            LOGD("+mMdExit_Cond wait");
            ret = pthread_cond_timedwait(&mMdExit_Cond,&mMdRecordMutex,&timeout);
            LOGD("-mMdExit_Cond receive ret=%d",ret);
        }
        pthread_mutex_unlock(&mMdRecordMutex);
    }
}

#endif

bool AudioYusuHardware::IsHeadsetConnect()
{
    LOGD("IsHeadsetConnect mOutputAvailable[MODE_NORMAL] = %d",mOutputAvailable[android_audio_legacy::AudioSystem::MODE_NORMAL]);
    return  (mOutputAvailable[android_audio_legacy::AudioSystem::MODE_NORMAL]&android_audio_legacy::AudioSystem::DEVICE_OUT_WIRED_HEADPHONE)||
            (mOutputAvailable[android_audio_legacy::AudioSystem::MODE_NORMAL]&android_audio_legacy::AudioSystem::DEVICE_OUT_WIRED_HEADSET);
}

uint32_t AudioYusuHardware::GetCurrentDevice()
{
    uint32 currentdevice = 0;
    currentdevice = Audio_Match_Output_device(mDevice ,mMode, true);
    return currentdevice;
}

uint32_t AudioYusuHardware::GetPreviousDevice()
{
    uint32 Predevice = 0;
    Predevice = Audio_Match_Output_device(mPredevice ,mPreviousMode, true);
    return Predevice;
}
/////////////////////////////////////////////////////////////////////////
// for PCMxWay Interface API
/////////////////////////////////////////////////////////////////////////
int AudioYusuHardware::xWayPlay_Start(int sample_rate)
{
    LOGD("+xWayPlay_Start");
    return pPlay2Way->Play2Way_Start(sample_rate);
}

int AudioYusuHardware::xWayPlay_Stop()
{
    LOGD("+xWayPlay_Stop");
    return pPlay2Way->Play2Way_Stop();
}

int AudioYusuHardware::xWayPlay_Write(void *buffer, int size_bytes)
{
    //LOGD("+xWayPlay_Write");
    return pPlay2Way->Play2Way_Write(buffer, size_bytes);
}

int AudioYusuHardware::xWayPlay_GetFreeBufferCount()
{
    return pPlay2Way->Play2Way_GetFreeBufferCount();
}

int AudioYusuHardware::xWayRec_Start(int sample_rate)
{
    LOGD("+xWayRec_Start");
    return pRecord2Way->Record2Way_Start(sample_rate);
}

int AudioYusuHardware::xWayRec_Stop()
{
    LOGD("+xWayRec_Stop");
    return pRecord2Way->Record2Way_Stop();
}

int AudioYusuHardware::xWayRec_Read(void *buffer, int size_bytes)
{
    //LOGD("+xWayRec_Read");
    return pRecord2Way->Record2Way_Read(buffer, size_bytes);
}

// ----------------------------------------------------------------------------

// for HDMI
int AudioYusuHardware::MuteHDMI( int timeMs)
{
	int fd = open("/dev/hdmitx", O_RDWR, 0);
	int ret;
    if (fd >= 0) {
        ret = ioctl(fd, MTK_HDMI_AUDIO_ENABLE, 0); //mute hdmi
        if (ret == -1) {
            LOGE("Fail to mute HDMI");
            return 0;
        }
		LOGD("sleep %d ms for HDMI",timeMs);
		usleep(timeMs *1000);
	    ret = ioctl(fd, MTK_HDMI_AUDIO_ENABLE, 1); //unmute hdmi
        if (ret == -1) {
			LOGE("Fail to unmute HDMI");
            return 0;
        }

        close(fd);
    } else {
        LOGE("Fail to Open HDMI");
        return 0;
    }
	return 0;
}

int AudioYusuHardware::SetHDMIAudioStatus(int mEnable)
{
    LOGD("+SetHDMIAudioStatus(%d) \n",mEnable);

    if(m_HDMIAudioStatus == mEnable)
    {
        LOGD(" SetHDMIAudioStatus, HDMI status no change %d \n",mEnable);
    }
    if(mEnable == true)
    {
        if(mOuputForceuse[mMode] !=0 )
        {
            LOGD("SetHDMIAudioStatus, add: mOuputForceuse(0x%x),mMode(%d) \n",mOuputForceuse[mMode],mMode);
            AddOutputDevices(android_audio_legacy::AudioSystem::DEVICE_OUT_HDMI_AUD);
        }
        else
        {
            mPredevice = mOutputAvailable[mMode] ;
            AddOutputDevices(android_audio_legacy::AudioSystem::DEVICE_OUT_HDMI_AUD);
            mDevice = mOutputAvailable[mMode] ;
            LOGD("SetHDMIAudioStatus, add: mPredevice(0x%x),mDevice(0x%x),mMode(%d) \n",mPredevice,mDevice,mMode);
            doOutputDeviceRouting();
        }
    }
    else
    {
        if(mOuputForceuse[mMode] !=0)
        {
            LOGD("SetHDMIAudioStatus, remove: mOuputForceuse(0x%x),mMode(%d) \n",mMode,mOuputForceuse[mMode]);
            RemoveOutputDevices(android_audio_legacy::AudioSystem::DEVICE_OUT_HDMI_AUD);
        }
        else
        {
            mPredevice = mOutputAvailable[mMode];
            RemoveOutputDevices(android_audio_legacy::AudioSystem::DEVICE_OUT_HDMI_AUD);
            mDevice = mOutputAvailable[mMode];
            LOGD("SetHDMIAudioStatus, remove: mPredevice(0x%x),mDevice(0x%x),mMode(%d) \n",mPredevice,mDevice,mMode);
            doOutputDeviceRouting();
        }
    }
    m_HDMIAudioStatus = mEnable;
    LOGD("-SetHDMIAudioStatus(%d) \n",m_HDMIAudioStatus);
    return true;
}

int AudioYusuHardware::GetHDMIAudioStatus()
{
    //LOGD("GetHDMIAudioStatus(%d) \n",m_HDMIAudioStatus);
    return m_HDMIAudioStatus;
}

int AudioYusuHardware::SetHDMIAudioConnect(bool connect)
{
    LOGD("+SetHDMIAudioConnect(%d) \n",connect);

    // HDMI Audio Connection
    if(connect == true)
    {
        // delay to open the audio hw
        if(mStreamHandler->GetSrcBlockRunning())
        {
            // turn on I2S clock
            // clock on/off should base on stream open/close.
            ::ioctl(mFd, AUD_SET_HDMI_CLOCK, 1);

            uint32 mSamplingRate = mStreamHandler->mOutput[0]->sampleRate();
            LOGD("SetHDMIAudioConnect, mSamplingRate(%d) \n",mSamplingRate);
            if(mSamplingRate==44100 || mSamplingRate==48000)
             {
                uint32 i2sMode = (mSamplingRate==44100) ? (0x9<< 8) : (0xa<< 8);
                mAfe_handle->SetAfeReg(AFE_I2S_OUT_CON,i2sMode,0x0F00);  // bit8~11: I2S1 output mode
               ::ioctl(mFd, AUD_SET_HDMI_SR, mSamplingRate);
             }
             // mute HDMI device some time if the audio is playing
             MuteHDMI(80);
        }
#ifdef MTK_TABLET_PLATFORM
        else
        {
           //alwasy turn on HDMI  clock
            ::ioctl(mFd, AUD_SET_HDMI_CLOCK, 1);
        }
#endif

        //set i2s 0 master out
        // Set connection
        mAfe_handle->SetAfeReg(AFE_CONN0,1<<22, 1<<22);  // Connect -- DL1_I_L to I2S_R,  bit5: I05-O01
        mAfe_handle->SetAfeReg(AFE_CONN0,1<<6, 1<<6);    // Connect -- DL1_I_R to I2S_L,  bit23: I06-O00
        mAfe_handle->SetAfeReg(AFE_I2S_OUT_CON, 0x0009, 0x000F);        // bit0: enable I2S1
    }
    else
    {
        // turn off I2S1 output GPIO
//        ::ioctl(mFd, AUD_SET_HDMI_GPIO, 0);

        // clear i2s0 connection
        mAfe_handle->SetAfeReg(AFE_CONN0, 0<<22, 1<<22);  // Connect -- DL1_I_L to I2S_R,  bit5: I05-O01
        mAfe_handle->SetAfeReg(AFE_CONN0, 0<<6, 1<<6);    // Connect -- DL1_I_R to I2S_L,  bit23: I06-O00

        // Reset I2S register
        mAfe_handle->SetAfeReg(AFE_I2S_OUT_CON, 0x0000, 0x0001);   // bit0: disable I2S1

        // turn off I2S clock
        // clock on/off should base on stream open/close.
        if(mStreamHandler->GetSrcBlockRunning())
        {
            mAfe_handle->SetAfeReg(AFE_DL_SRC2_1,0x1,0x1);  // bit0: DL_2_SRC_ON
            ::ioctl(mFd, AUD_SET_HDMI_CLOCK, 0);
        }
#ifdef MTK_TABLET_PLATFORM
        else
        {
            //alwasy turn off HDMI  clock
            ::ioctl(mFd, AUD_SET_HDMI_CLOCK, 0);
        }
#endif
    }

    LOGD("-SetHDMIAudioConnect \n");
    return true;
}


}; // namespace android
