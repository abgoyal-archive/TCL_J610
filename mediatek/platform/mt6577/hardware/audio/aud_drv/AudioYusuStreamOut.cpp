

//#define LOG_NDEBUG 0

#define LOG_TAG "AudioYusuStreamOut"

#include <stdint.h>
#include <sys/types.h>

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sched.h>
#include <fcntl.h>
#include <sys/ioctl.h>


#include <utils/Log.h>
#include <utils/String8.h>

#include "AudioYusuHardware.h"
#include "AudioAfe.h"
#include "AudioAnalogAfe.h"
#include <AudioYusuStreamHandler.h>
#include "AudioYusuVolumeController.h"
#include "AudioYusuDef.h"
#include "AudioYusuLad.h"
#include "AudioYusuLadPlayer.h"
#include <cutils/properties.h>

#include "audio_custom_exp.h"
//include to see if we need to ENABLE_AUDIO_COMPENSATION_FILTER, ENABLE_AUDIO_DRC_SPEAKER, ENABLE_HEADPHONE_COMPENSATION_FILTER

#include <AudioCompensationFilter.h>
#include <HeadphoneCompensationFilter.h>

#include <sys/time.h>
#include <stdio.h>


// AUDIO_HQA_SUPPORT: Audio Effect disable ACF, HCF, Audio DRC when HQA
#if defined(AUDIO_HQA_SUPPORT)
#ifdef ENABLE_AUDIO_COMPENSATION_FILTER
#undef ENABLE_AUDIO_COMPENSATION_FILTER
#endif

#ifdef ENABLE_AUDIO_DRC_SPEAKER
#undef ENABLE_AUDIO_DRC_SPEAKER
#endif

#ifdef ENABLE_HEADPHONE_COMPENSATION_FILTER
#undef ENABLE_HEADPHONE_COMPENSATION_FILTER
#endif

#endif

//Configure ACF Work Mode
#if defined(ENABLE_AUDIO_COMPENSATION_FILTER) && defined(ENABLE_AUDIO_DRC_SPEAKER)
#define AUDIO_COMPENSATION_FLT_MODE 5

#elif defined(ENABLE_AUDIO_COMPENSATION_FILTER)
#define AUDIO_COMPENSATION_FLT_MODE 4

#elif defined(ENABLE_AUDIO_DRC_SPEAKER)
#define AUDIO_COMPENSATION_FLT_MODE 3

#endif

