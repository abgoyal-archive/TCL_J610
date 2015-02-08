

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

#define LOG_TAG "AudioYusuStreamIn"
#include <utils/Log.h>
#include <utils/String8.h>

#include "AudioYusuHardware.h"
#include "AudioAfe.h"
#include "AudioAnalogAfe.h"
#include "AudioYusuStreamHandler.h"
#include "AudioYusuVolumeController.h"
#include "AudioYusuLad.h"
#include "AudioYusuCcci.h"
#include <assert.h>
#include <media/mediarecorder.h>

#include "audio_custom_exp.h"
#if defined(MTK_AUDIO_HD_REC_SUPPORT)
#include "AudioCustParam.h"
#endif
extern "C" {
#include "bli_exp.h"
}

//#define DUMP_MODEM_REC_DATA


//////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <cutils/properties.h>
#include <hardware_legacy/power.h>

// Mute the initial PCM data.
// For CTS Verifier - Audio Quality Verifier
#define MUTE_AUDIO_PCM_MS   0
#define MUTE_AUDIO_NORMAL_MS  0


#define MEM_DEV_PATH	"/dev/mem"
#define KMEM_DEV_PATH	"/dev/kmem"

#define REG_BASE       (0x00000000)/* physical address */

#define NORMAL_RECORDING_DEFAULT_MODE    (1)
#define VOICE_REC_RECORDING_DEFAULT_MODE (0)

#define READ_REG_DATA(a,v) v = *((volatile unsigned int *)((base) + ((a)-REG_BASE)))
#define WRITE_REG_DATA(a,v) *((volatile unsigned int *)((base) + ((a)-REG_BASE))) = v

static int dump_md_image()
{
	int fd;
	char c;
	unsigned char *base;
	volatile unsigned int addr, value;

	fd = open(MEM_DEV_PATH, O_RDWR);
	if (fd < 0) {
		LOGD("MEM_DEV_PATH open failed");
		return 1;
	}

	base = (unsigned char *) mmap(NULL, 0x1000000, PROT_READ | PROT_WRITE, MAP_SHARED, fd, REG_BASE);
	if (base == MAP_FAILED) {
		LOGD("mmap failed");
		return 1;
	}

	for(addr = 0; addr<=0xFC; addr+=4)
	{
   	READ_REG_DATA(addr,value);
   	LOGD("Read 0x%x as 0x%x\n", addr, value);
	}
	munmap(base, REG_BASE);
	close(fd);

	return 0;
}

#define CCIF_BASE       (0x80036000)/* physical address */
/* define HW register */
#define CCIF_CON(a) ((volatile unsigned int *)((a) + 0x0000))
#define CCIF_BUSY(a) ((volatile unsigned int *)((a) + 0x0004))
#define CCIF_START(a) ((volatile unsigned int *)((a) + 0x0008))
#define CCIF_TCHNUM(a) ((volatile unsigned int *)((a) + 0x000C))
#define CCIF_RCHNUM(a) ((volatile unsigned int *)((a) + 0x0010))
#define CCIF_ACK(a) ((volatile unsigned int *)((a) + 0x0014))
/* for CHDATA, the first half space belongs to AP and the remaining space belongs to MD */
#define CCIF_TXCHDATA(a) ((volatile unsigned int *)((a) + 0x0100))
#define CCIF_RXCHDATA(a) ((volatile unsigned int *)((a) + 0x0100 + 128))

static int dump_ccci_reg()
{
	int fd, i;
	unsigned char *base;
	unsigned int k;

	fd = open(MEM_DEV_PATH, O_RDWR);
	if (fd < 0) {
		LOGD("MEM_DEV_PATH open failed");
		return 1;
	}

	base = (unsigned char *) mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, fd, CCIF_BASE);
	if (base == MAP_FAILED) {
		LOGD("mmap failed");
		return 1;
	}

	LOGD("CCIF_CON    (0x%p) = %X\n", CCIF_CON(base), *CCIF_CON(base));
	LOGD("CCIF_BUSY   (0x%p) = %X\n", CCIF_BUSY(base), *CCIF_BUSY(base));
	LOGD("CCIF_START  (0x%p) = %X\n", CCIF_START(base), *CCIF_START(base));
	LOGD("CCIF_RCHNUM (0x%p) = %X\n", CCIF_RCHNUM(base), *CCIF_RCHNUM(base));

	munmap(base, CCIF_BASE);
	close(fd);

	return 0;
}
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////


