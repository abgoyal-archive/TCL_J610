

#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sched.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <time.h>
#include <sys/time.h>

#define LOG_TAG "AudioYusuApStreamIn"
#include <utils/Log.h>
#include <utils/String8.h>

#include "AudioYusuHardware.h"
#include "AudioAfe.h"
#include "AudioAnalogAfe.h"
#include "AudioYusuStreamHandler.h"
#include "AudioYusuVolumeController.h"
#include "AudioYusuApStreamIn.h"
#include "AudioYusuStreamInInterface.h"
#include "AudioAnalogAfe.h"
#include "audio_custom_exp.h"
#include <assert.h>
#include <cutils/properties.h>
#include <media/mediarecorder.h>


//#ifdef ENABLE_LOG_APSTREAMIN
#if 1
#undef LOGV
#define LOGV(...) LOGD(__VA_ARGS__)
#endif

namespace android {

static const unsigned short UNITY_GAIN = 0x0400; // 6.10 for 36dB";
static const int VolumeShiftBits = 10;
static const int UNITY_GAIN_Index = 35;

// step for 36dB , 1db per step
static const unsigned short FM_Map_gain[] =
{
    0xe0ef,
    0xc879,
    0xb2ac,
    0x9f3e,
    0x8dec,
    0x7e7d,
    0x70bc,
    0x6479,
    0x598c,
    0x4fcf,
    0x4721,
    0x3f65,
    0x3880,
    0x325b,
    0x2ce1,
    0x2800,
    0x23a6,
    0x1fc5,
    0x1c51,
    0x193d,
    0x167e,
    0x140c,
    0x11de,
    0xfec,
    0xe31,
    0xca6,
    0xb46,
    0xa0c,
    0x8f4,
    0x7fb,
    0x71c,
    0x656,
    0x5a6,
    0x509,
    0x47c,
    0x400,
};

static short clamp16(int sample)
{
    if ((sample>>15) ^ (sample>>31))
        sample = 0x7FFF ^ (sample>>31);
    return sample;
}

// apply volume with byes in count
int AudioYusuApStreamIn::StreamInApplyVolume(short *pcm , int count,unsigned short volume)
{
    //LOGD("StreamInApplyVolume count = %d volume = %x",count,volume);
    float Volume_inverse = 0.0;
    bool volume_change = false;
    int consumebytes = 0;
    count = count >> 1;
    if(mFm_Volume != volume){  // volume change
        Volume_inverse = (float)(volume-mFm_Volume)/count;
        volume_change = true;
    }
    short *pPcm = pcm;
    while(count){
        int value = 0;
        if(volume_change){
            value = *pPcm * (mFm_Volume+(Volume_inverse*consumebytes));
        }
        else{
            value = *pPcm * volume;
        }
        *pPcm = clamp16(value>>VolumeShiftBits);
        pPcm++;
        count--;
        consumebytes++;
    }
    mFm_Volume = volume;
    return consumebytes<<1;
}


status_t AudioYusuApStreamIn::set(
        AudioYusuHardware *hw,
	int fd,
	uint32_t devices,
	int *pFormat,
	uint32_t *pChannels,
	uint32_t *pRate,
	android_audio_legacy::AudioSystem::audio_in_acoustics acoustics)
{
    Mutex::Autolock lock(mLock);
    bool ret = true;
    LOGD("AudioYusuApStreamIn::set(%p, %d, %d, %d, %u)", hw, fd, *pFormat, *pChannels, *pRate);
    // check values


    LOGD("+set(%p, %d, %d, %d, %u)", hw, fd, *pFormat, *pChannels, *pRate);
    if (*pFormat != android_audio_legacy::AudioSystem::PCM_16_BIT|| *pChannels != channels() ||*pRate != 16000) {
        YAD_LOGE("Error opening input channel");
        goto SET_EXIT;
    }


    /* set parameters */
    mSampleRate = *pRate;
    if(!SupportedSampleRate(mSampleRate)){
        mApSampleRate = GetRecordSampleRate(*pRate);
    }
    else{
        mApSampleRate = mSampleRate;
    }
    mFormat = *pFormat;
    if(*pChannels == android_audio_legacy::AudioSystem::CHANNEL_IN_MONO){
        mChNum = 1;
    }
    else{
        mChNum = 2;
    }
    mFd = ::open(kAudioDeviceName, O_RDWR);
    mHw = hw;
    mReadBufferSize = (*pRate>>4)&0xfffffff0;
    mDevice =devices;
    if(mInputSource  == AUDIO_SOURCE_VOICE_RECOGNITION){
        mMutecount =(MUTE_APAUDIO_PCM_MS*mSampleRate*mChNum<<1)/1000;
    }
    else{
        mMutecount =(MUTE_APAUDIO_PCM_MS_NORMAL*mSampleRate*mChNum<<1)/1000;

}
    LOGD("mFd = %d mHw = %p mReadBufferSize = %d mMutecount = %d mChNum = %d mSampleRate = %d",
    mFd,mHw,mReadBufferSize,mMutecount,mChNum,mSampleRate);

    // ADC and AWB record setting
    mHw->mAfe_handle->Afe_Set_Stream_Attribute(mFormat, *pChannels,mApSampleRate,AFE_MODE_ADC);
    mHw->mAfe_handle->Afe_Set_Stream_Attribute(mFormat, *pChannels,mApSampleRate,AFE_MODE_AWB);
    #ifdef DIGITAL_MIC
    SetDigitalMic();
    #endif

    if(mApSampleRate != mSampleRate){
        uint32 srcBufLen;

        pSrcReadSize = (mSampleRate * mReadBufferSize / mApSampleRate)&0xfffffff0;
        pInputbuflen = (pSrcReadSize<<1);
        pInputSrcWrite =0;
        pInputSrcRead =0;
        LOGD("mApSampleRate=%d mSampleRate=%d need src srcBufSize = %d pSrcReadSize = %d",
            mApSampleRate,mSampleRate,pInputbuflen,pSrcReadSize);
        pInputbuf = new int8[pInputbuflen];
        BLI_GetMemSize( mApSampleRate, mChNum, mSampleRate, mChNum, &srcBufLen);
        pSrcBuf = new int8[srcBufLen];
        pSrcHdl = BLI_Open( mApSampleRate, mChNum, mSampleRate, mChNum, pSrcBuf);
        if ( !pSrcHdl ) {
            return false;
        }
    }
    mFm_Volume = UNITY_GAIN;
    mFm_Vlume_Index = UNITY_GAIN_Index;

    return NO_ERROR;

SET_EXIT:
    // modify default paramters and let Audiorecord openagina for reampler.
   *pFormat = android_audio_legacy::AudioSystem::PCM_16_BIT;
   *pChannels =  channels();
   *pRate = GetRecordSampleRate(*pRate);
    return BAD_VALUE;
}

uint32 AudioYusuApStreamIn::GetRecordSampleRate(uint32_t pRate)
{
    return 16000;
}


bool AudioYusuApStreamIn::SupportedSampleRate(uint32 SampleRate)
{
    if(SampleRate == 8000 || SampleRate == 16000 || SampleRate == 32000 || SampleRate == 48000){
        return true;
    }
    else{
       return false;
    }
}

void AudioYusuApStreamIn::getDataFromModem(void)
{
    usleep(30*1000);
    LOGW("AP should not get data from modem.");
}


uint32_t AudioYusuApStreamIn::sampleRate() const
{
   if(mSampleRate)
       return mSampleRate;
   else
       return 16000;
}

uint32_t AudioYusuApStreamIn::channels() const
{
    return android_audio_legacy::AudioSystem::CHANNEL_IN_STEREO;
}

AudioYusuApStreamIn::AudioYusuApStreamIn()
{
    LOGV("AudioYusuApStreamIn Constructor");
    int ret =0;

    ret = pthread_mutex_init(&mReadMutex, NULL);
    if ( ret != 0 )
        LOGE("Failed to initialize  mReadMutex");
    mHw = NULL;
    mFd = 0;
    mSampleRate = 0;
    mInputSource =0;
    mReadBufferSize = 1024;
    mstandby = true;
    mChNum =0;
    pSrcBuf = NULL;
    pSrcHdl = NULL;
    pInputbuf = NULL;
    // here to maaping FM volume in DB
    FM_Mapping_table = {8,8,14,18,21,23,25,27,28,29,30,31,32,33,34,35};
}

status_t  AudioYusuApStreamIn::standby()
{
    LOGV("AudioYusuApStreamIn standby");
    Mutex::Autolock lock(mLock);
    if(mstandby == false){
        RecClose ();
        mstandby = true;
        mMutecount =(MUTE_APAUDIO_PCM_MS*mSampleRate*mChNum<<1)/1000;
    }
    usleep(1*1000);
    return NO_ERROR;
}


AudioYusuApStreamIn::~AudioYusuApStreamIn()
{
    Mutex::Autolock lock(mLock);
    #ifdef DUMP_AP_STREAMIN
    if(pOutFile){
        fclose(pOutFile);
        pOutFile = NULL;
    }
    #endif
    if(mstandby == false){
        RecClose();
    }
    if(pSrcHdl!= NULL){
        BLI_Close(pSrcHdl);
        pSrcHdl = NULL;
    }
    if(pSrcBuf!=NULL){
        delete[] pSrcBuf;
        pSrcBuf = NULL;
    }
    if(mFd ){
        close(mFd);
        mFd = 0;
    }
    LOGV("~AudioYusuApStreamIn");
}

uint32 AudioYusuApStreamIn::GetSrcbufvalidSize()
{
    if(pInputbuf != NULL){
        if(pInputSrcWrite >=pInputSrcRead){
            return pInputSrcWrite-pInputSrcRead;
        }
        else{
            return pInputSrcRead+pInputbuflen -pInputSrcWrite;
        }
    }
    LOGW("pInputbuf == NULL");
    return 0;
}

uint32 AudioYusuApStreamIn::GetSrcbufFreeSize()
{
    if(pInputbuf != NULL){
        return pInputbuflen - GetSrcbufvalidSize();
    }
    LOGW("pInputbuf == NULL");
    return 0;
}

// here copy SRCbuf to input buffer , and return how many buffer copied
uint32 AudioYusuApStreamIn::CopySrcBuf(char *buffer,uint32 *bytes, char *SrcBuf , uint32 *length)
{
    uint32 consume =0;
    uint32 outputbyes = *bytes;
    //LOGD("+CopySrcBuf consume = %d bytes = %d length = %d",consume,*bytes,*length);
    consume = BLI_Convert(pSrcHdl,(short*)SrcBuf,length,(short*)buffer,bytes);
    //LOGD("-CopySrcBuf consume = %d bytes = %d length = %d",consume,*bytes,*length);
    pInputSrcRead += consume;
    if(pInputSrcRead >= pInputbuflen){
        //LOGD("pInputSrcRead = %d pInputbuflen = %d",pInputSrcRead,pInputbuflen);
        pInputSrcRead -= pInputbuflen;
    }
    *bytes = outputbyes -*bytes;
    return consume;
}

void AudioYusuApStreamIn::StereoToMono(short *buffer , int length)
{
    short left , right;
    int sum;
    int copysize = length>>2;

    while(copysize){
        left = *(buffer);
        right = *(buffer+1);
        sum = (left+right) >> 1;
        *(buffer) = (short)sum;
        *(buffer+1) = (short)sum;
        buffer+=2;
        copysize--;
    }

}

void AudioYusuApStreamIn::CheckNeedMute(void* buffer, int bytes)
{
    if(mMutecount > 0){
        memset(buffer,0,bytes);
        mMutecount -=bytes;
    }
}


void AudioYusuApStreamIn::Dump_StreamIn_PCM(void* buffer, int bytes)
{
   //set this property to dump data
   char value[PROPERTY_VALUE_MAX];
   property_get("streamin.pcm.dump", value, "0");
   int bflag=atoi(value);
   if(bflag)
   {
      char filename[]="/sdcard/StreamIn_Dump.pcm";
      FILE * fp= fopen(filename, "ab+");
      if(fp!=NULL)
      {
         fwrite(buffer,bytes,1,fp);
         fclose(fp);
      }
      else
      {
         LOGE("open  StreamIn_Dump.pcm fail");
      }
   }
}

ssize_t AudioYusuApStreamIn::read(void* buffer, ssize_t bytes)
{
    mLock.lock ();
    ssize_t Read_Size;
    if(mstandby == true){
        //open hardware and start record
        RecOpen ();
        mstandby = false;
        LOGD("AudioYusuApStreamIn::read mstandby = %d",mstandby);
    }
    mLock.unlock ();
    uint32 readbytes = bytes;
    usleep(1*1000);
    Mutex::Autolock _l(mLock);
    uint32 CopyBufferwriteIdx =0,emptybufLen=0;
    if(pSrcHdl && (mApSampleRate != mSampleRate)){
        do{
            //  here need to do SRC and copy to buffer,check if there is any buf in src , exhaust it.
            if(GetSrcbufvalidSize()){
                uint32 bufLen =0;
                uint32 ConsumeReadbytes =0;
                if(pInputSrcWrite >= pInputSrcRead){
                    bufLen = GetSrcbufvalidSize();
                    LOGV("pInputSrcWrite = %d pInputSrcRead = %d pInputbuflen = %d readbytes = %d CopyBufferwriteIdx = %d",
                        pInputSrcWrite,pInputSrcRead,pInputbuflen,readbytes,CopyBufferwriteIdx);
                    ConsumeReadbytes = readbytes;
                    CopySrcBuf((char*)buffer+CopyBufferwriteIdx,&readbytes,pInputbuf+pInputSrcRead,&bufLen);
                    CopyBufferwriteIdx +=ConsumeReadbytes - readbytes;
                    if(readbytes == 0){
						if(CanRecordFM ()){
    					int Volume_FM = mHw->mVolumeController->GetFmVolume ();
						StreamInApplyVolume((short*)buffer,bytes,FM_Map_gain[FM_Mapping_table[Volume_FM]]);
					    }
                        Dump_StreamIn_PCM(buffer,bytes);
                        CheckNeedMute(buffer,bytes);
                        return bytes;
                    }
                }
                else{
                    bufLen = pInputbuflen - pInputSrcRead;
                    LOGV("pInputSrcWrite = %d pInputSrcRead = %d pInputbuflen = %d readbytes = %d CopyBufferwriteIdx = %d",
                        pInputSrcWrite,pInputSrcRead,pInputbuflen,readbytes,CopyBufferwriteIdx);
                    ConsumeReadbytes = readbytes;
                    CopySrcBuf((char*)buffer+CopyBufferwriteIdx,&readbytes,pInputbuf+pInputSrcRead,&bufLen);
                    CopyBufferwriteIdx +=ConsumeReadbytes - readbytes;
                    if(readbytes == 0){
						if(CanRecordFM ()){
    					int Volume_FM = mHw->mVolumeController->GetFmVolume ();
						StreamInApplyVolume((short*)buffer,bytes,FM_Map_gain[FM_Mapping_table[Volume_FM]]);
					    }
                        Dump_StreamIn_PCM(buffer,bytes);
                        CheckNeedMute(buffer,bytes);
                        return bytes;
                    }
                    else{
                        bufLen = pInputSrcWrite - pInputSrcRead;
                        LOGV("pInputSrcWrite = %d pInputSrcRead = %d pInputbuflen = %d readbytes = %d CopyBufferwriteIdx = %d",
                            pInputSrcWrite,pInputSrcRead,pInputbuflen,readbytes,CopyBufferwriteIdx);
                        ConsumeReadbytes = readbytes;
                        CopySrcBuf((char*)buffer+CopyBufferwriteIdx,&readbytes,pInputbuf+pInputSrcRead,&bufLen);
                        CopyBufferwriteIdx +=ConsumeReadbytes - readbytes;
                        if(readbytes == 0){
							if(CanRecordFM ()){
    						int Volume_FM = mHw->mVolumeController->GetFmVolume ();
							StreamInApplyVolume((short*)buffer,bytes,FM_Map_gain[FM_Mapping_table[Volume_FM]]);
					    	}
                            Dump_StreamIn_PCM(buffer,bytes);
                            CheckNeedMute(buffer,bytes);
                            return bytes;
                        }
                    }
                }
            }

            if(pInputSrcRead >= pInputSrcWrite){
                 emptybufLen = GetSrcbufFreeSize();
                 LOGV("read emptybufLen = %d pSrcReadSize = %d readbytes = %d pSrcReadSize =%d",
                     emptybufLen,pSrcReadSize,readbytes,pSrcReadSize);
                 emptybufLen = (pSrcReadSize>= emptybufLen)? emptybufLen : pSrcReadSize;
                 Read_Size = ::read(mFd,pInputbuf+pInputSrcWrite,emptybufLen);
                 StereoToMono((short*)(pInputbuf+pInputSrcWrite),Read_Size);
                 pInputSrcWrite += Read_Size;
                     if(pInputSrcWrite >= pInputbuflen){
                     pInputSrcWrite -= pInputbuflen;
                 }
                 LOGV("read pInputSrcWrite = %d pInputSrcRead = %d pInputbuflen = %d readbytes = %d",
                     pInputSrcWrite,pInputSrcRead,pInputbuflen,readbytes);
             }
             else{
                 emptybufLen = pInputbuflen - pInputSrcWrite;
                 Read_Size = ::read(mFd,pInputbuf+pInputSrcWrite,emptybufLen);
                 StereoToMono((short*)(pInputbuf+pInputSrcWrite),Read_Size);
                 pInputSrcWrite += Read_Size;
                 if(pInputSrcWrite >= pInputbuflen){
                     pInputSrcWrite -= pInputbuflen;
                 }
                 LOGV("read pInputSrcWrite = %d pInputSrcRead = %d pInputbuflen = %d readbytes = %d",
                     pInputSrcWrite,pInputSrcRead,pInputbuflen,readbytes);
             }

             // error handle for read error
             if(Read_Size ==0){
                 LOGW("Read_Size return 0...");
                 usleep(50*1000);
				 if(CanRecordFM ()){
    			 int Volume_FM = mHw->mVolumeController->GetFmVolume ();
				 StreamInApplyVolume((short*)buffer,bytes,FM_Map_gain[FM_Mapping_table[Volume_FM]]);
				 }
                 Dump_StreamIn_PCM(buffer,bytes);
                 CheckNeedMute(buffer,bytes);
                 return bytes;
             }
         }while(readbytes);
     }

    else{
        Read_Size = ::read(mFd,buffer,bytes);
        if(CanRecordFM ()){
            // here to aplpy volume with fn recording
            int Volume_FM = mHw->mVolumeController->GetFmVolume ();
            StreamInApplyVolume((short*)buffer,Read_Size,FM_Map_gain[FM_Mapping_table[Volume_FM]]);
        }
        Dump_StreamIn_PCM(buffer,bytes);
        StereoToMono((short*)(buffer),Read_Size);
        CheckNeedMute(buffer,bytes);
        return bytes;
    }

	if(CanRecordFM ()){
    int Volume_FM = mHw->mVolumeController->GetFmVolume ();
	StreamInApplyVolume((short*)buffer,bytes,FM_Map_gain[FM_Mapping_table[Volume_FM]]);
    }
    Dump_StreamIn_PCM(buffer,bytes);
    CheckNeedMute(buffer,bytes);
    return bytes;
}

status_t AudioYusuApStreamIn::dump(int fd, const Vector<String16>& args)
{
    const size_t SIZE = 256;
    char buffer[SIZE];
    String8 result;
    snprintf(buffer, SIZE, "AudioYusuStreamIn::dump\n");
    result.append(buffer);
    snprintf(buffer, SIZE, "\tsample rate: %d\n", sampleRate());
    result.append(buffer);
    snprintf(buffer, SIZE, "\tbuffer size: %d\n", bufferSize());
    result.append(buffer);
    snprintf(buffer, SIZE, "\tchannel count: %d\n", channels());
    result.append(buffer);
    snprintf(buffer, SIZE, "\tformat: %d\n", format());
    result.append(buffer);
    snprintf(buffer, SIZE, "\tmAudioHardware: %p\n", mHw);
    result.append(buffer);
    snprintf(buffer, SIZE, "\tmFd: %d\n", mFd);
    result.append(buffer);
    ::write(fd, result.string(), result.size());
    return NO_ERROR;
}

status_t AudioYusuApStreamIn::setParameters(const String8& keyValuePairs)
{
    AudioParameter param = AudioParameter(keyValuePairs);
    String8 key = String8(AudioParameter::keyRouting);
    String8 key_input_device = String8(AudioParameter::keyInputSource);

    int input_source;
    int device;
    status_t status = NO_ERROR;
    LOGV("setParameters() %s", keyValuePairs.string());

    if (param.getInt(key, device) == NO_ERROR) {
        mDevice = device;
        param.remove(key);
    }

    if (param.getInt(key_input_device, input_source) == NO_ERROR) {
        LOGD("setParameters, input_source(%d)", input_source);
        mInputSource = input_source;
        if(mInputSource == AUDIO_SOURCE_VOICE_RECOGNITION){
            mMutecount =(MUTE_APAUDIO_PCM_MS*mSampleRate*mChNum<<1)/1000;
            LOGD("setParameters, VOICE_RECOGNITION(%d) mMutecount = %d", input_source,mMutecount);
        }
        else{
            mMutecount =(MUTE_APAUDIO_PCM_MS_NORMAL*mSampleRate*mChNum<<1)/1000;
            LOGD("setParameters, %d mMutecount = %d", input_source,mMutecount);
        }

        param.remove(key_input_device);
    }

    if (param.size()) {
        status = BAD_VALUE;
    }
    return status;
}

String8 AudioYusuApStreamIn::getParameters(const String8& keys)
{
    AudioParameter param = AudioParameter(keys);
    String8 value;
    String8 key = String8(AudioParameter::keyRouting);

    if (param.get(key, value) == NO_ERROR) {
        param.addInt(key, (int)mDevice);
    }

    LOGV("getParameters() %s", param.toString().string());
    return param.toString();
}

bool AudioYusuApStreamIn::IsStandby(void)
{
    return mstandby;
}

void AudioYusuApStreamIn::SetDigitalMic()
{
    uint32 ul_src =0;
    ul_src |= BIT_01;       // set SDM_3_LEVEL
    ul_src |= BIT_21;     // set UL3P25M_MODE CH1
    ul_src |= BIT_22;     // set UL3P25M_MODE CH2
    ul_src |= BIT_23;     // set TWO_WIRE_DIG_MIC
    ul_src &= ~(BIT_05); // use 3.25
    mHw->mAfe_handle->SetAfeReg (AFE_UL_SRC_1,ul_src ,(BIT_01|BIT_05|BIT_21|BIT_22|BIT_23));
}

void AudioYusuApStreamIn::SetMicSource(int mictype){
    LOGD("SetMicSource micsource = %d",mictype);
    mApInputSource = mictype;
    ioctl(mFd,AUD_SET_ANA_CLOCK,1);
    switch (mictype){
        case LADIN_FM_Radio:{
            mHw->mAnaReg->SetAnaReg(AUDIO_CON23,0xDD00,0xffffffff);
            mHw->mAnaReg->SetAnaReg(AUDIO_CON3,0x000C,0x000C);       // power up level shift buffer
            usleep(200);
            mHw->mVolumeController->SetLevelShiftBufferGain ();
            mHw->mAnaReg->SetAnaReg(AUDIO_CON5,0x0001,0xf);       // Select Level-shift buffer input as line-in L channels
            mHw->mAnaReg->SetAnaReg(AUDIO_CON6,0x2533,0xffff);       // Select Level-shift buffer input as line-in R ch
            usleep(200);
            mHw->mAnaReg->SetAnaReg(AUDIO_CON17,0X0008,0xffff);
            break;
        }
        case LADIN_Microphone1:{
            mHw->mAnaReg->SetAnaReg(AUDIO_CON24,0x0,0xffffffff);
            mHw->mAnaReg->SetAnaReg(AUDIO_CON23,0xCC00,0xffffffff);
            usleep(200);
            mHw->mAnaReg->SetAnaReg(AUDIO_CON21,0x1100,0xffffffff);  // left preamp input selection
            if(mChNum == 1){
                mHw->mAnaReg->SetAnaReg(AUDIO_CON22,0x1100,0xffffffff);  // right preamp input selection
            }
            else{
                mHw->mAnaReg->SetAnaReg(AUDIO_CON22,0x1200,0xffffffff);  // right preamp input selection
            }
            break;
        }
        case LADIN_Microphone2:{
            uint32 AudioReg;
            mHw->mAnaReg->SetAnaReg(AUDIO_CON24,0x0,0xffffffff);
            mHw->mAnaReg->SetAnaReg(AUDIO_CON23,0xCC00,0xffffffff);
            usleep(200);
            mHw->mAnaReg->SetAnaReg(AUDIO_CON21,0x1300,0xffffffff);  // left preamp input selection
            mHw->mAnaReg->SetAnaReg(AUDIO_CON22,0x1300,0xffffffff);  // right preamp input selection
            break;
        }
        case LADIN_BTIn:{
            // record BT??
            break;
        }
        case LADIN_SingleDigitalMic:{
            mHw->mAnaReg->SetAnaReg(AUDIO_CON24,0x1,0xffffffff);
            mHw->mAnaReg->SetAnaReg(AUDIO_CON23,0xCC00,0xffffffff);
            mHw->mAnaReg->SetAnaReg(AUDIO_CON21,0x1100,0xffffffff);  // left preamp input selection
            mHw->mAnaReg->SetAnaReg(AUDIO_CON22,0x1100,0xffffffff);  // right preamp input selection
            break;
        }
        case LADIN_DualAnalogMic:{
            mHw->mAnaReg->SetAnaReg(AUDIO_CON24,0x0,0xffffffff);
            mHw->mAnaReg->SetAnaReg(AUDIO_CON23,0xCC00,0xffffffff);
            usleep(200);
            mHw->mAnaReg->SetAnaReg(AUDIO_CON21,0x1100,0xffffffff);  // left preamp input selection
            mHw->mAnaReg->SetAnaReg(AUDIO_CON22,0x1200,0xffffffff);  // right preamp input selection
            break;
        }
        case LADIN_DualDigitalMic:{
            mHw->mAnaReg->SetAnaReg(AUDIO_CON24,0x4,0xffffffff);
            mHw->mAnaReg->SetAnaReg(AUDIO_CON23,0xCC00,0xffffffff);
            mHw->mAnaReg->SetAnaReg(AUDIO_CON21,0x1100,0xffffffff);  // left preamp input selection
            mHw->mAnaReg->SetAnaReg(AUDIO_CON22,0x1200,0xffffffff);  // right preamp input selection
            break;
        }
        case LADIN_FM_AnalogIn_Stereo:{
            mHw->mAnaReg->SetAnaReg(AUDIO_CON23,0xDD00,0xffffffff);
            mHw->mAnaReg->SetAnaReg(AUDIO_CON3,0x000C,0x000C);       // power up level shift buffer
            usleep(200);
            mHw->mVolumeController->SetLevelShiftBufferGain ();
            usleep(200);
            mHw->mAnaReg->SetAnaReg(AUDIO_CON5,0x0001,0xf);       // Select Level-shift buffer input as line-in L channels
            mHw->mAnaReg->SetAnaReg(AUDIO_CON6,0x2533,0xffff);       // Select Level-shift buffer input as line-in R ch
            mHw->mAnaReg->SetAnaReg(AUDIO_CON17,0X0008,0xffff);
            break;
        }
        default:
            break;
    }
    ioctl(mFd,AUD_SET_ANA_CLOCK,0);
}

void AudioYusuApStreamIn::CloseRecordDevicePower()
{
    LOGD("CloseRecordDevicePower mApInputSource = %d",mApInputSource);
    ioctl(mFd,AUD_SET_ANA_CLOCK,1);
    switch(mApInputSource){
        case LADIN_FM_Radio:
        case LADIN_FM_AnalogIn_Stereo:
        {
            if(mHw->mStreamHandler->GetSrcBlockRunning () == false && mHw->GetAnalogLineinEnable ()== false){
                mHw->mAnaReg->SetAnaReg(AUDIO_NCP0,0x0,0xffff);
                mHw->mAnaReg->SetAnaReg(AUDIO_NCP1,0x0,0xffff);
                mHw->mAnaReg->SetAnaReg(AUDIO_LDO0,0x0 ,0xffffffff);
                mHw->mAnaReg->SetAnaReg(AUDIO_LDO1,0x0 ,0xffffffff);
                mHw->mAnaReg->SetAnaReg(AUDIO_LDO2,0x0 ,0xffffffff);
                mHw->mAnaReg->SetAnaReg(AUDIO_GLB0,0x0,0xffff);
                mHw->mAnaReg->SetAnaReg(AUDIO_GLB1,0x0,0xffff);
                mHw->mAnaReg->SetAnaReg(AUDIO_GLB2,0x0,0xffff);
                mHw->mAnaReg->SetAnaReg(AUDIO_CON14,0x0 ,0xc0c0); // uplink preamp gain
            }

            usleep(200);
            mHw->mAnaReg->SetAnaReg(AUDIO_CON31,0x0 ,0xffffffff);
            usleep(200);
            mHw->mAnaReg->SetAnaReg(AUDIO_CON20,0x0 ,0xffffffff); // uplink preamp gain
            mHw->mAnaReg->SetAnaReg(AUDIO_CON34,0x0,0xffffffff);  // VMIC con34
            usleep(200);
            mHw->mAnaReg->SetAnaReg(AUDIO_CON3,0,0x000c);
            mHw->mAnaReg->SetAnaReg(AUDIO_CON4,0x0404,0x3f3f);
            mHw->mAnaReg->SetAnaReg(AUDIO_CON5,0,0x000f);
            mHw->mAnaReg->SetAnaReg(AUDIO_CON6,0,0xf000);
            break;
        }
        case LADIN_Microphone1:
        case LADIN_Microphone2:
        case LADIN_DualAnalogMic:
        {
            if(mHw->mStreamHandler->GetSrcBlockRunning () == false){
                mHw->mAnaReg->SetAnaReg(AUDIO_LDO1,0x0 ,0xffffffff);
                mHw->mAnaReg->SetAnaReg(AUDIO_LDO2,0x0 ,0xffffffff);
                mHw->mAnaReg->SetAnaReg(AUDIO_CON31,0x0 ,0xffffffff);
                mHw->mAnaReg->SetAnaReg(AUDIO_CON14,0x0 ,0xc0c0); // uplink preamp gain
            }
            usleep(200);
            mHw->mAnaReg->SetAnaReg(AUDIO_CON20,0x0 ,0xffffffff); // uplink preamp gain
            mHw->mAnaReg->SetAnaReg(AUDIO_CON34,0x0,0xffffffff);  // VMIC con34
            usleep(200);

            if(mHw->mStreamHandler->GetSrcBlockRunning () == false){
                mHw->mAnaReg->SetAnaReg(AUDIO_NCP0,0x0,0xffff);
                mHw->mAnaReg->SetAnaReg(AUDIO_NCP1,0x0,0x0E00);
                mHw->mAnaReg->SetAnaReg(AUDIO_LDO0,0x0,0x1fff);
                mHw->mAnaReg->SetAnaReg(AUDIO_LDO1,0x0,0xffff);
                mHw->mAnaReg->SetAnaReg(AUDIO_LDO2,0x0,0x0013);
                mHw->mAnaReg->SetAnaReg(AUDIO_GLB0,0x0,0xffff);
                mHw->mAnaReg->SetAnaReg(AUDIO_GLB1,0x0,0xffff);
                mHw->mAnaReg->SetAnaReg(AUDIO_REG1,0x0,0x0001);
            }
            break;
        }
        case LADIN_SingleDigitalMic:
        case LADIN_DualDigitalMic:
        {
            mHw->mAnaReg->SetAnaReg(AUDIO_CON33,0x0,0xffffffff);  // VMIC con34
            mHw->mAnaReg->SetAnaReg(AUDIO_CON34,0x0,0xffffffff);  // VMIC con34
            break;
        }
        case LADIN_BTIn:
        {
            break;
        }
        default:
        {
            LOGW("openRecordDevicePower with no config mHw->GetApMicType () = %d",mHw->GetApMicType ());
        }
    }
    ioctl(mFd,AUD_SET_ANA_CLOCK,0);
}

void AudioYusuApStreamIn::openRecordDevicePower()
{
    LadInPutDevice_Line Device;
    ioctl(mFd,AUD_SET_ANA_CLOCK,1);
    if(CanRecordFM()){
        Device = LADIN_FM_Radio;
    }
    else{
        Device = (LadInPutDevice_Line)mHw->GetApMicType ();
    }
    LOGD("openRecordDevicePower device = %d",Device);
    switch(Device){
        case LADIN_FM_Radio:
        case LADIN_FM_AnalogIn_Stereo:
        {
            if(mHw->mStreamHandler->GetSrcRunningNumber () == false){
                mHw->mAnaReg->SetAnaReg(AUDIO_NCP0,0x102B,0xffff);
                mHw->mAnaReg->SetAnaReg(AUDIO_NCP1,0x0000,0xffff);
                mHw->mAnaReg->SetAnaReg(AUDIO_LDO0,0x1030 ,0xffffffff);
                mHw->mAnaReg->SetAnaReg(AUDIO_LDO1,0x3010 ,0xffffffff);
                mHw->mAnaReg->SetAnaReg(AUDIO_LDO2,0x0013 ,0xffffffff);
                mHw->mAnaReg->SetAnaReg(AUDIO_GLB0,0x3D30,0xffff);
                mHw->mAnaReg->SetAnaReg(AUDIO_GLB1,0x0000,0xffff);
                mHw->mAnaReg->SetAnaReg(AUDIO_GLB2,0x0001,0xffff);
            }
            else{
                mHw->mAnaReg->SetAnaReg(AUDIO_LDO2,0x0013 ,0xffffffff);
                mHw->mAnaReg->SetAnaReg(AUDIO_GLB0,0x3D30,0xffff);
            }
            usleep(200);
            mHw->mAnaReg->SetAnaReg(AUDIO_CON31,0x8000 ,0xffffffff);
            usleep(200);

            mHw->mAnaReg->SetAnaReg(AUDIO_CON20,0x0c0c ,0xffffffff); // uplink preamp gain
            mHw->mAnaReg->SetAnaReg(AUDIO_CON34,0x8000,0xffffffff);  // VMIC con34
            mHw->mAnaReg->SetAnaReg(AUDIO_CON14,0xc0 ,0xc0c0); // uplink preamp gain

            //mHw->mAnaReg->SetAnaReg(AUDIO_CON9,0x3 ,0xf);
            mHw->mAnaReg->SetAnaReg(AUDIO_CON6,0x2a33 ,0xffffffff);
            usleep(200);
            break;
        }
        case LADIN_Microphone1:
        case LADIN_Microphone2:
        case LADIN_DualAnalogMic:
        {
            mHw->mAnaReg->SetAnaReg(AUDIO_LDO1,0x3000 ,0xffffffff);
            mHw->mAnaReg->SetAnaReg(AUDIO_LDO2,0x0013 ,0xffffffff);
            mHw->mAnaReg->SetAnaReg(AUDIO_CON31,0x8000 ,0xffffffff);
            usleep(200);

            mHw->mAnaReg->SetAnaReg(AUDIO_CON20,0x3f3f ,0xffffffff); // uplink preamp gain
            mHw->mAnaReg->SetAnaReg(AUDIO_CON34,0x8000,0xffffffff);  // VMIC con34
            mHw->mAnaReg->SetAnaReg(AUDIO_CON14,0xc0 ,0xc0c0); // uplink preamp gain
            usleep(200);
            if(mHw->mStreamHandler->GetSrcRunningNumber () == false){
                mHw->mAnaReg->SetAnaReg(AUDIO_NCP0,0x102B,0xffff);
                mHw->mAnaReg->SetAnaReg(AUDIO_NCP1,0x0600,0x0E00);
                mHw->mAnaReg->SetAnaReg(AUDIO_LDO0,0x1030,0x1fff);
                mHw->mAnaReg->SetAnaReg(AUDIO_LDO1,0x3010,0xffff);
                mHw->mAnaReg->SetAnaReg(AUDIO_LDO2,0x0013,0x0013);
                mHw->mAnaReg->SetAnaReg(AUDIO_GLB0,0x2920,0xffff);
                mHw->mAnaReg->SetAnaReg(AUDIO_GLB1,0x0000,0xffff);
                mHw->mAnaReg->SetAnaReg(AUDIO_REG1,0x0001,0x0001);
            }
            else{
                mHw->mAnaReg->SetAnaReg(AUDIO_GLB0,0x2920,0xffff);
            }
            break;
        }
        case LADIN_SingleDigitalMic:
        case LADIN_DualDigitalMic:
        {
            mHw->mAnaReg->SetAnaReg(AUDIO_CON33,0x0042,0xffffffff);  // VMIC con34
            mHw->mAnaReg->SetAnaReg(AUDIO_CON34,0x8000,0xffffffff);  // VMIC con34
            break;
        }
        case LADIN_BTIn:
        {
            break;
        }
        default:
        {
            LOGW("openRecordDevicePower with no config mHw->GetApMicType () = %d",mHw->GetApMicType ());
        }
    }
    ioctl(mFd,AUD_SET_ANA_CLOCK,0);
}

void AudioYusuApStreamIn::SetStreamInputSource(int Inputsource)
{
    LOGD("SetStreamInputSource Inputsource= %d mApInputSource = %d",Inputsource,mApInputSource);
    if(Inputsource == mApInputSource){
        return;
    }
    // now
    else if(mApInputSource == LADIN_FM_Radio){
        return;
    }
    else{
        Mutex::Autolock lock(mLock);
        // close and open with new input
        CloseRecordDevicePower ();
        //RecClose ();
        usleep(5*1000);
        mApInputSource = Inputsource;
        //RecOpen ();
        mHw->mVolumeController->SetApMicGain();
        usleep(1*1000);
        SetMicSource (mHw->GetApMicType ());
        openRecordDevicePower ();
    }
}


bool AudioYusuApStreamIn::RecOpen(void)
{
    LOGD("RecOpen mstandby = %d",mstandby);
    if(mstandby == true)
    {
        mHw->mStreamHandler->OutputStreamLock();
        mHw->SwitchAudioClock(true);
        mHw->Set_Recovery_Record(true);
        ioctl(mFd,AUD_SET_ANA_CLOCK,1);
        // start ADC and AWB write back
        LOGD("RecOpen SET_AWB_INPUT_STREAM_STATE");
        ::ioctl(mFd,SET_AWB_INPUT_STREAM_STATE,1);
        usleep(200);
        // set mic gain for recordingAllowed
        mHw->mVolumeController->SetApMicGain();
        //mHw->mAfe_handle->SetAfeReg(AUDIO_TOP_CON0,0x0,0xffffffff);
        //usleep(200);

        #ifdef AUDIO_HQA_SUPPORT
        LOGD("RecOpen,AP InputSource");
        SetMicSource (mHw->GetApMicType ());
        #else
        if(CanRecordFM()){
            SetMicSource (LADIN_FM_Radio);
        }
        else{
            SetMicSource (mHw->GetApMicType ());
        }

        #endif

        //mHw->mAfe_handle->SetAfeReg(AFE_UL_SRC_1,0x08df8df8,0xffffffff);
        mHw->mAfe_handle->SetAWBConenction (true);
        mHw->mAfe_handle->SetAWBChannel(mChNum);
        usleep(200);
        if((mHw->GetApMicType()==LADIN_SingleDigitalMic)||(mHw->GetApMicType()==LADIN_DualDigitalMic)){
            mHw->mAfe_handle->Afe_UL_Start (AFE_MODE_DMIC);
        }else{
            mHw->mAfe_handle->Afe_UL_Start (AFE_MODE_ADC);
        }
        mHw->mAfe_handle->Afe_UL_Start (AFE_MODE_AWB);
        usleep(200);

        openRecordDevicePower ();
        ioctl(mFd,AUD_SET_ANA_CLOCK,0);
        mHw->mStreamHandler->OutputStreamUnLock();

    }
    return true;
}

bool AudioYusuApStreamIn::RecClose(void)
{
    LOGD("RecClose mstandby = %d",mstandby);
    if(mstandby == false){
        LOGD("RecClose SET_AWB_INPUT_STREAM_STATE");
        mHw->mAfe_handle->Afe_UL_Stop (AFE_MODE_ADC);
        usleep(200);
        mHw->mAfe_handle->Afe_UL_Stop (AFE_MODE_AWB);
        ::ioctl(mFd,SET_AWB_INPUT_STREAM_STATE,0);
        usleep(200);
        CloseRecordDevicePower ();
        mstandby = true;
        mHw->SwitchAudioClock(false);
        mHw->Set_Recovery_Record(false);
    }
    return true;
}

bool AudioYusuApStreamIn::CanRecordFM()
{
    int mode;
    mHw->getMode (&mode);
    //LOGD("+CanRecordFM mode:%d, mDevice:%x, FmEnable:%d",mode,mDevice,mHw->GetAnalogLineinEnable());

   if(mode != android_audio_legacy::AudioSystem::MODE_NORMAL)
       return false;

   //if(mHw->GetAnalogLineinEnable() && mHw->GetRecordFM () &&mDevice == android_audio_legacy::AudioSystem::DEVICE_IN_FM )
   if(mHw->GetAnalogLineinEnable() && mHw->GetRecordFM ())
   {
      return true;
   }
   else
   {
      return false;
   }
}


bool AudioYusuApStreamIn::MutexLock(void)
{
    mLock.lock ();
    return true;
}
bool AudioYusuApStreamIn::MutexUnlock(void)
{
    mLock.unlock ();
    return true;
}

// ----------------------------------------------------------------------------
}