namespace{

#define LITERAL_TO_STRING_INTERNAL(x)    #x
#define LITERAL_TO_STRING(x) LITERAL_TO_STRING_INTERNAL(x)

#define CHECK(condition)                                \
    LOG_ALWAYS_FATAL_IF(                                \
            !(condition),                               \
            "%s",                                       \
            __FILE__ ":" LITERAL_TO_STRING(__LINE__)    \
            " CHECK(" #condition ") failed.")



#define AUDIO_COMPENSATION_FLT_SAMPLE_COUNT (2048)
#define HDMI_ADD_SILENCE_DELAY 120

// ----------------------------------------------------------------------------
//Method to enable PCM Dump
//streamoutori_dump.pcm:  adb shell setprop streamout_ori.pcm.dump 1
//streamoutdcr_dump.pcm:  adb shell setprop streamout_dcr.pcm.dump 1
//streamouts2m_dump.pcm:  adb shell setprop streamout_s2m.pcm.dump 1
//streamoutacf_dump.pcm:  adb shell setprop streamout_acf.pcm.dump 1
//streamouthcf_dump.pcm:  adb shell setprop streamout_hcf.pcm.dump 1
//streamout.pcm:  adb shell setprop streamout.pcm.dump 1

const char * streamout_ori = "/sdcard/streamoutori_dump.pcm";
const char * streamout_ori_propty = "streamout_ori.pcm.dump";

const char * streamout_dcr = "/sdcard/streamoutdcr_dump.pcm";
const char * streamout_dcr_propty = "streamout_dcr.pcm.dump";

const char * streamout_s2m = "/sdcard/streamouts2m_dump.pcm";
const char * streamout_s2m_propty = "streamout_s2m.pcm.dump";
const char * streamout_acf = "/sdcard/streamoutacf_dump.pcm";
const char * streamout_acf_propty = "streamout_acf.pcm.dump";
const char * streamout_hcf = "/sdcard/streamouthcf_dump.pcm";
const char * streamout_hcf_propty = "streamout_hcf.pcm.dump";

const char * streamout = "/sdcard/streamout.pcm";
const char * streamout_propty = "streamout.pcm.dump";

void dumpPcmData(const char * filepath, void * buffer, int count,const char * propty)
{
    char value[PROPERTY_VALUE_MAX];
    property_get(propty, value, "0");
    int bflag=atoi(value);
    if(bflag)
    {
       FILE * fp= fopen(filepath, "ab+");
       if(fp!=NULL)
       {
            fwrite(buffer,1,count,fp);
            fclose(fp);
       }
    }
}

 bool modeApplyCompensation(int mode)
{
    return (mode == android_audio_legacy::AudioSystem::MODE_NORMAL || mode == android_audio_legacy::AudioSystem::MODE_RINGTONE);
}

 bool isBtScoDevice(uint32 device)
{
    if(device == android_audio_legacy::AudioSystem::DEVICE_OUT_BLUETOOTH_SCO || device == android_audio_legacy::AudioSystem::DEVICE_OUT_BLUETOOTH_SCO_HEADSET ||
            device == android_audio_legacy::AudioSystem::DEVICE_OUT_BLUETOOTH_SCO_CARKIT)
    {
        return true;
    }
    return false;
}

#ifdef GEN1KTONE
#define SOUND_FAKE_READ_SIZE (128)
short Tone_1k[] =
{
    0x0   ,     0x0,          0xa02     ,0xa02,
    0x13a4   ,0x13a3,    0x1c83   ,0x1c83,
    0x2449   ,0x2449,    0x2aac   ,0x2aac,
    0x2f6a   ,0x2f69,    0x3254   ,0x3255,
    0x3352   ,0x3351,    0x3255   ,0x3255,
    0x2f6a   ,0x2f69,    0x2aac   ,0x2aac,
    0x2449   ,0x244a,    0x1c83   ,0x1c83,
    0x13a4   ,0x13a3,    0xa03     ,0xa02,
    0x0         ,0x0     ,    0xf5fd   ,0xf5fd,
    0xec5d   ,0xec5c,    0xe37e   ,0xe37e,
    0xdbb7   ,0xdbb7,    0xd555   ,0xd555,
    0xd097   ,0xd096,    0xcdab   ,0xcdac,
    0xccaf   ,0xccae,    0xcdab   ,0xcdab,
    0xd096   ,0xd097,    0xd554   ,0xd554,
    0xdbb7   ,0xdbb7,    0xe37e   ,0xe37d,
    0xec5d   ,0xec5d,    0xf5fd   ,0xf5fe
};

void ReplaceBufferWithTone(const void* buffer,size_t bytes)
{
    int copysize;
    char *copybuffer = NULL;
    copybuffer = (char*)buffer;
    while(bytes)
    {
        if(bytes >=SOUND_FAKE_READ_SIZE)
        {
            copysize = SOUND_FAKE_READ_SIZE;
        }
        else
        {
            copysize = bytes;
        }
        memcpy((void*)copybuffer ,(void*)Tone_1k ,copysize);
        bytes -= copysize;
        copybuffer += copysize;
    }
}
#endif


}

// ----------------------------------------------------------------------------
namespace android
{

static bool sipcall_to_bt()
{
#ifdef ENABLE_VOIP_BTSCO
    return true;
#endif
    return false;
}

// ----------------------------------------------------------------------------
AudioYusuStreamOut::AudioYusuStreamOut()
    :mAudioHardware(NULL),
     mStandby(true),
     mFd(-1),
     mAttribute(NULL),
     mBTscoAttribute(NULL),
     mLatency(0),
     mInterruptCounter(0),
     mHwBufferSize(0),
     mGain(0x1000),
     mSetGainFlag(false),
     mOutStreamIndex(0),
     mLADPlayBuffer(NULL),
     mFillinZero(false),
     mAudioCompFlt(NULL),
     mHeadphoneCompFlt(NULL),
     mDcRemove(NULL),
     mBliSrc(NULL),
     mSwapBufferOne(NULL),
     mSwapBufferTwo(NULL),
     mMuteHwMs(0),
     mStreamMode(0),
     mDevice(0)
{

}

uint32_t  AudioYusuStreamOut::sampleRate() const
{
    return mAttribute->mSampleRate;
}

status_t AudioYusuStreamOut::getRenderPosition(uint32_t *dspFrames)
{
    return INVALID_OPERATION;
}

void AudioYusuStreamOut::ResetGainFlag(void)
{
    mSetGainFlag = false;
}

bool AudioYusuStreamOut::InitOutStream(int StreamIndex,int fd)
{
    int32 ret;
    LOGD("InitOutStream, fd=%d",fd);
    CHECK(fd>0);
    mFd = fd;
    mHwBufferSize = ::ioctl(mFd,GET_AFE_BUFFER_SIZE,0);  //ioctl to stanby mode
    ret = ::ioctl(mFd,OPEN_DL1_STREAM,mOutStreamIndex);
    if(ret < 0 || ret >= MAX_OUTPUT_STREAM_NUM)
    {
        LOGD("InitOutStream fail");
    }
    mAttribute = new AudioAttribute();
    mBTscoAttribute = new AudioAttribute();
    CHECK(mAttribute!=NULL);
    CHECK(mBTscoAttribute!=NULL);
    return true;
}

status_t AudioYusuStreamOut::set(
    AudioYusuHardware *hw,
    int mFd,
    uint32_t devices,
    int *pFormat,
    uint32_t *pChannels,
    uint32_t *pRate)
{
    LOGD("format = %d, channels = %d, rate = %d", *pFormat,*pChannels,*pRate);

    if(*pFormat == android_audio_legacy::AudioSystem::PCM_16_BIT)
        mAttribute->mFormat = android_audio_legacy::AudioSystem::PCM_16_BIT;
    else if(*pFormat == android_audio_legacy::AudioSystem::PCM_8_BIT)
        mAttribute->mFormat =android_audio_legacy::AudioSystem::PCM_8_BIT;
    else
    {
        ASSERT(1);
        LOGE("Format is not a valid number");
        mAttribute->mFormat = android_audio_legacy::AudioSystem::PCM_16_BIT;
    }

    if(*pChannels == android_audio_legacy::AudioSystem::CHANNEL_OUT_MONO)
    {
        mAttribute->mChannelCount = 1;
    }
    else if (*pChannels == android_audio_legacy::AudioSystem::CHANNEL_OUT_STEREO)
    {
        mAttribute->mChannelCount = 2;
    }
    else
    {
        ASSERT(1);
        LOGE("Channelsis not a valid number");
        mAttribute->mChannelCount = 2;
    }

    mAttribute->mSampleRate = *pRate;
    // set default BT_SCO attribute
    mBTscoAttribute->mChannelCount = 2;
    mBTscoAttribute->mFormat = android_audio_legacy::AudioSystem::PCM_16_BIT;
    mBTscoAttribute->mSampleRate = 16000;


    CHECK(hw != NULL);
    mAudioHardware = hw;
    mInterruptCounter = calInterrupttime();
    CHECK(mInterruptCounter != 0);
    //set Hardware setting
    mAudioHardware->mAfe_handle->Afe_Set_Stream_Attribute(*pFormat,*pChannels,*pRate,AFE_MODE_DAC);

    // for I2S1 user
    mAudioHardware->mAfe_handle->Afe_Set_Stream_Attribute(*pFormat,*pChannels,*pRate,AFE_MODE_I2S1_OUT);

    mAudioHardware->mAfe_handle->Afe_Set_Stream_Gain(0xffff); // hardcore AFE gain

    LOGD("BufferSize = %u, Format = %d, ChannelCount=%u, SampleRate=%u"
                  ,mHwBufferSize,mAttribute->mFormat,mAttribute->mChannelCount ,mAttribute->mSampleRate);

    mLatency = (mHwBufferSize * 1000) / (mAttribute->mSampleRate * mAttribute->mChannelCount * (mAttribute->mFormat == android_audio_legacy::AudioSystem::PCM_8_BIT?1:2));
    LOGD("mLatency = %u",mLatency);

    mSwapBufferOne = new uint8[bufferSize()];
	mSwapBufferTwo = new uint8[bufferSize()];
    CHECK(mSwapBufferOne!=NULL); // should not be empty
    CHECK(mSwapBufferTwo!=NULL); // should not be empty

#if defined(ENABLE_AUDIO_COMPENSATION_FILTER) || defined(ENABLE_AUDIO_DRC_SPEAKER)
    mAudioCompFlt = new ACFCompensationFilter();
    if(mAudioCompFlt)
    {
        mAudioCompFlt->init(AUDIO_COMPENSATION_FLT_MODE);
    }
#endif

#ifdef ENABLE_HEADPHONE_COMPENSATION_FILTER
    mHeadphoneCompFlt = new HCFCompensationFilter();
    if(mHeadphoneCompFlt)
    {
        mHeadphoneCompFlt->init(HEADPHONE_COMPENSATION_FLT_MODE);
    }
#endif

#ifdef ENABLE_DC_REMOVE
    mDcRemove = new DcRemove();
    if(mDcRemove)
    {
        mDcRemove->init(channels_t(),sampleRate(),DcRemove::DCR_MODE_3);
    }
#endif

#ifdef ENABLE_VOIP_BTSCO
    mBliSrc = new BliSrc();
#endif
    return NO_ERROR;
}


AudioYusuStreamOut::~AudioYusuStreamOut()
{
    if(mAudioHardware)
    {  
        if(mAudioCompFlt)
        {
            mAudioCompFlt->deInit();
            delete mAudioCompFlt;
            mAudioCompFlt=NULL;
        }      
        if(mHeadphoneCompFlt)
        {
            mHeadphoneCompFlt->deInit();
            delete mHeadphoneCompFlt;
            mHeadphoneCompFlt=NULL;
        }
        if(mDcRemove)
        {
            mDcRemove->close();
            delete mDcRemove;
            mDcRemove=NULL;
        }
        if(mBliSrc)
        {
            mBliSrc->close();
            delete mBliSrc;
            mBliSrc=NULL;
        }
        if(mSwapBufferOne)
        {
            delete []mSwapBufferOne;
            mSwapBufferOne = NULL;
        }
        if(mSwapBufferTwo)
        {
            delete []mSwapBufferTwo;
            mSwapBufferTwo=NULL;
        }
        mAudioHardware->closeOutputStream(this);
    }
}

int  AudioYusuStreamOut::padMuteDataToHw(void *buffer,uint32 timeMs)
{
    size_t muteSize = (mAttribute->mSampleRate * timeMs)/1000 * mAttribute->mChannelCount * 2;
    const size_t  bufSize = bufferSize();
    memset(buffer,0,bufSize);
    usleep(5 * 1000);
    size_t writeSize = bufSize;
    if(muteSize < bufSize)
    {
        writeSize = muteSize;
    }
    ::write(mFd, buffer, writeSize);
    muteSize -= writeSize;
    if(mAudioHardware->mStreamHandler->GetSrcBlockRunning() == false ) // src is not running , open afe and analog block
    {
        startAudioHw();
    }
    while(muteSize>0)
    {
        writeSize = bufSize;
        if(muteSize < bufSize)
        {
            writeSize = muteSize;
        }
        ::write(mFd, buffer, writeSize);
        muteSize -= writeSize;
    }
    return muteSize;
}


size_t AudioYusuStreamOut::stereoToMono(const void* buffer, size_t bytes, void *outbuffer)
{
    uint32 Idx_i;
    int16 PCM_M;
#ifdef ENABLE_AUDIO_SW_STEREO_TO_MONO
    if((mAudioHardware->mAfe_handle->mAudioStereoToMono)&&(mAudioHardware->mI2STxStatus<=0))
    {
        //enable software stereo to mono
        for(Idx_i = 0; Idx_i<bytes>>1; Idx_i+=2)
        {
            PCM_M = (* ((const short *)buffer+Idx_i)>>1)+(*((const short *) buffer+Idx_i+1)>>1);
            *((short *)outbuffer+Idx_i) = PCM_M;
            *((short *)outbuffer+Idx_i+1) = PCM_M;
        }
        return bytes;
    }
    else
    {
        LOGV("SW_S2M  NO S2M");
       return 0;
    }
#else
    LOGV("HW_S2M  NO S2M");
    return 0;
#endif

}

void AudioYusuStreamOut::prepareAudioHw(void)
{
#ifdef  ENABLE_EXT_DAC
    mAudioHardware->mAudioCustomFunction->EXT_DAC_SetPlaybackFreq (sampleRate());
#endif
    pthread_mutex_lock(&mAudioHardware->ModeMutex);

    ioctl(mFd,START_DL1_STREAM,mOutStreamIndex);
    if(mAudioHardware->GetVoiceStatus() == true)
    {
        mAudioHardware->EnableSpeaker();
        mAudioHardware->mAnaReg->AnalogAFE_Close(AUDIO_PATH);
    }
    else if(mAudioHardware->GetFmAnalogInEnable () &&
        (mAudioHardware->GetFmSpeakerStatus() || (mAudioHardware->GetCurrentDevice() == android_audio_legacy::AudioSystem::DEVICE_OUT_SPEAKER)))
    {
        mAudioHardware->ForceEnableSpeaker();
    }
    else if(mAudioHardware->GetMatvAnalogEnable())
    {
        mAudioHardware->EnableSpeaker();
    }
    mAudioHardware->mStreamHandler->AddSrcRunningNumber();
    pthread_mutex_unlock(&mAudioHardware->ModeMutex);

}

size_t AudioYusuStreamOut::writePcmDataToModem(const void* buffer, size_t bytes)
{
    pthread_mutex_lock(&mAudioHardware->ModeMutex);
    mAudioHardware->mStreamHandler->SetLadPlayer();
    if(mAudioHardware->GetBgsStatus() == true && mLADPlayBuffer != NULL)
    {
        // mutex for LADPlaywrite must be return
        LOGV("MODE_IN_CALL write to LAD bytes:%d",bytes);
        pthread_mutex_lock(&mAudioHardware->LadMutex);
        bytes = mLADPlayBuffer->LADPlayWrite((int8 *) buffer,  bytes); // write to LAD
        pthread_mutex_unlock(&mAudioHardware->LadMutex);
        pthread_mutex_unlock(&mAudioHardware->ModeMutex);
        usleep(2*1000);
    }
    else
    {
        LOGD("MODE_IN_CALL Discard data");
        pthread_mutex_lock(&mAudioHardware->LadMutex);
        memset((void*)buffer,0,bytes);
        pthread_mutex_unlock(&mAudioHardware->LadMutex);
        pthread_mutex_unlock(&mAudioHardware->ModeMutex);
        usleep(1000*mLatency);
    }
    dumpPcmData(streamout,(void*)buffer,bytes,streamout_propty);
    return bytes;
}

size_t AudioYusuStreamOut::writePcmDataToBtSco(void* buffer, size_t bytes)
{
    void * outbuffer = buffer;
    size_t outputSize =bytes;
#ifdef  ENABLE_EXT_DAC
    mAudioHardware->mAudioCustomFunction->EXT_DAC_SetPlaybackFreq(sampleRate());
#endif
    if(mFillinZero ==true)
    {
        mFillinZero = false;
        //LOGD("mFillinZero write data to hardware");
        memset(mSwapBufferTwo,0,bufferSize());
        ::write(mFd, mSwapBufferTwo, bufferSize()); // write to afe hardware
    }
    // when in communication mode and BT mode should be 16K
    if(mBliSrc)
    {
        if(mBliSrc->initStatus()!=OK)
        {
            mBliSrc->init(mAttribute->mSampleRate, mAttribute->mChannelCount, mBTscoAttribute->mSampleRate, mBTscoAttribute->mChannelCount);
        }
        outbuffer = mSwapBufferTwo;
        outputSize = mBliSrc->process(buffer,bytes,outbuffer);
        if(outputSize <=0)
        {
            outputSize = bytes;
            outbuffer = buffer;
        }
    }
    dumpPcmData(streamout,outbuffer,outputSize,streamout_propty);
    ::write(mFd, outbuffer, outputSize); // write to afe hardware
    return bytes;

}

size_t AudioYusuStreamOut::writePcmDataToAudioHw(void* buffer, size_t bytes)
{
    // From the period "AP side set Speech on" to "Speech off ack message back from Modem side"
    // During this period, AP side doesn't control AFE hardware.
    if( (mAudioHardware->Get_Recovery_Speech()==true) || (mAudioHardware->Get_Recovery_VT()==true) )
    {
        LOGD("writePcmDataToAudioHw,AFE HW is controlled by Modem side...");
        memset(buffer,0,bytes);
        usleep(1000*mLatency);
        return bytes;
    }
    // for siwtch samlpeate rate , write zero buffer
    if(mFillinZero ==true)
    {
        mFillinZero = false;
        //LOGD("mFillinZero write data to hardware");
        memset(mSwapBufferTwo,0,bufferSize());
        ::write(mFd, mSwapBufferTwo, bufferSize()); // write to afe hardware
    }

    void * inbuffer = buffer; // mSwapBufferOne  or mixerbufer of audioflinger
    void * outbuffer =mSwapBufferTwo;
    if(stereoToMono(inbuffer,bytes,outbuffer) > 0)
    {
        void * temp = outbuffer;
        outbuffer = inbuffer; // mSwapBufferOne  or mixerbufer of audioflinger
        inbuffer =  temp; //mSwapBufferTwo
    }
    dumpPcmData(streamout_s2m, inbuffer, bytes, streamout_s2m_propty);
    if(mAudioHardware->GetSpeakerStatus()&& modeApplyCompensation(mStreamMode) && mAudioCompFlt)
    {   
        if(!mAudioCompFlt->isStarted()) mAudioCompFlt->start(mAttribute->mChannelCount, mAttribute->mSampleRate);
        size_t outputSize =mAudioCompFlt->process(inbuffer, bytes, outbuffer); //process acf 
        if(outputSize==0) outbuffer=inbuffer; //acf not handle inbuffer;
        
        if(mHeadphoneCompFlt->isStarted()) mHeadphoneCompFlt->stop();
        dumpPcmData(streamout_acf, outbuffer, bytes, streamout_acf_propty);
    }
    else if(!mAudioHardware->GetSpeakerStatus() && (mAudioHardware->GetHDMIAudioStatus()==false) && mHeadphoneCompFlt) 
    {
        if(!mHeadphoneCompFlt->isStarted()) mHeadphoneCompFlt->start(mAttribute->mChannelCount, mAttribute->mSampleRate);
        size_t outputSize =mHeadphoneCompFlt->process(inbuffer, bytes, outbuffer); //process hcf  
        if(outputSize == 0) outbuffer = inbuffer; //hcf not handle inbuffer;
        
        if(mAudioCompFlt->isStarted()) mAudioCompFlt->stop();
        dumpPcmData(streamout_hcf, outbuffer, bytes, streamout_hcf_propty);
    }
    else
    {
        if(mAudioHardware->GetHDMIAudioStatus() == true && mMuteHwMs > 0){
            padMuteDataToHw(outbuffer,mMuteHwMs); 
            mMuteHwMs = 0;
        }
        outbuffer = inbuffer;
    }
    dumpPcmData(streamout,outbuffer,bytes,streamout_propty);
    bytes = ::write(mFd, outbuffer, bytes);
    return bytes;
}



void  AudioYusuStreamOut::startAudioSco()
{
    LOGD("startAudioSco");
    int interval = (bufferSize()*1000) / (BTSCO_SAMPLERATE*2);
    mAudioHardware->mStreamHandler->OutputStreamLock();
//    ::ioctl(mFd,STANDBY_DL1_STREAM,mOutStreamIndex);
//    ioctl(mFd,START_DL1_STREAM,mOutStreamIndex);
    mFillinZero = true;
    SetBTscoSamplerate (BTSCO_SAMPLERATE);
    mAudioHardware->mStreamHandler->AudioStartRunning(mOutStreamIndex,interval);
    mAudioHardware->mStreamHandler->OutputStreamUnLock();
    usleep(50*1000);
}

void  AudioYusuStreamOut::startAudioHw()
{
    LOGD("startAudioHw");
    /* Get ModeMutex to ensure domoderouting() will not interrupt audiostartrunning() */
    pthread_mutex_lock(&mAudioHardware->ModeMutex);
    mAudioHardware->mStreamHandler->OutputStreamLock();
    mAudioHardware->mAnaReg->AnalogAFE_Request_ANA_CLK();
#ifdef  ENABLE_EXT_DAC
    mAudioHardware->mAudioCustomFunction->EXT_DAC_SetPlaybackFreq (sampleRate());
#endif
    mAudioHardware->mStreamHandler->mAfe_handle->Afe_Set_Timer(IRQ1_MCU,mInterruptCounter);
    mAudioHardware->mStreamHandler->AudioStartRunning(mOutStreamIndex,mInterruptCounter);
    mAudioHardware->mAnaReg->AnalogAFE_Release_ANA_CLK();
    mAudioHardware->mStreamHandler->OutputStreamUnLock();
    pthread_mutex_unlock(&mAudioHardware->ModeMutex);
}

bool AudioYusuStreamOut::isAudioHwTurnOff()
{
    uint32 dac=0;
    uint32 src=0;
    uint32 i2s=0;
    mAudioHardware->mAfe_handle->GetAfeReg(AFE_DAC_CON0,&dac);
    uint32 afeOn = dac & 0x01;
    uint32 dl1On = dac & 0x02;
    mAudioHardware->mAfe_handle->GetAfeReg(AFE_DL_SRC2_1,&src);
    uint32 dlSrcOn = src & 0x01;
    mAudioHardware->mAfe_handle->GetAfeReg(AFE_I2S_OUT_CON,&i2s);
    uint32 i2sOutOn = i2s & 0x01;
    // Check that afe/dl1/dl_src are turn offset
    if(!afeOn  || !dl1On || !(dlSrcOn||i2sOutOn)) // hw is turned off2
    {
        LOGD("isAudioHwTurnOff, AFE_DAC_CON0 (0x%x) \n",dac);
        LOGD("isAudioHwTurnOff, AFE_DL_SRC2_1 (0x%x) \n",src);
        return true;
    }
    return false;
}
void AudioYusuStreamOut::turnOnAudioHw()
{
    LOGV("turnOnAudioHw");
    mAudioHardware->mStreamHandler->OutputStreamLock();
    mAudioHardware->mAnaReg->AnalogAFE_Request_ANA_CLK();
    mAudioHardware->mStreamHandler->AudioStopRunning();
    ::ioctl(mFd,STANDBY_DL1_STREAM,mOutStreamIndex);  //ioctl to stanby mode, clean afe read and write index
    usleep((mLatency>>1)*1000);
    ioctl(mFd,START_DL1_STREAM,mOutStreamIndex);
#ifdef  ENABLE_EXT_DAC
    mAudioHardware->mAudioCustomFunction->EXT_DAC_SetPlaybackFreq (sampleRate ());
#endif
    mAudioHardware->mStreamHandler->mAfe_handle->Afe_Set_Timer(IRQ1_MCU,mInterruptCounter);
    mAudioHardware->mStreamHandler->AudioStartRunning(mOutStreamIndex,mInterruptCounter);
    mAudioHardware->mAnaReg->AnalogAFE_Release_ANA_CLK();
    mAudioHardware->mStreamHandler->OutputStreamUnLock();
}

bool AudioYusuStreamOut::isAudioScoTurnoff()
{
    uint32 dac=0;
    mAudioHardware->mAfe_handle->GetAfeReg(AFE_DAC_CON0,&dac);
    uint32 afeOn = dac & 0x01;
    uint32 dl1On = dac & 0x02;
    // Check that afe/dl1/dl_src are turn offset
    if(!afeOn  || !dl1On ) // sco is turned off
    {
        LOGD("isAudioScoTurnoff, AFE_DAC_CON0 (0x%x) \n",dac);
        return true;
    }
    return false;
}
void AudioYusuStreamOut::turnOnAudioSco()
{
    LOGD("turnOnAudioSco");
    int interval = (bufferSize() *1000) / (BTSCO_SAMPLERATE*2);
    mAudioHardware->mStreamHandler->OutputStreamLock();
    mAudioHardware->mStreamHandler->AudioStopRunning();
    ::ioctl(mFd,STANDBY_DL1_STREAM,mOutStreamIndex);
    ioctl(mFd,START_DL1_STREAM,mOutStreamIndex);
    SetBTscoSamplerate (BTSCO_SAMPLERATE);
    mAudioHardware->mStreamHandler->AudioStartRunning(mOutStreamIndex,interval);
    mAudioHardware->mStreamHandler->OutputStreamUnLock();
}

ssize_t AudioYusuStreamOut::write(const void* buffer, size_t bytes)
{
    Mutex::Autolock _l(mLock);
    void * outbuffer = const_cast<void *>(buffer);
    dumpPcmData(streamout_ori,outbuffer,bytes,streamout_ori_propty);
    //DC Remove Filter
    if(mDcRemove)
    {
        outbuffer = mSwapBufferOne;
        size_t outputSize= mDcRemove->process(buffer,bytes,outbuffer);
    }
    dumpPcmData(streamout_dcr, outbuffer, bytes, streamout_dcr_propty);
    
#ifdef GEN1KTONE
    //replace buffer with 1ktone sample
#endif
    //prepare audio HW
    if(mStandby)
    {
        LOGD("AudioHw is standby");
        //prepare audio HW
        prepareAudioHw();
        // restart compensation filter
        if(mAudioCompFlt)
        {
            mAudioCompFlt->start(mAttribute->mChannelCount, mAttribute->mSampleRate);
        }
        if(mHeadphoneCompFlt)
        {
            mHeadphoneCompFlt->start(mAttribute->mChannelCount, mAttribute->mSampleRate);
        }
        if(mAudioHardware->GetHDMIAudioStatus() == true)
        {
            mMuteHwMs = HDMI_ADD_SILENCE_DELAY;
        }
        mStandby = false;
    }
    mAudioHardware->getMode(&mStreamMode);
    if(mStreamMode == android_audio_legacy::AudioSystem::MODE_IN_CALL)
    {
        // background sound in in-call mode
        writePcmDataToModem(outbuffer,bytes);
    }
    else if(sipcall_to_bt() && mStreamMode == android_audio_legacy::AudioSystem::MODE_IN_COMMUNICATION &&
            mAudioHardware->GetCurrentDevice () == android_audio_legacy::AudioSystem::DEVICE_OUT_BLUETOOTH_SCO)
    {
        // BT-SCO in in-communication mode
        writePcmDataToBtSco(outbuffer,bytes); //outbuffer is buffer  or mSwapBufferOne
    }
    else // normal/Ringtone case
    {
        writePcmDataToAudioHw(outbuffer,bytes);//outbuffer is buffer  or mSwapBufferOne
    }
    // check hw / sco status
    if(mStreamMode != android_audio_legacy::AudioSystem::MODE_IN_CALL && !mAudioHardware->Get_Recovery_Speech() && !mAudioHardware->Get_Recovery_VT())
    {
        if(mAudioHardware->mStreamHandler->GetSrcBlockRunning() == false) // src is not running , open afe and analog block
        {
            if(sipcall_to_bt() && mAudioHardware->GetCommunicationStatus() && isBtScoDevice(mAudioHardware->GetCurrentDevice()))
            {
                startAudioSco();
            }
            else
            {
                startAudioHw();
            }
        }
        else
        {
            // Turn on AFE HW when phone call disconnect.
            // While phone call disconnect, the AFE HW would be turned off by modem side.
            // AP side need to turn on AFE HW again.
            if(sipcall_to_bt() && mAudioHardware->GetCommunicationStatus() && isBtScoDevice(mAudioHardware->GetCurrentDevice()))
            {
                if(isAudioScoTurnoff())
                    turnOnAudioSco();
            }
            else
            {
                if(isAudioHwTurnOff())
                     turnOnAudioHw();
            }
        }
    }
    else
    {
        LOGD_IF(mStreamMode != android_audio_legacy::AudioSystem::MODE_IN_CALL,"AFE HW is controlled by Modem side ...");
    }
    return bytes;
}

status_t AudioYusuStreamOut::standby()
{
    Mutex::Autolock _l(mLock);

    if(mStandby == false)  // stop this stream afe
    {
        pthread_mutex_lock(&mAudioHardware->ModeMutex);

        pthread_mutex_lock( &mAudioHardware->LadMutex);
        bool BgsStatus = mAudioHardware->GetBgsStatus();
        pthread_mutex_unlock( &mAudioHardware->LadMutex);

        mAudioHardware->mStreamHandler->OutputStreamLock();
        LOGV("!! standby, execute stop !!!");
        mStandby = true;
        mAudioHardware->mStreamHandler->MinusSrcRunningNumber();
        if(BgsStatus == true)
        {
            LOGV("!! standby, BgsStatus == true ");
            mAudioHardware->mStreamHandler->ResetLadPlayer();
        }

        if(mAudioHardware->mStreamHandler->GetSrcRunningNumber() <= 0) // no stream is output ,close analog
        {
            LOGD("!! standby, AudioStopRunning, STANDBY_DL1_STREAM");
            mAudioHardware->mAnaReg->AnalogAFE_Request_ANA_CLK();
            mAudioHardware->mStreamHandler->AudioStopRunning();
            ::ioctl(mFd,STANDBY_DL1_STREAM,mOutStreamIndex);  //ioctl to stanby mode, clean afe read and write index
            mAudioHardware->mAnaReg->AnalogAFE_Release_ANA_CLK();
        }
        pthread_mutex_unlock(&mAudioHardware->ModeMutex);

        if(mAudioCompFlt != NULL)
        {
            mAudioCompFlt->stop();
        }
        if(mHeadphoneCompFlt != NULL)
        {
            mHeadphoneCompFlt->stop();
        }
        if(mBliSrc)
        {
            mBliSrc->close();
        }
        mAudioHardware->mStreamHandler->OutputStreamUnLock();
    }
    return NO_ERROR;
}

// ToDo: SetVolume
status_t AudioYusuStreamOut::setVolume(float left, float right)
{
    float volume = (left+ right) /2;
    uint32 MapGain;
    if(mSetGainFlag == false)  // update gain
    {

        MapGain = volume * (float) mGain;
        if(MapGain > MAX_ASM_GAIN)
            MapGain = MAX_ASM_GAIN;

        mSetGainFlag == true;
    }
    else //use original gain
    {

        MapGain = volume *  (float) mGain;
        if(MapGain > MAX_ASM_GAIN)
            MapGain = MAX_ASM_GAIN;
    }
    return NO_ERROR;
}

status_t AudioYusuStreamOut::dump(int fd, const Vector<String16>& args)
{
    const size_t SIZE = 256;
    char buffer[SIZE];
    String8 result;
    snprintf(buffer, SIZE, "AudioYusuStreamOut::dump\n");
    result.append(buffer);
    snprintf(buffer, SIZE, "\tsample rate: %d\n", sampleRate());
    result.append(buffer);
    snprintf(buffer, SIZE, "\tbuffer size: %d\n", bufferSize());
    result.append(buffer);
    snprintf(buffer, SIZE, "\tchannel count: %d\n", channels());
    result.append(buffer);
    snprintf(buffer, SIZE, "\tformat: %d\n", format());
    result.append(buffer);
    snprintf(buffer, SIZE, "\tmAudioHardware: %p\n", mAudioHardware);
    result.append(buffer);
    snprintf(buffer, SIZE, "\tmFd: %d\n", mFd);
    result.append(buffer);
    ::write(fd, result.string(), result.size());
    return NO_ERROR;
}

size_t  AudioYusuStreamOut::bufferSize() const
{
    LOGV("bufferSize=%u",mHwBufferSize);
    return mHwBufferSize;
}
uint32_t  AudioYusuStreamOut::latency() const
{
    LOGV("latency=%u",mLatency);
    return mLatency;
}

uint32_t  AudioYusuStreamOut::channels() const
{
    //LOGV("channels=%d",mAttribute->mChannelCount);
    if(mAttribute->mChannelCount == 1)
        return  android_audio_legacy::AudioSystem::CHANNEL_OUT_MONO;
    else if(mAttribute->mChannelCount == 2)
        return  android_audio_legacy::AudioSystem::CHANNEL_OUT_STEREO;
    else
    {
        LOGE("mAttribute->mChannelCount error");
        return android_audio_legacy::AudioSystem::CHANNEL_OUT_STEREO;
    }
}

uint32_t  AudioYusuStreamOut::channels_t() const
{
    return mAttribute->mChannelCount;
}

int  AudioYusuStreamOut::format() const
{
    return  android_audio_legacy::AudioSystem::PCM_16_BIT;
}

void AudioYusuStreamOut::InitLadplayerBuffer(void)
{
    if(mLADPlayBuffer ==NULL)
    {
        // Set target sample rate = 16000 Hz
        mLADPlayBuffer = mAudioHardware->pLadPlayer->LADPlayer_CreateBuffer( sampleRate(),channels_t(),format(),16000);
        LOGV("InitLadplayerBuffer, SR=%u ,ch=%u, format=%d",sampleRate(),channels_t(),format());
    }
    else
    {
        LOGE("InitLadplayerBuffer, exist");
        return ;
    }
}

void AudioYusuStreamOut::DeinitLadplayerBuffer(void)
{
    if(mLADPlayBuffer != NULL)
    {
        delete mLADPlayBuffer;
        mLADPlayBuffer = NULL;
        LOGV("DeinitLadplayerBuffer, mLADPlayBuffer=NULL");
    }
    else
        LOGE("DeinitLadplayerBuffer, delete LADPlayBuffer");
}

int32 AudioYusuStreamOut::GetStreamMode(void)
{
    return mStreamMode;
}

status_t AudioYusuStreamOut::SetOutSamplerate(uint32 samplerate)
{
    //set Hardware setting
    LOGV("SetOutSamplerate SR=%d",samplerate);
    if(samplerate == 0)
        return BAD_VALUE;

    mAttribute->mSampleRate = samplerate;
    mAudioHardware->mAfe_handle->Afe_Set_Stream_Attribute(format(),channels(),samplerate,AFE_MODE_DAC);
#ifdef ENABLE_EXT_DAC
    mAudioHardware->mAfe_handle->Afe_Set_Stream_Attribute(format(),channels(),samplerate,AFE_MODE_I2S1_OUT);
#endif
    mLatency = (mHwBufferSize * 1000) / (mAttribute->mSampleRate * mAttribute->mChannelCount * (mAttribute->mFormat == android_audio_legacy::AudioSystem::PCM_8_BIT?1:2));
    return NO_ERROR;
}

status_t AudioYusuStreamOut::RestoreOutSamplerate(void)
{
    //set Hardware setting
    LOGV("RestoreOutSamplerate");
    mAudioHardware->mAfe_handle->Afe_Set_Stream_Attribute(format(),channels(),mAttribute->mSampleRate,AFE_MODE_DAC);

    // for I2S1 user
    mAudioHardware->mAfe_handle->Afe_Set_Stream_Attribute(format(),channels(),mAttribute->mSampleRate,AFE_MODE_I2S1_OUT);

    mLatency = (mHwBufferSize * 1000) / (mAttribute->mSampleRate * mAttribute->mChannelCount * (mAttribute->mFormat == android_audio_legacy::AudioSystem::PCM_8_BIT?1:2));
    return NO_ERROR;
}


status_t AudioYusuStreamOut::SetBTscoSamplerate(uint32 samplerate)
{
    //set to BT_SCO setting
    LOGV("::SetBTscoSamplerate samplerate = %d",samplerate);
    if(samplerate == 0)
    {
        return BAD_VALUE;
    }
    mBTscoAttribute->mSampleRate = samplerate;
    mAudioHardware->mAfe_handle->Afe_Set_Stream_Attribute(mBTscoAttribute->mFormat,mBTscoAttribute->mChannelCount,samplerate,AFE_MODE_DAC);
    mLatency = (mHwBufferSize * 1000) / (mBTscoAttribute->mSampleRate * mBTscoAttribute->mChannelCount * (mBTscoAttribute->mFormat == android_audio_legacy::AudioSystem::PCM_8_BIT?1:2));
    if(mBliSrc)
    {
        if(mBliSrc->initStatus()!=OK)
        {
            mBliSrc->init(mAttribute->mSampleRate,mAttribute->mChannelCount,mBTscoAttribute->mSampleRate,mBTscoAttribute->mChannelCount);
        }
    }
    return NO_ERROR;
}

status_t AudioYusuStreamOut::setParameters(const String8& keyValuePairs)
{
    AudioParameter param = AudioParameter(keyValuePairs);
    String8 key = String8(AudioParameter::keyRouting);
    status_t status = NO_ERROR;
    int device;
    LOGV("setParameters() %s", keyValuePairs.string());
    mAudioHardware->setParameters(keyValuePairs); //set to hardware
    if (param.getInt(key, device) == NO_ERROR)
    {
        //save previous device
        uint32 Predevice = 0;
        LOGD("Predevice = %d mDevice = %d", Predevice,mDevice);
        param.remove(key);
    }

    if (param.size())
    {
        status = BAD_VALUE;
    }
    return status;
}

String8 AudioYusuStreamOut::getParameters(const String8& keys)
{
    AudioParameter param = AudioParameter(keys);
    String8 value;
    String8 key = String8(AudioParameter::keyRouting);

    if (param.get(key, value) == NO_ERROR)
    {
        param.addInt(key, (int)mDevice);
    }

    LOGV("getParameters() %s", param.toString().string());
    return param.toString();
}

status_t AudioYusuStreamOut::SetACFPreviewParameter(void *ptr, int len)
{
    LOGD("::SetACFPreviewParameter!! ");
    if(mAudioCompFlt != NULL)
    {
        mAudioCompFlt->setParameter(ptr, len);
    }
    return NO_ERROR;
}

status_t AudioYusuStreamOut::SetHCFPreviewParameter(void *ptr, int len)
{
    LOGD("::SetHCFPreviewParameter!! ");
    if(mHeadphoneCompFlt != NULL)
    {
        mHeadphoneCompFlt->setParameter(ptr,len);
    }
    return NO_ERROR;
}

status_t AudioYusuStreamOut::startACFProcess(bool bStart)
{
    LOGD("::startACFProcess!! ");
    if((mAudioCompFlt != NULL) && (mStandby == false))
    {
        if (bStart) {
            mAudioCompFlt->start(mAttribute->mChannelCount, mAttribute->mSampleRate);
        }else {
            mAudioCompFlt->stop();
        }
    }
    return NO_ERROR;
}


status_t AudioYusuStreamOut::startHCFProcess(bool bStart)
{
    LOGD("::startHCFProcess!! ");
    if((mHeadphoneCompFlt != NULL) && (mStandby == false))
    {
        if (bStart) {
            mHeadphoneCompFlt->start(mAttribute->mChannelCount, mAttribute->mSampleRate);
        }else {
            mHeadphoneCompFlt->stop();
        }
    }
    return NO_ERROR;
}

bool AudioYusuStreamOut::SetFillinZero(bool bEnable)
{
    LOGD("SetFillinZero = %d",bEnable);
    mFillinZero = bEnable;
    return true;
}


uint32_t AudioYusuStreamOut::calInterrupttime()
{
    int SampleCount = bufferSize()/ mAttribute->mChannelCount;

    if(mAttribute->mFormat == android_audio_legacy::AudioSystem::PCM_16_BIT)
        SampleCount = SampleCount>>1;

    LOGV("calInterrupttime SampleCount = %d",(SampleCount>>1));
    mInterruptCounter =  SampleCount>>1;
    return mInterruptCounter ;//this is interrupt occur time
}

uint32_t  AudioYusuStreamOut::GetInterrupttime(void)
{
    return mInterruptCounter;
}


AudioYusuStreamOut::ACFCompensationFilter::ACFCompensationFilter()
    :mCompFlt(NULL),
     bStarted(false),
     mFilterMode(ACF_MODE_DEFAULT)
{
}

AudioYusuStreamOut::ACFCompensationFilter::~ACFCompensationFilter()
{
    deInit();
}
status_t AudioYusuStreamOut::ACFCompensationFilter::init(int filterMode)
{
    LOGV("ACFCompensationFilter::init");
    mCompFlt = new AudioCompensationFilter(); // create filter
    if(mCompFlt)
    {
        mCompFlt->Init();
        mCompFlt->LoadACFParameter();
        mFilterMode = filterMode;
        return NO_ERROR;
    }
    return NO_INIT;
}

status_t AudioYusuStreamOut::ACFCompensationFilter::setParameter(void * param, size_t len)
{
    if(mCompFlt)
    {
        mCompFlt->SetACFPreviewParameter((AUDIO_ACF_CUSTOM_PARAM_STRUCT *) param);
        return NO_ERROR;
    }
    return NO_INIT;
}

status_t AudioYusuStreamOut::ACFCompensationFilter::start(uint32 channel, uint32 samplerate)
{
    if(mCompFlt&&(bStarted==false))
    {
        LOGV("ACFCompensationFilter::start channel=%u, samplerate=%u, filtermode =%d", channel, samplerate,mFilterMode);
        Mutex::Autolock _l(mACFLock);
        mCompFlt->SetWorkMode(channel, samplerate, mFilterMode);
        mCompFlt->Start();
        bStarted = true;
        return NO_ERROR;
    }
    return NO_INIT;
}

size_t AudioYusuStreamOut::ACFCompensationFilter::process(const void * inbuffer, size_t bytes, void *outbuffer)
{
    if(mCompFlt&&bStarted)
    {
        // if write buffer is bigger than mAudioCompFltBufSize , allcoate more .
        int inputSampleCount = bytes >> 1;
        if(inputSampleCount >= 1024)
        {
            Mutex::Autolock _l(mACFLock);
            int consumedSampleCount  = inputSampleCount;
            int outputSampleCount =0;
            mCompFlt->Process((const short *)inbuffer, &consumedSampleCount, (short *)outbuffer, &outputSampleCount);
            size_t outputbytes = outputSampleCount<<1;
            LOGD_IF(outputbytes != bytes,"ACFCompensationFilter::process consumedSampleCount=%d, outputSampleCount=%d",
                consumedSampleCount,outputSampleCount);
            return outputbytes;
        }
        LOGW("ACFCompensationFilter::process ACF don't handle this buffer");
    }
    LOGW("ACFCompensationFilter::process ACF not initialized");
    return 0;
}

status_t AudioYusuStreamOut::ACFCompensationFilter::stop()
{
    LOGV("ACFCompensationFilter::stop");
    if(mCompFlt&&bStarted)
    {
        Mutex::Autolock _l(mACFLock);
        mCompFlt->Stop();
        mCompFlt->ResetBuffer();
        bStarted = false;
        return NO_ERROR;
    }
    return NO_INIT;
}
bool AudioYusuStreamOut::ACFCompensationFilter::isStarted()
{
    return bStarted;
}

void AudioYusuStreamOut::ACFCompensationFilter::deInit()
{
    LOGV("ACFCompensationFilter::deinit");
    if(mCompFlt != NULL)
    {
        mCompFlt->Deinit();
        delete mCompFlt;
        mCompFlt = NULL;
    }
    return;
}

AudioYusuStreamOut::HCFCompensationFilter::HCFCompensationFilter()
    :mCompFlt(NULL),
     bStarted(false),
     mFilterMode(HCF_MODE_DEFAULT)
{
}

AudioYusuStreamOut::HCFCompensationFilter::~HCFCompensationFilter()
{
    deInit();
}

status_t AudioYusuStreamOut::HCFCompensationFilter::init(int filterMode)
{
    LOGV("HCFCompensationFilter::init");
    mCompFlt = new HeadphoneCompensationFilter(); // create filter
    if(mCompFlt)
    {
        mCompFlt->Init();
        mCompFlt->LoadHCFParameter();
        mFilterMode = filterMode;
        return NO_ERROR;
    }
    return NO_INIT;
}

status_t AudioYusuStreamOut::HCFCompensationFilter::setParameter(void * param, size_t len)
{
    if(mCompFlt)
    {
        mCompFlt->SetHCFPreviewParameter((AUDIO_ACF_CUSTOM_PARAM_STRUCT *) param);
        return NO_ERROR;
    }
    return NO_INIT;
}

status_t AudioYusuStreamOut::HCFCompensationFilter::start(uint32 channel, uint32 samplerate)
{
    LOGV("HCFCompensationFilter::start channel=%u, samplerate=%u, filtermode =%d", channel, samplerate,mFilterMode);
    if(mCompFlt&&(bStarted==false))
    {
        Mutex::Autolock _l(mHCFLock);
        mCompFlt->SetWorkMode(channel, samplerate, mFilterMode);
        mCompFlt->Start();
        bStarted = true;
        return NO_ERROR;
    }
    return NO_INIT;
}

size_t AudioYusuStreamOut::HCFCompensationFilter::process(const void * inbuffer, size_t bytes, void *outbuffer)
{
    if(mCompFlt&&bStarted)
    {
        // if write buffer is bigger than mHeadphoneCompFltBufSize , allcoate more .
        int inputSampleCount = bytes >> 1;
        if(inputSampleCount >= 1024)
        {
            Mutex::Autolock _l(mHCFLock);
            int consumedSampleCount = inputSampleCount;
            int outputSampleCount =0;
            mCompFlt->Process((const short *)inbuffer, &consumedSampleCount, (short *)outbuffer, &outputSampleCount);
            size_t outputBytes = outputSampleCount<<1;
            LOGD_IF(outputBytes != bytes,"HCFCompensationFilter::::process ConsumedSampleCount=%d,OutputSampleCount=%d",
                consumedSampleCount,outputSampleCount);
            return outputBytes;
        }
        LOGW("HCFCompensationFilter::process HCF don't handle this buffer");
    }
    LOGW("HCFCompensationFilter::process HCF not initialized");
    return 0;

}

status_t AudioYusuStreamOut::HCFCompensationFilter::stop()
{
    LOGV("HCFCompensationFilter::stop");
    if(mCompFlt&&bStarted)
    {
        Mutex::Autolock _l(mHCFLock);
        mCompFlt->Stop();        
        mCompFlt->ResetBuffer();
        bStarted = false;
        return NO_ERROR;
    }
    return NO_INIT;
}
bool AudioYusuStreamOut::HCFCompensationFilter::isStarted()
{
    return bStarted;
}

void AudioYusuStreamOut::HCFCompensationFilter::deInit()
{
    LOGV("HCFCompensationFilter::deinit");
    if(mCompFlt != NULL)
    {
        mCompFlt->Deinit();
        delete mCompFlt;
        mCompFlt = NULL;
    }
    return ;
};

AudioYusuStreamOut::DcRemove::DcRemove()
    :mHandle(NULL)
{
}
AudioYusuStreamOut::DcRemove::~DcRemove()
{
    close();
}

status_t  AudioYusuStreamOut::DcRemove::init(uint32 channel, uint32 samplerate,uint32 dcrMode)
{
    LOGD("DcRemove::init channel=%u,samplerate=%u,dcrMode=%u",channel,samplerate,dcrMode);
    mHandle= DCR_Open(channel,samplerate,dcrMode);
    if(!mHandle)
    {
        LOGW("Fail to get DCR Handle");
        return NO_INIT;
    }
    return NO_ERROR;
}

status_t  AudioYusuStreamOut::DcRemove::close()
{
    LOGV("DcRemove::deinit");
    if(mHandle){
        DCR_Close(mHandle);
        mHandle = NULL;
    }
    return NO_ERROR;
}

size_t AudioYusuStreamOut::DcRemove::process(const void * inbuffer, size_t bytes, void *outbuffer)
{

    if(mHandle)
    {
        size_t outputBytes =0;
        uint32 inputBufSize  = bytes;
        uint32 outputBufSize = bytes;
        outputBytes = DCR_Process(mHandle,(short*)inbuffer,&inputBufSize,(short*)outbuffer,&outputBufSize);
        LOGD_IF(outputBytes!= bytes,"DcRemove::process inputBufSize = %d,outputBufSize=%d,outputBytes=%d ",
                inputBufSize,outputBufSize,outputBytes);
        return outputBytes;
    }
    LOGW("DcRemove::process Dcr not initialized");
    return 0;
}


AudioYusuStreamOut::BliSrc::BliSrc()
    :mHandle(NULL),mBuffer(NULL),mInitCheck(NO_INIT)
{
}

AudioYusuStreamOut::BliSrc::~BliSrc()
{
    close();
}

status_t AudioYusuStreamOut::BliSrc::initStatus()
{
    return mInitCheck;
}

status_t  AudioYusuStreamOut::BliSrc::init(uint32 inSamplerate,uint32 inChannel, uint32 OutSamplerate,uint32 OutChannel)
{
    if(mHandle==NULL)
    {
        uint32 workBufSize;
        BLI_GetMemSize(inSamplerate,inChannel, OutSamplerate,OutChannel,&workBufSize);
        LOGD("BliSrc::init InputSampleRate=%u, inChannel=%u, OutputSampleRate=%u, OutChannel=%u, mWorkBufSize = %u",
            inSamplerate,inChannel, OutSamplerate, OutChannel, workBufSize);
        mBuffer = new uint8[workBufSize];
        if(!mBuffer)
        {
            LOGE("BliSrc::init Fail to create work buffer");
            return NO_MEMORY;
        }
        memset((void*)mBuffer,0,workBufSize);
        mHandle = BLI_Open(inSamplerate, 2, OutSamplerate, 2, (char *)mBuffer);
        if(!mHandle)
        {
            LOGE("BliSrc::init Fail to get blisrc handle");
            if(mBuffer)
            {
                delete []mBuffer;
                mBuffer=NULL;
            }
            return NO_INIT;
        }
        mInitCheck =OK;
    }
    return NO_ERROR;

}

size_t  AudioYusuStreamOut::BliSrc::process(const void * inbuffer, size_t inBytes, void *outbuffer)
{
    if(mHandle)
    {
        size_t inputLength = inBytes;
        size_t outputLength = inBytes;
        size_t consume = BLI_Convert(mHandle,(short*)inbuffer, &inputLength, (short *)outbuffer, &outputLength);
        LOGD_IF(consume != inBytes, "inputLength=%d,consume=%d,outputLength=%d",inputLength,consume,outputLength);
        return outputLength;
    }
    LOGW("BliSrc::process src not initialized");
    return 0;
}

status_t  AudioYusuStreamOut::BliSrc::close(void)
{
    if(mHandle)
    {
        BLI_Close(mHandle);
        mHandle = NULL;
    }
    if(mBuffer)
    {
        delete []mBuffer;
        mBuffer=NULL;
    }
     mInitCheck =NO_INIT;
    return NO_ERROR;
}



// ---------------------------------------------------------------------------
}