namespace android {

#define RECORDING_NOISE_FLOOR (350)

static void GenRandomNoise(char* buffer,int count)
{
     short *dataptr = (short*)buffer;
     count = count >>1;
     srand ( time(NULL) );
     for(int i=0; i < count ; i++){
         *dataptr =rand()%RECORDING_NOISE_FLOOR-(RECORDING_NOISE_FLOOR>>1);
         dataptr++;
     }
     return;
}

bool AudioYusuStreamIn::CanRecordFM(int mode)
{
   LOGD("+CanRecordFM mode:%d, mDevice:%x, FmEnable:%d",mode,mDevice,mHw->GetAnalogLineinEnable());

   if( (mode == android_audio_legacy::AudioSystem::MODE_NORMAL) && (mHw->GetAnalogLineinEnable() && mHw->GetRecordFM ()) )
   {
      return true;
   }
   else
   {
      return false;
   }
}

void AudioYusuStreamIn::SetHdrecordingMode(int mode , bool *bStereoRecOn)
{
    if( mode == android_audio_legacy::AudioSystem::MODE_IN_CALL ||
            mode == android_audio_legacy::AudioSystem::MODE_IN_COMMUNICATION ){
        LOGD("SetHdrecordingMode mode = %d return ",mode);
        return;
    }

    #if defined(MTK_AUDIO_HD_REC_SUPPORT)
       // HD Record
       uint8 modeIndex = 0;
       int32 u4SceneIdx = 0;
       if(mSetSphEnh == true){ /* HD Record is used when sph enhancement is required */
           if(vGetHdRecordModeInfo(&modeIndex, bStereoRecOn)==true){
               pLad->LAD_SetSpeechMode((ENUM_Speech_Mode)modeIndex);
           }
           else{
               // check 3rd party camcorder
               if (mInputSource != AUDIO_SOURCE_CAMCORDER) {
                   u4SceneIdx = NORMAL_RECORDING_DEFAULT_MODE;
               }
               else {
                   AUDIO_HD_RECORD_SCENE_TABLE_STRUCT rHdRecordSceneTable; // TODO: improve it!!
                   GetHdRecordSceneTableFromNV(&rHdRecordSceneTable);
                   u4SceneIdx = rHdRecordSceneTable.num_voice_rec_scenes + NORMAL_RECORDING_DEFAULT_MODE;//1:cts verifier offset
               }
           
               // here is not RecordMode is selected , choose an default HD record mode ==> now is normal.
               if(vGetHdSpecifictModeInfo(&modeIndex, bStereoRecOn,u4SceneIdx)==true){
                   pLad->LAD_SetSpeechMode((ENUM_Speech_Mode)modeIndex);
               }
               else{
                   pLad->LAD_SetSpeechMode((ENUM_Speech_Mode)SPH_MODE_NORMAL);
               }
           }
       }
       // no need speech enhancement
       else{
           if(vGetHdSpecifictModeInfo(&modeIndex, bStereoRecOn,VOICE_REC_RECORDING_DEFAULT_MODE)==true){
               pLad->LAD_SetSpeechMode((ENUM_Speech_Mode)modeIndex);
           }
           else{
               pLad->LAD_SetSpeechMode((ENUM_Speech_Mode)SPH_MODE_NORMAL);
           }
       }
    #else
       //otherwise , idle record should use normal.
       pLad->LAD_SetSpeechMode((ENUM_Speech_Mode)SPH_MODE_NORMAL);
    #endif

}


bool AudioYusuStreamIn::RecOpen()
{
   bool ret = true;
   m_no_data_count = 0;

   LOGD("+RecOpen, mSampleRateModem:%d mSampleRate = %d",mSampleRateModem,mSampleRate);

   // this will set Mic gain base on current routing
   if(mSetSphEnh && mHw->SetMicGain() == false){
      LOGD("RecOpen SetMicGain -- IN_CALL MODE");
   }

   if(mSetSphEnh == false){
      // VOICE_RECOGNITION, must disable sph enhancement
      // Remember to turn on again if we turn off it.
      int Device= mHw->Audio_Match_Input_device(mDevice,true); // turn on this deivce
      LOGD("+RecOpen, mSetSphEnh=false mDevice = %x",Device);
      if(Device == android_audio_legacy::AudioSystem::DEVICE_IN_WIRED_HEADSET){
          mHw->mVolumeController->SetLadMicGain(Voice_Rec_Mic_Headset);
      }
      else{
          mHw->mVolumeController->SetLadMicGain(Voice_Rec_Mic_Handset);
      }
      //pLad->LAD_Set_Speech_Enhancement(mSetSphEnh);
      pLad->LAD_SetSpeechMode((ENUM_Speech_Mode)SPH_MODE_BT_CORDLESS);
      pLad->LAD_SetInputSource (LADIN_Microphone1);
      pLad->LAD_SetOutputDevice (LADOUT_SPEAKER2);

   }

   //Get Wake Lock for phone call recording
   acquire_wake_lock(PARTIAL_WAKE_LOCK, MD_RECORD_WAKELOCK_NAME);

   //Set Modem Sampling rate
   int32 mode;
   mHw->getMode(&mode);
   if ( mode == android_audio_legacy::AudioSystem::MODE_IN_CALL ) {
      mSampleRateModem = 8000;
   }
   else if ( mSampleRate <= 8000 ) {
      mSampleRateModem = 8000;
   }
   else {
      mSampleRateModem = 16000;
   }

   //Modem Sampling rate convert to index
   int32 srIdx;
   if ( mSampleRateModem == 8000 )
      srIdx = Record_8k;
   else if ( mSampleRateModem == 16000 )
      srIdx = Record_16k;
   else {
      YAD_LOGE("RecOpen modem SR error, use 8k Hz \n");
      srIdx = Record_8k;
   }
   // call this function to decide set whick mode to modem.
   bool bStereoRecOn = false;
   SetHdrecordingMode(mode,&bStereoRecOn);

   // get real ch num for BLI
   uint8 mRealChNum;
#if defined(MTK_AUDIO_HD_REC_SUPPORT)
   if ((mHw->GetVmFlag() == true && mode == android_audio_legacy::AudioSystem::MODE_IN_CALL ) ||
   (mHw->META_Get_DualMic_Test_Mode()==TRUE) ||
   (mHw->GetCtm4WayFlag() == true && mode == android_audio_legacy::AudioSystem::MODE_IN_CALL ) ||
   (mode == android_audio_legacy::AudioSystem::MODE_IN_CALL)){
      mRealChNum = 1;
   }
   else {
      mRealChNum = (bStereoRecOn == true) ? 2 : 1;
      LOGD(" mRealChNum = %d bStereoRecOn = %d",mRealChNum,bStereoRecOn);
   }
#else
   mRealChNum = 1;
#endif

   LOGD("RecOpen Sampling Rate : %d, Modem Sample Rate : %d \n", mSampleRate, mSampleRateModem);

   //If modem sampling rate different from sampling rate, SRC required
   if ( (mSampleRateModem != mSampleRate) || mChNum != 1 && mFormat != android_audio_legacy::AudioSystem::VM_FMT ) {
       LOGD("mSampleRateModem=%d, mRealChNum=%d, mSampleRate=%d need src pSrcHdl = %p mChNum = %d",
           mSampleRateModem,mRealChNum,mSampleRate,pSrcHdl,mChNum);
      uint32 srcBufLen;
      if(pSrcHdl == NULL){
          BLI_GetMemSize( mSampleRateModem, mRealChNum, mSampleRate, mChNum, &srcBufLen);
          pSrcBuf = new int8[srcBufLen];
          pSrcHdl = BLI_Open( mSampleRateModem, mRealChNum, mSampleRate, mChNum, pSrcBuf);
      }
      else{
          LOGD("pSrcHdl exit = %p",pSrcHdl);
      }
      if ( !pSrcHdl ) return false;
   }

   // Add delay 10ms for Recording. Because record process is on-off-on (very quickly. about 2~8 tick counts),
   // modem side DSP has control flow problem. The recorded sound is bad.
   // So just add delay to work around this issue.
   // Modem side will also find out the root cause. If modem side fix this issue, remove this work around.
   usleep(10*1000);
   if( true == CanRecordFM(mode) )
   {


      pLad->LAD_SetInputSource(LADIN_FM_Radio);
      LOGD("RecOpen, LAD_SetInputSource, LADIN_FM_Radio");


   }
   else
   {


//      pLad->LAD_SetInputSource(LADIN_Microphone1);
//      LOGD("RecOpen, LAD_SetInputSource, LADIN_Microphone1");

   }

   //reset read and write pointer of internal buffer
   inBuf.pRead = inBuf.pBufBase;
   inBuf.pWrite = inBuf.pBufBase;
   LOGD("SetRecDropFrameCount(%d)", m_DropFrameCount);
   pLad->LAD_SetRecDropFrameCount(m_DropFrameCount);

   //Start recording
   if ( mHw->GetVmFlag() == true && mode == android_audio_legacy::AudioSystem::MODE_IN_CALL ){
      mHw->Set_Recovery_Record(true);
      ret &= pLad->LAD_OpenNormalRecPath(LADDATA_VM, 0);
      LOGD("-RecOpen, LADDATA_VM");
   }
#if defined(MTK_DUAL_MIC_SUPPORT)||defined(MTK_AUDIO_HD_REC_SUPPORT)
   else if(mHw->META_Get_DualMic_Test_Mode()==TRUE){
      pLad->LAD_SetInputSource(LADIN_DualAnalogMic);
      // set rec level
      uint32 ladLevel = 0xffffff - (MAX_MIC_GAIN_DB - mHw->mDualMicTool_micGain) * 2;                                                                                                           // in 0.5dB
      LOGD("set dual mic rec level level Rec: 0x%x, ladlevel: 0x%x",mHw->mDualMicTool_micGain,ladLevel);
      pLad->LAD_SetMicrophoneVolume(ladLevel);
      // start record
      mHw->Set_Recovery_Record(true);
      ret &= pLad->LAD_OpenNormalRecPath(LADDATA_DUAL_MIC_VM, 0);
      LOGD("-RecOpen, LADDATA_DUAL_MIC");
   }
#endif
   else if ( mHw->GetCtm4WayFlag() == true && mode == android_audio_legacy::AudioSystem::MODE_IN_CALL ){
      mHw->Set_Recovery_Record(true);
      ret &= pLad->LAD_OpenNormalRecPath(LADDATA_CTM_4WAY, 0);
      LOGD("-RecOpen, LADDATA_CTM_4WAY");
   }
#if !defined(MTK_AUDIO_HD_REC_SUPPORT)
   else{
      mHw->Set_Recovery_Record(true);
      ret &= pLad->LAD_OpenNormalRecPath(LADDATA_PCM, srIdx);
   }
#else //MTK_AUDIO_HD_REC_SUPPORT = yes
   else if ( mode == android_audio_legacy::AudioSystem::MODE_IN_CALL ){
      mHw->Set_Recovery_Record(true);
      ret &= pLad->LAD_OpenNormalRecPath(LADDATA_PCM, srIdx);
      LOGD("RecOpen, LADDATA_PCM(IN CALL MODE)");
   }
   else {
      mHw->Set_Recovery_Record(true);
#if defined(MTK_AUDIO_HD_REC_SUPPORT)
      //set this property to dump data
      char value[PROPERTY_VALUE_MAX];

      //set this property to dump vm
      property_get("streamin.vm.dump", value, "0");
      int vflag=atoi(value);
      if(vflag){
          struct tm *timeinfo;
          time_t rawtime;
          char path[80];
          int32 i4format;
          time(&rawtime);
          timeinfo=localtime(&rawtime);
          memset((void*)path,0,80);
          strftime (path,80,"/sdcard/StreamIn_%a_%b_%Y__%H_%M_%S.vm", timeinfo);
          pVMFile = fopen(path,"w");
          if(pVMFile == NULL){
              LOGE("open %s file error\n",path);
              i4format = LADDATA_PCM;
          }else{
              LOGD("open %s file success\n",path);
              i4format = LADDATA_VM;
          }
          ret &= pLad->LAD_OpenNormalRecPath(i4format, srIdx | (bStereoRecOn << 4));
      }else{
          ret &= pLad->LAD_OpenNormalRecPath(LADDATA_PCM, srIdx | (bStereoRecOn << 4));
      }
#else  //#if defined(MTK_AUDIO_HD_REC_SUPPORT)
      ret &= pLad->LAD_OpenNormalRecPath(LADDATA_PCM, srIdx | (bStereoRecOn << 4));
#endif //#if defined(MTK_AUDIO_HD_REC_SUPPORT)
      LOGD("RecOpen, LADDATA_PCM");
   }
#endif
   // when record on and in communication mode , set mic state.
   if(mode == android_audio_legacy::AudioSystem::MODE_IN_COMMUNICATION){
       bool micmute =false;
       mHw->getMicMute (&micmute);
       mHw->setMicMute (micmute);
   }

#ifdef DUMP_MODEM_REC_DATA
   pOutFile = fopen("/data/Record2.pcm","wb");
   if( pOutFile == NULL)
   {
      LOGD("Fail to Open File %s ",pOutFile);
   }
#endif
   LOGD("-RecOpen");
   mstandby = false;

   return ret;
}

bool AudioYusuStreamIn::RecClose()
{
   bool ret=0;
   LOGD("RecClose");
   mstandby = true;

   // Add delay 10ms for Recording. Because record process is on-off-on (very quickly. about 2~8 tick counts),
   // modem side DSP has control flow problem. The recorded sound is bad.
   // So just add delay to work around this issue.
   // Modem side will also find out the root cause. If modem side fix this issue, remove this work around.
   usleep(10*1000);
   ret =  pLad->LAD_CloseNormalRecPath();
//   mHw->Set_Recovery_Record (false);

   pLad->LAD_Set_Speech_Enhancement(true);
   mSetSphEnh   = true;
   //Release Wake Lock for phone call recording
   release_wake_lock(MD_RECORD_WAKELOCK_NAME);
#if defined(MTK_AUDIO_HD_REC_SUPPORT)
   if(pVMFile!=NULL)
   {
       fclose(pVMFile);
       pVMFile = NULL;
   }
#endif
#ifdef DUMP_MODEM_REC_DATA
   fclose(pOutFile);
#endif
   return ret;
}

uint32_t AudioYusuStreamIn::sampleRate() const
{
   if(mSampleRate)
       return mSampleRate;
   else
       return 16000;
}

uint32_t AudioYusuStreamIn::channels() const
{
   LOGD("+channels  %x",mChNum);
   return android_audio_legacy::AudioSystem::CHANNEL_IN_STEREO;
}

status_t AudioYusuStreamIn::set(
        AudioYusuHardware *hw,
	int fd,
	uint32_t devices,
	int *pFormat,
	uint32_t *pChannels,
	uint32_t *pRate,
	android_audio_legacy::AudioSystem::audio_in_acoustics acoustics)
{
   bool ret = true;
   // init mutex
   pthread_mutex_init(&mInputMutex, NULL);
   // FIXME: remove logging
   // check values


   LOGD("+set(%p, %d, %d, %d, %u)", hw, fd, *pFormat, *pChannels, *pRate);
   //only if format is vm or 616bits , stereo
   if ( *pFormat != android_audio_legacy::AudioSystem::VM_FMT&&((*pFormat != android_audio_legacy::AudioSystem::PCM_16_BIT|| *pChannels != channels())|| *pRate != 16000)) {
      YAD_LOGE("Error opening input channel");
      goto SET_EXIT;
   }


   /* set parameters */
   mSampleRate = *pRate;
   mFormat = *pFormat;
   if(*pChannels == android_audio_legacy::AudioSystem::CHANNEL_IN_MONO){
       mChNum = 1;
   }
   else{
       mChNum = 2;
   }

   if(mSampleRate != 0){
       if(mSampleRate == 8000){
           mBufDuration = bufferSize () / 320 *20 / mChNum;
       }
           //mBufDuration = 20;
       else{
           mBufDuration = bufferSize () / 640 *20 / mChNum;
           //mBufDuration = 10;
       }
   }
   LOGD("mBufDuration = %d bufferSize = %d mSampleRate = %d mChNum = %d",mBufDuration,bufferSize(),mSampleRate,mChNum);

   mFd = fd;
   /* get hw pointer */
   mHw = hw;

   /* initialize internal buffer */
   inBuf.pBufBase = new char[AUDIO_IN_BUFFER_LEN];
   inBuf.bufLen = AUDIO_IN_BUFFER_LEN;
   inBuf.pRead = inBuf.pBufBase;
   inBuf.pWrite = inBuf.pBufBase;

   /* set share buffer base and length */
   pLad = mHw->pLad;
   shareBuf.pBufBase = (char *)pLad->pCCCI->GetM2AShareBufAddress();
   shareBuf.bufLen = pLad->pCCCI->GetM2AShareBufLen();

   // initial the lock
#if defined(PC_EMULATION)
   InitializeCriticalSection(&ibMutex);
#else
   pthread_mutex_init(&ibMutex, NULL);
#endif

    YAD_LOGW("set() start to enable recording\n");

   /* start the recording path */
//   ret = RecOpen();
//   mstandby = false;

   if ( ret == false ) {
       YAD_LOGE("set() has error\n");
   	return -1;
   }
   else
   	return NO_ERROR;

SET_EXIT:
    // modify default paramters and let Audiorecord open again for reampler.
   *pFormat  = android_audio_legacy::AudioSystem::PCM_16_BIT;
   *pChannels =  channels();
   *pRate = sampleRate ();
    return BAD_VALUE;
}

size_t  AudioYusuStreamIn::bufferSize() const
{
    if(mFormat == android_audio_legacy::AudioSystem::VM_FMT){
        return 320;
    }

    if(mSampleRate >= 8000){
        return (mSampleRate/16000)*640*8;
    }
    else{
        return 640;
    }
}

AudioYusuStreamIn::AudioYusuStreamIn()
{
    LOGD("AudioYusuStreamIn Constructor = %p",this);
    mHw = NULL;
    mFd = 0;
    memset((void*)&inBuf,0,sizeof(rb));
    memset((void*)&shareBuf,0,sizeof(rb));
    data_nofity_count =0;
    mSampleRate =0;
    mSampleRateModem =0;
    mstandby = true;
    pLad = NULL;
    m_no_data_count = 0;
    mChNum =0;
    pSrcBuf = NULL;
    pSrcHdl = NULL;
    m_mute_count = 0;
    mInputSource = 0;
    mSetSphEnh   = true; // modem side speech enhancement default=on
    mBufDuration = 10;  // default = 10ms
#if defined(MTK_AUDIO_HD_REC_SUPPORT)
    pVMFile = NULL;
#endif
    pthread_mutex_init(&mControlMutex, NULL);
}

AudioYusuStreamIn::~AudioYusuStreamIn()
{
    // FIXME: remove logging
    //LOGD("~AudioYusuStreamIn()");
    MutexLock ();
    if(pLad != NULL){
        int Quenecount = pLad->pCCCI->GetQueueCount();
        LOGD("~StreamIn Q-cnt=%d",Quenecount);
        pLad->pCCCI->RecordLock();
        LOGD("~StreamIn +InBuffer Lock");
        InBuffLock();

        if (mHw != NULL) {
            if(mstandby == false)
                RecClose();
            delete []inBuf.pBufBase;
        }
        if(pSrcHdl!= NULL){
            BLI_Close(pSrcHdl);
            pSrcHdl = NULL;
        }
        if(pSrcBuf!=NULL){
            delete[] pSrcBuf;
            pSrcBuf = NULL;
        }

        InBuffUnLock();
        pLad->pCCCI->RecordUnLock();
        LOGD("~StreamIn -InBuf/Rec Lock");
    }
    MutexUnlock ();
}

int AudioYusuStreamIn::myWaitEvent()
{
#if defined(PC_EMULATION)
   WaitForSingleObject(mHw->LadRecNotifyEvent,INFINITE);
   ResetEvent(mHw->LadRecNotifyEvent);
   return 0;
#else
   pthread_mutex_lock( &mHw->lrnMutex );
   pthread_cond_wait( &mHw->LadRecNotifyEvent, &mHw->lrnMutex );
   pthread_mutex_unlock( &mHw->lrnMutex );
   return 0;
#endif
}

void AudioYusuStreamIn::getSharePointer()
{

   AudioCCCI *pCCCI = pLad->pCCCI;

   shareBuf.pRead = pCCCI->pShareR;
   shareBuf.pWrite = pCCCI->pShareW;
}

void AudioYusuStreamIn::InBuffLock()
{
#if defined(PC_EMULATION)
   EnterCriticalSection(&ibMutex);
#else
   pthread_mutex_lock( &ibMutex );
#endif
}

void AudioYusuStreamIn::InBuffUnLock()
{
#if defined(PC_EMULATION)
   LeaveCriticalSection(&ibMutex);
#else
   pthread_mutex_unlock( &ibMutex );
#endif
}

void AudioYusuStreamIn::ControlLock()
{
#if defined(PC_EMULATION)
   EnterCriticalSection(&mControlMutex);
#else
   pthread_mutex_lock( &mControlMutex );
#endif
}

void AudioYusuStreamIn::ControlUnLock()
{
#if defined(PC_EMULATION)
   LeaveCriticalSection(&mControlMutex);
#else
   pthread_mutex_unlock( &mControlMutex );
#endif
}


void AudioYusuStreamIn::getDataFromModem()
{
   int32 freeSpaceInpBuf, cntShareBuf;

   //lock for buffer access
   InBuffLock();

   if ( mstandby == true ) {
      LOGD("getDataFromModem::StreamIn has standby\n");
      pLad->LAD_NormalRecDataSendBack();
      InBuffUnLock();
      return;
   }

   /* update pointers of share buffer */
   getSharePointer();

   //get free space of internal buffer
   freeSpaceInpBuf = inBuf.bufLen - rb_getDataCount(&inBuf);
//   YAD_LOGV("buffer count :%d\n",rb_getDataCount(&inBuf));

   //get free share buffer data count
   cntShareBuf = rb_getDataCount(&shareBuf);

   //debug use
//   if ( cntShareBuf > 640 )
//      YAD_LOGW("cntShareBuf is big : %d\n", cntShareBuf);

   //Judge if we have enough space in internal buffer
   if ( cntShareBuf * mSampleRate > freeSpaceInpBuf * mSampleRateModem ) {
      YAD_LOGW("recording buffer full\n");
      pLad->LAD_NormalRecDataSendBack();
      InBuffUnLock();
      return;
   }
   /* copy from modem share buffer to input buffer */
   if ( (mSampleRate == mSampleRateModem) && (mChNum == 1)||(mFormat ==android_audio_legacy::AudioSystem::VM_FMT)
#if defined(MTK_AUDIO_HD_REC_SUPPORT)
    || (pVMFile!=NULL)
#endif
    ){
      rb_copyEmpty(&inBuf, &shareBuf);
   }
   else{
       rb_copyEmptySRC(pSrcHdl, &inBuf, &shareBuf, mSampleRate, mSampleRateModem);
   }

   //YAD_LOGW("inBuf b:0x%x r:%d w:%d l:%d\n", inBuf.pBufBase, inBuf.pRead-inBuf.pBufBase, inBuf.pWrite-inBuf.pBufBase, inBuf.bufLen);
   //YAD_LOGW("shareBuf b:0x%x r:%d w:%d l:%d\n", shareBuf.pBufBase, shareBuf.pRead-shareBuf.pBufBase, shareBuf.pWrite-shareBuf.pBufBase, shareBuf.bufLen);

   /* send DataSendBack message */
   if( (data_nofity_count % 200) == 0 ){
       LOGD("getDataFromModem, cnt:%d ",data_nofity_count);
   }
   data_nofity_count ++;

   pLad->LAD_NormalRecDataSendBack();

   //unlock
   InBuffUnLock();

   //YAD_LOGW("data read loop \n");
}

ssize_t AudioYusuStreamIn::read(void* buffer, ssize_t bytes)
{
   int ret = 0;
   int cntInBuf;
   int count = bytes;
   char *buf = (char*)buffer;
   int tryCount = 0, i4Standby = 0;
   //set this property to dump data
   char value[PROPERTY_VALUE_MAX];
   property_get("streamin.pcm.dump", value, "0");
   int bflag=atoi(value);
   i4Standby = mstandby;
   if(i4Standby == true)
   {
       /* Only when we have try to open record, we try to get modemutex */
       /* Use lock to prevent recopen is peform between domoderouting and dooutputdevicerouting */
       if( pthread_mutex_trylock(&mHw->ModeMutex))
       {
           LOGD("read, ModeMutex Lock Fail\n");
           return 0;
       }
   }
   //YAD_LOGW("read: %d of bytes \n", (int32)bytes);
   InBuffLock();
   pthread_mutex_lock(&mInputMutex);
   if(mstandby == true){
       LOGD("read, Start to Record");
       RecOpen ();
   }
   pthread_mutex_unlock(&mInputMutex);
   InBuffUnLock();
   /* Use lock to prevent recopen is peform between domoderouting and dooutputdevicerouting */
   if(i4Standby == true)
   {
       pthread_mutex_unlock(&mHw->ModeMutex);
   }
   /* if internal buffer has enough data for this read, do it and return */
   InBuffLock();
   cntInBuf = rb_getDataCount(&inBuf);
   if ( cntInBuf >= count )
   {
      rb_copyToLinear(buf, &inBuf, count);
#if defined(MTK_AUDIO_HD_REC_SUPPORT)
      if(pVMFile!=NULL)
      {
          fwrite(buffer,bytes,1,pVMFile);
          LOGV("vm write size = %d\n",bytes);
      }
#endif
      if(mSetSphEnh == false && mHw->GetVmFlag () == false){
          if(m_mute_count < (MUTE_AUDIO_PCM_MS/mBufDuration)){
              memset(buf,0,count);
              GenRandomNoise(buf,count);
          }
          m_mute_count++;
      }
      else if(mSetSphEnh== true && mHw->GetVmFlag () == false){
          if(m_mute_count < (MUTE_AUDIO_NORMAL_MS/mBufDuration)){
              //LOGD("m_mute_count = %d mBufDuration = %d",m_mute_count,mBufDuration);
              memset(buf,0,count);
          }
          m_mute_count++;
      }

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
#ifdef DUMP_MODEM_REC_DATA
   fwrite(buf, sizeof(char), count, pOutFile);
#endif

      InBuffUnLock();
      return count;
   }
   InBuffUnLock();

   /* if internal buffer is not enough, keep on trying */
   while (tryCount++ < 30)
   {
      if(tryCount > 12){
         YAD_LOGW("read: try read again (%d) \n",tryCount);
      }

      //sleep some time
      AAD_Sleep_1ms(20);

      //try again
      InBuffLock();
      cntInBuf = rb_getDataCount(&inBuf);
      if ( cntInBuf >= count )
      {
         rb_copyToLinear(buf, &inBuf, count);
#if defined(MTK_AUDIO_HD_REC_SUPPORT)
         if(pVMFile!=NULL)
         {
             fwrite(buffer,bytes,1,pVMFile);
             LOGV("vm write size = %d\n",bytes);
         }
#endif

         if(mSetSphEnh == false && mHw->GetVmFlag () == false){
            if(m_mute_count < (MUTE_AUDIO_PCM_MS/mBufDuration)){
               memset(buf,0,count);
               GenRandomNoise(buf,count);
            }
            m_mute_count++;
         }
         else if(mSetSphEnh== true && mHw->GetVmFlag () == false){
             if(m_mute_count < (MUTE_AUDIO_NORMAL_MS/mBufDuration)){
                 //LOGD("m_mute_count = %d mBufDuration = %d",m_mute_count,mBufDuration);
                 memset(buf,0,count);
             }
             m_mute_count++;
         }

         InBuffUnLock();

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
#ifdef DUMP_MODEM_REC_DATA
         fwrite(buf, sizeof(char), count, pOutFile);
#endif
         return count;
      }
      InBuffUnLock();
   }

   m_no_data_count++;

   if( (mFd != 0) && (m_no_data_count < 10) ){
      ::ioctl(mFd, AUDDRV_LOG_PRINT, 0);// print audio HW register
      //dump_md_image();
      //dump_ccci_reg();
   }

   LOGD("read: No data from Modem \n");
   return 0;
}

status_t AudioYusuStreamIn::standby()
{
    LOGD("standby");
    InBuffLock();
    if(mstandby== false)
    {
        RecClose();
        mstandby = true;
        m_mute_count = 0;
    }
    InBuffUnLock();
    LOGD("standby done");
    return NO_ERROR;
}

int AudioYusuStreamIn::format() const
{
    return mFormat;
}


status_t AudioYusuStreamIn::dump(int fd, const Vector<String16>& args)
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

status_t AudioYusuStreamIn::setParameters(const String8& keyValuePairs)
{
    AudioParameter param = AudioParameter(keyValuePairs);
    String8 key = String8(AudioParameter::keyRouting);
    String8 key_input_device = String8(AudioParameter::keyInputSource);

    status_t status = NO_ERROR;
    int device;
    int input_source;
    LOGD("setParameters() %s", keyValuePairs.string());

    if (param.getInt(key, device) == NO_ERROR) {
        LOGD("setParameters, device(%d)", device);
        mDevice = device;
        param.remove(key);
    }

    if (param.getInt(key_input_device, input_source) == NO_ERROR) {
        LOGD("setParameters, input_source(%d)", input_source);
        mInputSource = input_source;
        switch(mInputSource){
            case AUDIO_SOURCE_DEFAULT:
            case AUDIO_SOURCE_MIC:
            case AUDIO_SOURCE_CAMCORDER:
                mSetSphEnh = true;
                m_DropFrameCount = AUDIO_DROP_FRAME_COUNT_NORMAL;
                break;
            case AUDIO_SOURCE_VOICE_UPLINK:
            case AUDIO_SOURCE_VOICE_DOWNLINK:
            case AUDIO_SOURCE_VOICE_CALL:
            case AUDIO_SOURCE_VOICE_COMMUNICATION:
                mSetSphEnh = true;
                m_DropFrameCount = AUDIO_DROP_FRAME_COUNT_RECORD;
                break;
            case AUDIO_SOURCE_VOICE_RECOGNITION:
                mSetSphEnh = false;
                m_DropFrameCount = AUDIO_DROP_FRAME_COUNT_CTS;
                break;
            default:                
                mSetSphEnh = true;
                m_DropFrameCount = 5;
                break;
        }
       
        param.remove(key_input_device);
    }

    if (param.size()) {
        status = BAD_VALUE;
    }
    return status;
}

String8 AudioYusuStreamIn::getParameters(const String8& keys)
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

bool AudioYusuStreamIn::MutexLock()
{
    LOGD("MutexLock");
    pthread_mutex_lock(&mInputMutex);
    return true;
}

bool AudioYusuStreamIn::MutexUnlock()
{
    LOGD("MutexUnlock");
    pthread_mutex_unlock(&mInputMutex);
    return true;
}

void AudioYusuStreamIn::SetStreamInputSource(int Inputsource){
    if(pLad != NULL){
        LOGD("SetStreamInputSource Inputsource = %d",Inputsource);
        pLad->LAD_SetInputSource((LadInPutDevice_Line)Inputsource);
        if(mHw != NULL){
            int mode = 0;
            mHw->getMode (&mode);
            // handle for voice reg situation
            if(mSetSphEnh == false){
                if(Inputsource  == LADIN_Microphone2){
                    mHw->mVolumeController->SetLadMicGain(Voice_Rec_Mic_Headset);
                }
                else{
                    mHw->mVolumeController->SetLadMicGain(Voice_Rec_Mic_Handset);
                }
                return;
            }
            switch(mode){
                case android_audio_legacy::AudioSystem::MODE_NORMAL:
                case android_audio_legacy::AudioSystem::MODE_RINGTONE:
                    if(Inputsource  == LADIN_Microphone2){
                        mHw->mVolumeController->SetLadMicGain(Idle_Headset_Record);
                    }
                    else{
                        mHw->mVolumeController->SetLadMicGain(Idle_Normal_Record);
                    }
                    break;
                case android_audio_legacy::AudioSystem::MODE_IN_CALL:
                    break;
                case android_audio_legacy::AudioSystem::MODE_IN_COMMUNICATION:
                    if(Inputsource  == LADIN_Microphone2){
                        mHw->mVolumeController->SetLadMicGain(VOIP_Headset_Mic);
                    }
                    else{
                        if(mHw->GetSpeakerStatus ()){
                            mHw->mVolumeController->SetLadMicGain(VOIP_Handfree_Mic);
                        }
                        else{
                            mHw->mVolumeController->SetLadMicGain(VOIP_Normal_Mic);
                        }
                    }
                    break;
                default:
                    break;
            }
        }
    }
}

#if defined(MTK_AUDIO_HD_REC_SUPPORT)
bool AudioYusuStreamIn::vGetHdSpecifictModeInfo(uint8 *modeIndex, bool *bStereoRecOn, int AssignedModeIndex)
{
    bool  ret = false;
    int32 u4SceneIdx = AssignedModeIndex;  // use voice normal as default record parameter
    *bStereoRecOn = false;

    LOGD("vGetHdSpecifictModeInfo modeIndex = %d u4SceneIdx = %d",*modeIndex,u4SceneIdx);
    // get scene table
    AUDIO_HD_RECORD_SCENE_TABLE_STRUCT hdRecordSceneTable;
    GetHdRecordSceneTableFromNV(&hdRecordSceneTable);

    // get hd rec param
    AUDIO_HD_RECORD_PARAM_STRUCT hdRecordParam;
    GetHdRecordParamFromNV(&hdRecordParam);

    #if 1 // update hd rec param
            bUpdateHdRecordMappingTable(&hdRecordSceneTable, &hdRecordParam);
    #endif

    // get input device
    uint32 u4CurrentDevice = mHw->Audio_Match_Input_device(mHw->mInputAvailable, true);

    #if 0 // Not support BT Earphone Mic yet
    if (hdRecordSceneTable.scene_table[u4SceneIdx][HD_REC_DEVICE_SOURCE_BT_EARPHONE] != 0xFF
        && u4CurrentDevice == android_audio_legacy::AudioSystem::DEVICE_IN_BLUETOOTH_SCO_HEADSET)
    {
        *modeIndex = hdRecordSceneTable.scene_table[u4SceneIdx][HD_REC_DEVICE_SOURCE_BT_EARPHONE];
        LOGD("vGetHdSpecifictModeInfo: BT_EARPHONE,  modeIndex = %d", *modeIndex);
    }
    // HeadsetFd
    else
    #endif
    if (hdRecordSceneTable.scene_table[u4SceneIdx][HD_REC_DEVICE_SOURCE_HEADSET] != 0xFF
        && u4CurrentDevice == android_audio_legacy::AudioSystem::DEVICE_IN_WIRED_HEADSET)
    {
        *modeIndex = hdRecordSceneTable.scene_table[u4SceneIdx][HD_REC_DEVICE_SOURCE_HEADSET];
        LOGD("(RecOpen)vGetHdSpecifictModeInfo: HEADSET,  modeIndex = %d", *modeIndex);
    }
    // Handset MIC_GAIN_MODE
    else if (hdRecordSceneTable.scene_table[u4SceneIdx][HD_REC_DEVICE_SOURCE_HANDSET] != 0xFF)
    {
        *modeIndex = hdRecordSceneTable.scene_table[u4SceneIdx][HD_REC_DEVICE_SOURCE_HANDSET];
        #if defined(MTK_DUAL_MIC_SUPPORT)
        /* only stereo flag is true, the stereo record is enabled */
        if(hdRecordParam.hd_rec_map_to_stereo_flag[*modeIndex]!=0){
            *bStereoRecOn = true;
        }
        LOGD("(RecOpen)vGetHdSpecifictModeInfo: HANDSET,  modeIndex = %d bStereoRecOn = %d", *modeIndex,*bStereoRecOn);
        #endif
    }
    else {
        LOGD("(RecOpen)vGetHdSpecifictModeInfo: Handset mode index shoule not be -1");
    }

    #if 1 // Debug printBinary
        LOGD("(RecOpen)vGetHdSpecifictModeInfo: map_fir_ch1=%d, map_fir_ch2=%d, device_mode=%d",
            hdRecordParam.hd_rec_map_to_fir_for_ch1[*modeIndex],
            hdRecordParam.hd_rec_map_to_fir_for_ch2[*modeIndex],
            hdRecordParam.hd_rec_map_to_dev_mode[*modeIndex]);
    #endif
    *modeIndex += HD_RECORD_MODE_OFFSET;
     ret = true;
     return ret;
}

bool AudioYusuStreamIn::vGetHdRecordModeInfo(uint8 *modeIndex, bool *bStereoRecOn)
{
    bool  ret = false;
    int32 u4SceneIdx = mHw->mHdRecScene;
    mHw->mHdRecScene = -1; //reset record scene
    *bStereoRecOn = false;

    LOGD("(RecOpen)+vGetHdRecordModeInfo: u4SceneIdx = %d", u4SceneIdx);
    if(u4SceneIdx >= 0)
    {
        // get scene table
        AUDIO_HD_RECORD_SCENE_TABLE_STRUCT hdRecordSceneTable;
        GetHdRecordSceneTableFromNV(&hdRecordSceneTable);

        // get hd rec param
        AUDIO_HD_RECORD_PARAM_STRUCT hdRecordParam;
        GetHdRecordParamFromNV(&hdRecordParam);

#if 1 // update hd rec param
        bUpdateHdRecordMappingTable(&hdRecordSceneTable, &hdRecordParam);
#endif

#if 0 // Debug print
        uint8 total_num_scenes = hdRecordSceneTable.num_voice_rec_scenes + hdRecordSceneTable.num_video_rec_scenes;
        for(int i=0; i<total_num_scenes; i++)
            for(int j=0; j<NUM_HD_REC_DEVICE_SOURCE; j++)
                LOGD("vGetHdRecordModeInfo, scene_table[%d][%d] = %d", i,j,hdRecordSceneTable.scene_table[i][j]);
#endif

        // get mode index
        uint32 u4CurrentDevice = mHw->Audio_Match_Input_device(mHw->mInputAvailable, true);
#if 0 // Not support BT Earphone Mic yet
        if (hdRecordSceneTable.scene_table[u4SceneIdx][HD_REC_DEVICE_SOURCE_BT_EARPHONE] != 0xFF
            && u4CurrentDevice == AudioSystem::DEVICE_IN_BLUETOOTH_SCO_HEADSET)
        {
            *modeIndex = hdRecordSceneTable.scene_table[u4SceneIdx][HD_REC_DEVICE_SOURCE_BT_EARPHONE];
            LOGD("vGetHdRecordModeInfo: BT_EARPHONE,  modeIndex = %d", *modeIndex);
        }
        // Headset
        else
#endif
        if (hdRecordSceneTable.scene_table[u4SceneIdx][HD_REC_DEVICE_SOURCE_HEADSET] != 0xFF
            && u4CurrentDevice == android_audio_legacy::AudioSystem::DEVICE_IN_WIRED_HEADSET)
        {
            *modeIndex = hdRecordSceneTable.scene_table[u4SceneIdx][HD_REC_DEVICE_SOURCE_HEADSET];
            LOGD("(RecOpen)vGetHdRecordModeInfo: HEADSET,  modeIndex = %d", *modeIndex);
        }
        // Handset Mic
        else if (hdRecordSceneTable.scene_table[u4SceneIdx][HD_REC_DEVICE_SOURCE_HANDSET] != 0xFF)
        {
            *modeIndex = hdRecordSceneTable.scene_table[u4SceneIdx][HD_REC_DEVICE_SOURCE_HANDSET];
            LOGD("(RecOpen)vGetHdRecordModeInfo: HANDSET,  modeIndex = %d", *modeIndex);
#if defined(MTK_DUAL_MIC_SUPPORT)
            /* only stereo flag is true, the stereo record is enabled */
            if(hdRecordParam.hd_rec_map_to_stereo_flag[*modeIndex]!=0){
                *bStereoRecOn = true;
            }
#endif
        }
        else {
            LOGD("(RecOpen)vGetHdRecordModeInfo: Handset mode index shoule not be -1");
        }

#if 1 // Debug print
        LOGD("(RecOpen)vGetHdRecordModeInfo: map_fir_ch1=%d, map_fir_ch2=%d, device_mode=%d",
            hdRecordParam.hd_rec_map_to_fir_for_ch1[*modeIndex],
            hdRecordParam.hd_rec_map_to_fir_for_ch2[*modeIndex],
            hdRecordParam.hd_rec_map_to_dev_mode[*modeIndex]);
#endif

        *modeIndex += HD_RECORD_MODE_OFFSET;
        ret = true;
    }else{
        *modeIndex = 0;
        ret = false;
    }
    LOGD("(RecOpen)-vGetHdRecordModeInfo: ENUM_HD_Record_Mode = %d, bStereoRecOn = %d", *modeIndex, *bStereoRecOn);
    return ret;
}

bool AudioYusuStreamIn::bUpdateHdRecordMappingTable(
    AUDIO_HD_RECORD_SCENE_TABLE_STRUCT *hdRecordSceneTable,
    AUDIO_HD_RECORD_PARAM_STRUCT *hdRecordParam )
{
    bool retval;
    uint8 modeIndex;

    LOGD("(RecOpen)+bUpdateHdRecordMappingTable");

    // set fir index & dev mode
    //uint8 total_num_scenes = hdRecordSceneTable->num_voice_rec_scenes + hdRecordSceneTable->num_video_rec_scenes;
    uint8 total_num_scenes = MAX_HD_REC_SCENES;
    for(int i=0; i<total_num_scenes; i++){
        // Handset
        modeIndex = hdRecordSceneTable->scene_table[i][HD_REC_DEVICE_SOURCE_HANDSET];
        if (modeIndex != 0xFF) {
            hdRecordParam->hd_rec_map_to_fir_for_ch1[modeIndex] = SPH_FIR_COEFF_NORMAL;
            hdRecordParam->hd_rec_map_to_fir_for_ch2[modeIndex] = SPH_FIR_COEFF_HANDSET_MIC2;
            hdRecordParam->hd_rec_map_to_dev_mode[modeIndex] = SPH_MODE_NORMAL;
        }

        // Headset
        modeIndex = hdRecordSceneTable->scene_table[i][HD_REC_DEVICE_SOURCE_HEADSET];
        if (modeIndex != 0xFF) {
            hdRecordParam->hd_rec_map_to_fir_for_ch1[modeIndex] = SPH_FIR_COEFF_HEADSET;
            hdRecordParam->hd_rec_map_to_fir_for_ch2[modeIndex] = SPH_FIR_COEFF_NONE;
            hdRecordParam->hd_rec_map_to_dev_mode[modeIndex] = SPH_MODE_EARPHONE;
        }
#if 0
        // BT Earphone
        modeIndex = hdRecordSceneTable.scene_table[i][HD_REC_DEVICE_SOURCE_BT_EARPHONE];
        if (modeIndex != 0xFF) {
            hdRecordParam.hd_rec_map_to_fir_for_ch1[modeIndex] = SPH_FIR_COEFF_BT;
            hdRecordParam.hd_rec_map_to_fir_for_ch2[modeIndex] = SPH_FIR_COEFF_NONE;
            hdRecordParam.hd_rec_map_to_dev_mode[modeIndex] = SPH_MODE_BT_EARPHONE;
        }
#endif
    }

    // specific for voice recognitaion
    {
        // fix me , need hardware core for voice rec??
        int i=VOICE_REC_RECORDING_DEFAULT_MODE;
          // Handset
        modeIndex = hdRecordSceneTable->scene_table[i][HD_REC_DEVICE_SOURCE_HANDSET];
        LOGD("modeIndex = %d",modeIndex);
        if (modeIndex != 0xFF) {
            hdRecordParam->hd_rec_map_to_fir_for_ch1[modeIndex] = SPH_FIR_COEFF_VOICE_REC;
            hdRecordParam->hd_rec_map_to_fir_for_ch2[modeIndex] = SPH_FIR_COEFF_HANDSET_MIC2;
            hdRecordParam->hd_rec_map_to_dev_mode[modeIndex] = SPH_MODE_NORMAL;
        }

        // Headset
        modeIndex = hdRecordSceneTable->scene_table[i][HD_REC_DEVICE_SOURCE_HEADSET];
        if (modeIndex != 0xFF) {
            hdRecordParam->hd_rec_map_to_fir_for_ch1[modeIndex] = SPH_FIR_COEFF_VOICE_REC;
            hdRecordParam->hd_rec_map_to_fir_for_ch2[modeIndex] = SPH_FIR_COEFF_HANDSET_MIC2;
            hdRecordParam->hd_rec_map_to_dev_mode[modeIndex] = SPH_MODE_EARPHONE;
        }
#if 0
        // BT Earphone
        modeIndex = hdRecordSceneTable.scene_table[i][HD_REC_DEVICE_SOURCE_BT_EARPHONE];
        if (modeIndex != 0xFF) {
            hdRecordParam.hd_rec_map_to_fir_for_ch1[modeIndex] = SPH_FIR_COEFF_BT;
            hdRecordParam.hd_rec_map_to_fir_for_ch2[modeIndex] = SPH_FIR_COEFF_NONE;
            hdRecordParam.hd_rec_map_to_dev_mode[modeIndex] = SPH_MODE_BT_EARPHONE;
        }
#endif
    }

    // save to nvram
    SetHdRecordParamToNV(hdRecordParam);

    // send to MD side
    retval = pLad->LAD_SetHdRecordParameters(hdRecordParam);

    LOGD("(RecOpen)-bUpdateHdRecordMappingTable");

    return retval;
}
#endif


// ----------------------------------------------------------------------------
}

