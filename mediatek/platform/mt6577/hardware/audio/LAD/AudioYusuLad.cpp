



#define LOG_TAG "AudioLAD"
#include <sys/resource.h>
#include <sys/time.h>
#include <fcntl.h>
#include <sys/types.h>
#include <errno.h>
#include "AudioYusuLad.h"
#include "CFG_AUDIO_File.h"
#include "AudioCustParam.h"
#include "AudioYusuHeadsetMessage.h"
#include "AudioYusuCcci.h"
#include "AudioYusuLadPlayer.h"
#include "audio_custom_exp.h"
#include <cutils/properties.h>

#define MD_INIT_FILE "/sys/devices/virtual/BOOT/BOOT/boot/md"

#if defined(MTK_SPH_EHN_CTRL_SUPPORT)
#define SPH_ENH_INFO_FILENAME "/data/system/sph_enh.info"
#endif

#ifdef ENABLE_LOG_LAD
    #define LOG_LAD LOGD
#else
    #define LOG_LAD LOGV
#endif

namespace android {


LAD::LAD(AudioYusuHardware *hw)
{
   /* set hw pointer */
   mHw = hw;

   //set TTY state in phone call
   mTty_used_in_phone_call = false;

   // modem side Speech enhancement enable. Default=on
   mSph_Enh_enable = true;
   m_SideToneGain = 0;
   m_MicGain = 0;
   m_InputSrc = (int)0;

#if defined(MTK_SPH_EHN_CTRL_SUPPORT)
   // default all enhancement on
   mSphEnhInfo.spe_usr_mask = SPH_ENH_MASK_ALL;
   mSphEnhInfo.spe_usr_subFunc_mask = SPH_ENH_SUBFUNC_MASK_VCE;

   FILE *f_sph_enh = fopen(SPH_ENH_INFO_FILENAME, "rb");
   if (f_sph_enh == NULL)
   {
       LOGD("%s doesn't exist, create a file", SPH_ENH_INFO_FILENAME);
       f_sph_enh = fopen(SPH_ENH_INFO_FILENAME, "wb");

       if(f_sph_enh == NULL)
       {
           LOGE("cannot create file %s in consturoctor", SPH_ENH_INFO_FILENAME);
       }
       else
       {
           chmod(SPH_ENH_INFO_FILENAME, 0644);
           fwrite(&mSphEnhInfo, sizeof(SPH_ENH_INFO_T), 1, f_sph_enh);
           fclose(f_sph_enh);
       }
   }
   else
   {
       LOGD("%s exists", SPH_ENH_INFO_FILENAME);
       fread(&mSphEnhInfo, sizeof(SPH_ENH_INFO_T), 1, f_sph_enh);
       fclose(f_sph_enh);
   }
#endif //MTK_SPH_EHN_CTRL_SUPPORT


   /* create HeadsetMessage*/
   mHeadSetMessager = new AudioYusuHeadSetMessager(this);
   LOG_LAD("AudioYusuHeadSetMessager init  finish");
   /* create CCCI */
   pCCCI = new AudioCCCI(hw);

}


//This function should be called immediately after the creation of LAD
bool LAD::LAD_Initial()
{
   int ret;
   int count = 0;
   int NeedUnlockSpeechCoeff = 0;
   AUDIO_CUSTOM_PARAM_STRUCT sndParam;
#if defined(MTK_DUAL_MIC_SUPPORT)
   AUDIO_CUSTOM_EXTRA_PARAM_STRUCT dualMicParam;
#endif
#if defined(MTK_WB_SPEECH_SUPPORT)
   AUDIO_CUSTOM_WB_PARAM_STRUCT sndWbParam;
#endif
   LOG_LAD("LAD_Initial start");

   // initialize CCCI
   ret = pCCCI->Initial();
   if(ret == false) {
   	LOGD("LAD Initial fail");
   	return ret;
   }
	LOG_LAD("LAD_Initial success");

   // set Parameters from NV ram, if no NV ram, uses default value
   bool rest;
   rest = LAD_WaitUntilModemReady();
   LOG_LAD("LAD_WaitUntilModemReady (%d)",rest);

   GetCustParamFromNV(&sndParam);
   //YAD_LOGW("Digi_DL_Speech = %u",sndParam.Digi_DL_Speech);
   LOG_LAD("uMicbiasVolt=%u",sndParam.uMicbiasVolt);

   // Get SphCoeffLock for prevent been interruptted by md reset
   if(pCCCI->SpeechCoeffLock() == false){
       LOGE(AUDMSG"LAD_Init fail to get SpeechCoeffLock for ensuring share buffer control is completed\n");
   }else{
       NeedUnlockSpeechCoeff = 1;
   }

   ret = LAD_SetSpeechParameters(&(sndParam));
   LOG_LAD("+LAD_SetSpeechParameters ret=%d",ret);

   while(ret < 0)
   {
      LOG_LAD("Wait modem ready ... %d (sec)",count);
      count++;
      if(count > 10)
         break;

      // because AudioFlinger is pipeline with other process, not attect the boot up time.
      usleep(1000*1000);
      ret = LAD_SetSpeechParameters(&(sndParam));
   }
   LOG_LAD("-LAD_SetSpeechParameters ret=%d",ret);

#if defined(MTK_DUAL_MIC_SUPPORT)
   LOGD("LAD_Initial Read Dual Mic NVRAM");
   Read_DualMic_CustomParam_From_NVRAM(&dualMicParam);
   ret = LAD_SetDualMicParameters(&(dualMicParam));
#endif

#if defined(MTK_WB_SPEECH_SUPPORT)
   GetCustWBParamFromNV(&sndWbParam);
   ret = LAD_SetWBSpeechParameters(&(sndWbParam));
   LOG_LAD("LAD_SetWBSpeechParameters ret=%d",ret);
#endif

#if defined(MTK_AUDIO_HD_REC_SUPPORT)
   LAD_SetMDCapability(1);//bit 0:Enable Audio HD Record Support
   LOG_LAD("+LAD_SetHdRecordParameters");
   AUDIO_HD_RECORD_PARAM_STRUCT hdRecordParam;
   GetHdRecordParamFromNV(&hdRecordParam);
   ret = LAD_SetHdRecordParameters(&hdRecordParam);
   LOG_LAD("-LAD_SetHdRecordParameters, ret=%d", ret);
#endif
   //The 1st time to set speech mode after booting up
   LAD_SetSpeechMode(SPH_MODE_NORMAL);
#if defined(USE_REFMIC_IN_LOUDSPK)
   //Set LoudSPK Mic mode
   LAD_Set_LoudSPK_RefMicMode(USE_REFMIC_IN_LOUDSPK);
#endif

   //LAD_GetMDCapability();

   //Use lock to ensure the previous command with share buffer control is completed
   if(pCCCI->A2MBufLock() == false){
       LOGE(AUDMSG"LAD_Init fail to get A2M Buffer for ensuring share buffer control is completed\n");
   }else{
       pCCCI->A2MBufUnLock();
   }

   if(NeedUnlockSpeechCoeff == 1){
       pCCCI->SpeechCoeffUnLock();
   }

   return true;
}

//This function should be called immediately after the creation of LAD
bool LAD::LAD_Deinitial()
{
   bool ret;
   // initialize CCCI
   ret = pCCCI->Deinitial();
   if(ret == false) {
   	LOGD("LAD_Deinitial fail");
   	return ret;
   }

   return true;
}



LAD::~LAD()
{
   delete(pCCCI);
}


bool LAD::LAD_WaitUntilModemReady()
{
   int fd;
   int s;
   int trycnt = 0;
   volatile char data[20];

   fd = open(MD_INIT_FILE, O_RDONLY);
   LOG_LAD("LAD_WaitUntilModemReady with fd = %x",fd);
   if (fd < 0) {
      LOGD("fail to open %s: errno: %d ", MD_INIT_FILE, errno);
      return false;
   }

   trycnt = 0;
   do {
      memset((void *)data, 0, sizeof(char) * 20);
      lseek(fd, 0, SEEK_SET);
      s = read(fd, (void *)data, sizeof(char) * 3);
      if (data[0] == '2') {
         break;
      }
      else {
         sleep(1);
         LOGD("LAD wait CCCI open %d sec ", trycnt);
         if ( trycnt++ > 20 ) break;
      }
   } while (1);

exit:
   close(fd);

   return true;
}


/*---------------Recording Functions---------------------*/

/* open recording */
bool LAD::LAD_OpenNormalRecPath(int32 format, int32 sampleRate)
{
   pCCCI->mHw->SwitchAudioClock(true);
   //SendMsgInQueue( DataTransfer(M2A_Record_turn_on, (Mode<<0xc)));
   int data = (sampleRate << 12) | format;
   LOGD(AUDMSG"LAD Record_On (0x%x), data=%x", A2M_Record_turn_on, data);
   return pCCCI->SendMessageInQueue( CCCI_MSG1(A2M_Record_turn_on, data));
}

bool LAD::LAD_CloseNormalRecPath()
{
   m_MicGain = 0;
   m_InputSrc = (int)0;
   //SendMsgInQueue( CCCI_MSG1(M2A_Record_turn_off, 0));
   LOGD(AUDMSG"LAD Record_Off (0x%x)", A2M_Record_turn_off);
   return pCCCI->SendMessageInQueue( CCCI_MSG1(A2M_Record_turn_off, 0));
}

bool LAD::LAD_NormalRecDataSendBack()
{
//   LOG_LAD(AUDMSG"LAD_NormalRecDataSendBack, f:%x",A2M_Record_DataSendBack);
   return pCCCI->SendMessageInQueue( CCCI_MSG1(A2M_Record_DataSendBack, 0));
}

bool LAD::LAD_SetRecDropFrameCount( int32 frame_count)
{
   int data = (frame_count<0)? 0: frame_count;
   LOGD(AUDMSG"LAD LAD_SetRecDropFrameCount (0x%x), data=%x", A2M_Record_Drop_Frame, data);
   return pCCCI->SendMessageInQueue( CCCI_MSG1(A2M_Record_Drop_Frame, data));
}

/*-----------------PCM 2Way Functions---------------------*/

bool LAD::LAD_PCM2WayPlayOn()
{
   if( mPCM2WayState == 0 ) {
      //nothing is on, just turn it on
      mPCM2WayState |= PCM2WAY_PLAY_ON;
      LOG_LAD(AUDMSG"LAD_PCM2WayPlayOn, A2M_PCM_OPEN1");
      return pCCCI->SendMessageInQueue(CCCI_MSG1(A2M_PCM_OPEN, mPCM2WayState));
   }

   else if ( mPCM2WayState == PCM2WAY_PLAY_ON  )  {
      // only play on, return trun;
      return true;
   }

   else if ( mPCM2WayState == PCM2WAY_REC_ON  )  {
      // only rec is on, turn off, modify state and turn on again
      bool ret;
      mPCM2WayState |= PCM2WAY_PLAY_ON;
      LOG_LAD(AUDMSG"LAD_PCM2WayPlayOn, A2M_PCM_Close");
      ret = pCCCI->SendMessageInQueue(CCCI_MSG1(A2M_PCM_Close, 0));
      LOG_LAD(AUDMSG"LAD_PCM2WayPlayOn, A2M_PCM_OPEN2");
      ret &= pCCCI->SendMessageInQueue(CCCI_MSG1(A2M_PCM_OPEN, mPCM2WayState));
      return ret;
   }

   else if ( mPCM2WayState == (PCM2WAY_REC_ON | PCM2WAY_PLAY_ON) ) {
      // both on, return true
      return true;
   }

   else {
      return false;
   }
}

bool LAD::LAD_PCM2WayPlayOff()
{
   if( mPCM2WayState == 0 ) {
      //nothing is on, return true
      return true;
   }

   else if ( mPCM2WayState == PCM2WAY_PLAY_ON  )  {
      // only play on, return trun;
      LOG_LAD(AUDMSG"LAD_PCM2WayPlayOff, A2M_PCM_Close1");
      return pCCCI->SendMessageInQueue(CCCI_MSG1(A2M_PCM_Close, 0));
   }

   else if ( mPCM2WayState == PCM2WAY_REC_ON  )  {
      // only rec on, return trun;
      return true;
   }

   else if ( mPCM2WayState == (PCM2WAY_REC_ON | PCM2WAY_PLAY_ON) ) {
      // both rec and play on, turn off, modify state and turn on again
      bool ret;
      mPCM2WayState &= (~PCM2WAY_PLAY_ON);
      LOG_LAD(AUDMSG"LAD_PCM2WayPlayOff, A2M_PCM_Close2");
      ret = pCCCI->SendMessageInQueue(CCCI_MSG1(A2M_PCM_Close, 0));
      LOG_LAD(AUDMSG"LAD_PCM2WayPlayOff, A2M_PCM_OPEN");
      ret &= pCCCI->SendMessageInQueue(CCCI_MSG1(A2M_PCM_OPEN, mPCM2WayState));
      return ret;
   }
   else {
      return false;
   }
}



bool LAD::LAD_PCM2WayRecordOn()
{
   if( mPCM2WayState == 0 ) {
      //nothing is on, just turn it on
      mPCM2WayState |= PCM2WAY_REC_ON;
      LOG_LAD(AUDMSG"LAD_PCM2WayRecordOn, A2M_PCM_OPEN1");
      return pCCCI->SendMessageInQueue(CCCI_MSG1(A2M_PCM_OPEN, mPCM2WayState));
   }

   else if ( mPCM2WayState == PCM2WAY_PLAY_ON  )  {
      // only play is on, turn off, modify state and turn on again
      bool ret;
      mPCM2WayState |= PCM2WAY_REC_ON;
      LOG_LAD(AUDMSG"LAD_PCM2WayRecordOn, A2M_PCM_Close");
      ret = pCCCI->SendMessageInQueue(CCCI_MSG1(A2M_PCM_Close, 0));
      LOG_LAD(AUDMSG"LAD_PCM2WayRecordOn, A2M_PCM_OPEN2");
      ret &= pCCCI->SendMessageInQueue(CCCI_MSG1(A2M_PCM_OPEN, mPCM2WayState));
      return ret;
   }

   else if ( mPCM2WayState == PCM2WAY_REC_ON  )  {
     // only rec on, return true;
      return true;
   }

   else if ( mPCM2WayState == (PCM2WAY_REC_ON | PCM2WAY_PLAY_ON) ) {
      return true;
   }

   else {
      return false;
   }
}

bool LAD::LAD_PCM2WayRecordOff()
{
   if( mPCM2WayState == 0 ) {
      //nothing is on, return true
      return true;
   }

   else if ( mPCM2WayState == PCM2WAY_PLAY_ON  )  {
      // only play on, return trun;
      return true;
   }

   else if ( mPCM2WayState == PCM2WAY_REC_ON  )  {
      // already rec on, return trun;
      LOG_LAD(AUDMSG"LAD_PCM2WayRecordOff, A2M_PCM_Close1");
      return pCCCI->SendMessageInQueue(CCCI_MSG1(A2M_PCM_Close, 0));
   }

   else if ( mPCM2WayState == (PCM2WAY_REC_ON | PCM2WAY_PLAY_ON) ) {
      // both rec and play on, turn off, modify state and turn on again
      bool ret;
      mPCM2WayState &= (~PCM2WAY_REC_ON);
      LOG_LAD(AUDMSG"LAD_PCM2WayRecordOff, A2M_PCM_Close2");
      ret = pCCCI->SendMessageInQueue(CCCI_MSG1(A2M_PCM_Close, 0));
      LOG_LAD(AUDMSG"LAD_PCM2WayRecordOff, A2M_PCM_OPEN");
      ret &= pCCCI->SendMessageInQueue(CCCI_MSG1(A2M_PCM_OPEN, mPCM2WayState));
      return ret;
   }
   else {
      return false;
   }
}


bool LAD::LAD_PCM2WayOn()
{
   mPCM2WayState = (PCM2WAY_PLAY_ON | PCM2WAY_REC_ON);
   LOG_LAD(AUDMSG"LAD PCM2WayOn (0x%x), mPCM2WayState:%d",A2M_PCM_OPEN,mPCM2WayState);
   return pCCCI->SendMessageInQueue(CCCI_MSG1(A2M_PCM_OPEN, mPCM2WayState));
}

bool LAD::LAD_PCM2WayOff()
{
   mPCM2WayState = 0;
   LOG_LAD(AUDMSG"LAD PCM2WayOff (0x%x), mPCM2WayState:%d",A2M_PCM_Close,mPCM2WayState);
   return pCCCI->SendMessageInQueue(CCCI_MSG1(A2M_PCM_Close, 0));
}


bool LAD::LAD_PCM2WayDataSendBack()
{
//   LOG_LAD("LAD_PCM2WayDataSendBack (Mic), f:0x%x",A2M_PCM_DataSendBack);
   return pCCCI->SendMessageInQueue( CCCI_MSG1(A2M_PCM_DataSendBack, 0));
}


bool LAD::LAD_PCM2WayDataNotify(uint32 offset, uint32 len)
{
//   LOG_LAD("LAD_PCM2WayDataNotify (Spk)");
   return pCCCI->SendMessageInQueue(CCCI_MSG2(A2M_PCM_DataNotify, offset, len));
   return true;
}

#if defined(MTK_DUAL_MIC_SUPPORT)||defined(MTK_AUDIO_HD_REC_SUPPORT)
// ToDo: for dual mic
bool LAD::LAD_DualMic_2WayOn(int WBMode, int PlayRec)
{
   //mPCM2WayState = (PCM2WAY_PLAY_ON | PCM2WAY_REC_ON);
   LOG_LAD("LAD_DualMic_2WayOn (0x%x), type:%d, wbmode:%d, PlayRec:%d",A2M_PCM_OPEN, P2W_FORMAT_CAL, WBMode, PlayRec);

   //prevent 'on' for second time cause problem
   if (mDualMic2WayState) {
      LOGW("LAD_DualMic_2WayOn when record is ongoing: %d, ignore", mDualMic2WayState);
      return TRUE;
   }
   if (PlayRec == DMMETA_2WAY_NORMAL) {
      mDualMic2WayState = DUALMIC2WAY_PLAYREC;
      if (WBMode == FALSE) {
         return pCCCI->SendMessageInQueue(CCCI_MSG1(A2M_DualRecPlay_turn_on, P2W_FORMAT_CAL));
      }
      else {
         return pCCCI->SendMessageInQueue(CCCI_MSG1(A2M_DualRecPlay_turn_on, P2W_FORMAT_WB_CAL));
      }
   }
   else {
      mDualMic2WayState = DUALMIC2WAY_REC_ONLY;
      if (WBMode == FALSE) {
         return pCCCI->SendMessageInQueue(CCCI_MSG1(A2M_DualRecOnly_turn_on, P2W_FORMAT_CAL));
      }
      else {
         return pCCCI->SendMessageInQueue(CCCI_MSG1(A2M_DualRecOnly_turn_on, P2W_FORMAT_WB_CAL));
      }
   }
}
bool LAD::LAD_DualMic_2WayOff()
{
    LOG_LAD("LAD_DualMic_2WayOff, (0x%x) type:%d",A2M_PCM_Close,0);
    if (mDualMic2WayState == DUALMIC2WAY_STOP) {
       LOGW("LAD_DualMic_2WayOff when record is off: %d, ignore", mDualMic2WayState);
       return TRUE;
    }

    mDualMic2WayState = DUALMIC2WAY_STOP;
    if ( mDualMic2WayState == DUALMIC2WAY_PLAYREC ) {
       return pCCCI->SendMessageInQueue(CCCI_MSG1(A2M_DualRecPlay_turn_off, 0));
    }
    else {
       return pCCCI->SendMessageInQueue(CCCI_MSG1(A2M_DualRecOnly_turn_off, 0));
    }
}
#endif

/*-----------------Background Sound Functions---------------------*/

bool LAD::LAD_BGSoundOn()
{
   pCCCI->mHw->SwitchAudioClock(true);
   LOGD(AUDMSG"LAD BGSoundOn, A2M_BGSND_Open(0x%x)",A2M_BGSND_Open);
   return pCCCI->SendMessageInQueue(CCCI_MSG1(A2M_BGSND_Open, 0));
}

bool LAD::LAD_BGSoundOff()
{
   bool ret;
   LOGD(AUDMSG"LAD BGSoundOff, A2M_BGSND_Close(0x%x)",A2M_BGSND_Close);
   ret = pCCCI->SendMessageInQueue(CCCI_MSG1(A2M_BGSND_Close, 0));
   return ret;
}

bool LAD::LAD_BGSoundDataNotify(uint32 offset, uint32 len)
{
//   LOG_LAD("LAD_BGSoundDataNotify, A2M_BGSND_DataNotify offset:%d, len:%d",offset,len);
   return pCCCI->SendMessageInQueue(CCCI_MSG2(A2M_BGSND_DataNotify, offset, len));
}

bool LAD::LAD_BGSoundConfig(uint32 UL_gain, uint32 DL_gain)
{
   LOG_LAD(AUDMSG"LAD_BGSoundConfig, A2M_BGSND_Config ul:%x, dl:%x",UL_gain,DL_gain);
   return pCCCI->SendMessageInQueue(CCCI_MSG3(A2M_BGSND_Config, UL_gain, DL_gain));
}


/*-----------------Playback Functions---------------------*/

bool LAD::LAD_PlaybackTurnOn(int32 data_type)
{
   LOG_LAD(AUDMSG"LAD_PlaybackTurnOn, A2M_playback_turn_on");
   return pCCCI->SendMessageInQueue(CCCI_MSG1(A2M_playback_turn_on, (int32)data_type));
}

bool LAD::LAD_PlaybackTurnOff()
{
   LOG_LAD(AUDMSG"LAD_PlaybackTurnOff, A2M_playback_turn_off");
   return pCCCI->SendMessageInQueue(CCCI_MSG1(A2M_playback_turn_off, 0));
}

bool LAD::LAD_PlaybackDataNotify(uint32 offset, uint32 len)
{
   LOG_LAD(AUDMSG"LAD_PlaybackDataNotify, A2M_Playback_DataNotify");
   return pCCCI->SendMessageInQueue(CCCI_MSG2(A2M_Playback_DataNotify, offset, len));
}


/*------------------Volume Control -----------------------*/

bool LAD::LAD_SetOutputVolume(uint32 Gain)
{
   LOGD(AUDMSG"LAD SetOutputVolume (0x%x), gain:%x",A2M_SetOutputVolume,Gain);
   return pCCCI->SendMessageInQueue( CCCI_MSG1(A2M_SetOutputVolume, Gain) );
}

bool LAD::LAD_SetGainTableOutputVolume(int32 Gain)
{
    LOGD(AUDMSG"LAD_SetGainTableOutputVolume (0x%x), gain:%x",A2M_SetGainTableVolume,Gain);
    return pCCCI->SendMessageInQueue( CCCI_MSG1(A2M_SetGainTableVolume, Gain) );
}

bool LAD::LAD_SetGainTableMicVolume(int32 Gain)
{
    LOGD(AUDMSG"LAD_SetGainTableMicVolume (0x%x), gain:%x",A2M_SetGainTableMicVolume,Gain);
    return pCCCI->SendMessageInQueue( CCCI_MSG1(A2M_SetGainTableMicVolume, Gain) );
}

bool LAD::LAD_SetMicrophoneVolume(uint32 Gain)
{
   if(m_MicGain == Gain)
      return true;

   LOGD(AUDMSG"LAD SetMicrophoneVolume (0x%x), gain:%x",A2M_SetMicrophoneVolume,Gain);
   m_MicGain = Gain;
   return pCCCI->SendMessageInQueue( CCCI_MSG1(A2M_SetMicrophoneVolume, Gain) );
}


bool LAD::LAD_MuteMicrophone(bool ENABLE)
{
   LOGD(AUDMSG"LAD_MuteMicrophone (0x%x), en:%x",A2M_MuteMicrophone,ENABLE);
   return pCCCI->SendMessageInQueue( CCCI_MSG1(A2M_MuteMicrophone, ENABLE) );
}


bool LAD::LAD_MuteSpeaker(bool ENABLE)
{
   LOGD(AUDMSG"LAD_MuteSpeaker (0x%x), en:%x",A2M_MuteSpeaker,ENABLE);
   return pCCCI->SendMessageInQueue( CCCI_MSG1(A2M_MuteSpeaker, ENABLE) );
}


bool LAD::LAD_SetSidetoneVolume(uint32 Gain)
{
   //LOGD(AUDMSG"LAD_SetSidetoneVolume, gain:%x",Gain);
   if(m_SideToneGain == Gain)
      return true;

   LOGD(AUDMSG"LAD_SetSidetoneVolume, gain:%x",Gain);
   m_SideToneGain = Gain;
   return pCCCI->SendMessageInQueue( CCCI_MSG1(A2M_SetSidetoneVolume, Gain) );
}

/*---------------------Device Control--------------------------*/

bool LAD::LAD_SetOutputDevice(LadOutPutDevice_Line HW_Mode)
{
   LOGD(AUDMSG"LAD_SetOutputDevice (0x%x), HW_Mode:%x",A2M_SetOutputDevice,HW_Mode);
   return pCCCI->SendMessageInQueue(  CCCI_MSG1(A2M_SetOutputDevice, HW_Mode));
}

bool LAD::LAD_SetInputSource(LadInPutDevice_Line HW_Mode)
{


#if defined(MTK_DUAL_MIC_SUPPORT)
   //
   // For the dual mic solution, always replace the "Phone Mic" setting as "Dual Mic" setting.
   // For the single mic solution, set the mode as "Phone Mic".
   //
   if(pCCCI->mHw!=NULL)
   {
        if(pCCCI->mHw->mHAL_DualMic_Setting == true) // integration with MMI setting
        {
            if(HW_Mode == LADIN_Microphone1){
                //LOGD(AUDMSG"LAD_SetInputSource, Set LADIN_DualAnalogMic");
                HW_Mode = LADIN_DualAnalogMic;
            }
#if 1//Added by jrd.lipeng for digital dual mic
#if MTK_DIGITAL_MIC_SUPPORT
            if(HW_Mode == LADIN_SingleDigitalMic){
                HW_Mode = LADIN_DualDigitalMic;
            }
#endif
#endif

        }
    }
#endif

#if MTK_DIGITAL_MIC_SUPPORT
   if(HW_Mode == LADIN_Microphone1)
   {
      HW_Mode = LADIN_SingleDigitalMic;
   }
   else if(HW_Mode == LADIN_DualAnalogMic)
   {
      HW_Mode = LADIN_DualDigitalMic;
   }
#endif


   if(m_InputSrc == (int)HW_Mode)
      return true;

   m_InputSrc = (int)HW_Mode;
   LOGD(AUDMSG"LAD_SetInputSource (0x%x), HW_Mode(%x)(%x)",A2M_SetInputSource,HW_Mode,m_InputSrc);
   return pCCCI->SendMessageInQueue(  CCCI_MSG1(A2M_SetInputSource, HW_Mode));
}

/*-------------------Speech Control----------------------------*/
bool LAD::LAD_SetSpeechMode(ENUM_Speech_Mode Speech_Mode)
{
   m_SideToneGain = 0;
   m_MicGain = 0;
   m_InputSrc = (int)0;
   LOGD(AUDMSG"LAD SetSpeechMode (0x%x), HW_Mode:%x",A2M_SetSpeechMode,Speech_Mode);
   return pCCCI->SendMessageInQueue(CCCI_MSG1(A2M_SetSpeechMode, Speech_Mode));
}


bool LAD::LAD_Speech_On(int sp_mode)
{

   pCCCI->mHw->SwitchAudioClock(true);

   int mode=0;
   LOG_LAD(AUDMSG"LAD Speech_On (0x%x), sp_mode:%x",A2M_Speech_On,sp_mode);

// In l1sp.c, the phone call status define as follow.
// #define RAT_2G_MODE  0
// #define RAT_3G_MODE  1
// #define RAT_3G324M_MODE 2 --> VT
// For the VT phone call, AP side must ask the modem side l1audio drvier that this is VT phone call.

   // sp_mode=2 --> this is VT phone call
   if(sp_mode == 2){
   // for VT phone call. Set mode as 2. AP side must info modem side that it is VT call
      mode=2;
   }
   else{
   // for 2G/3G phone call. Set mode as 1.  Modem side can query 2G/3G phone call.
      mode=0;
   }
   return pCCCI->SendMessageInQueue(CCCI_MSG1(A2M_Speech_On, mode));  // Stan,  0:2G, 1:3G

}

bool LAD::LAD_Speech_Off()
{
   m_SideToneGain = 0;
   m_MicGain = 0;
   m_InputSrc = (int)0;
   LOG_LAD(AUDMSG"LAD Speech_Off (0x%x) ",A2M_Speech_Off);
   return pCCCI->SendMessageInQueue(CCCI_MSG1(A2M_Speech_Off, 0));
}

int LAD::LAD_Set_Speech_Enhancement(bool bEnable)
{
   if(mSph_Enh_enable == bEnable){
      LOG_LAD(AUDMSG"LAD_Set_Speech_Enh, mSph_Enh_enable(%d)=bEnable(%d) return",mSph_Enh_enable,bEnable);
      return true;
   }

   mSph_Enh_enable = bEnable;
   LOG_LAD(AUDMSG"LAD_Set_Speech_Enh (0x%x),(%d) ",A2M_SetSphEnhancementAll,bEnable);
   int ret = 0;
   ret = pCCCI->SendMessageInQueue(CCCI_MSG1(A2M_SetSphEnhancementAll, bEnable));
   return ret;
}

#if defined(MTK_SPH_EHN_CTRL_SUPPORT)
/// Set specific enhancement configuration
int LAD::LAD_Set_Speech_Enhancement_Info(const SPH_ENH_INFO_T &eSphEnhInfo)
{
    bool retval = true;

    uint32 param;

    /// (Ctrl 2) set spe_usr_mask to decide each enhancement function to be on/off
    param = (eSphEnhInfo.spe_usr_mask << 4) | SPH_ENH_USR_CFG1;
    LOG_LAD(AUDMSG"LAD_Set_Speech_Enhancement_Info (0x%x),(0x%x) ",A2M_SetSphEnhancementSpecific, param);
    retval &= pCCCI->SendMessageInQueue(CCCI_MSG1(A2M_SetSphEnhancementSpecific, param));

    /// (Ctrl 3) set spe_usr_subFunc_mask[0:15] to decide each enhancement function to be on/off
    param = ((eSphEnhInfo.spe_usr_subFunc_mask & 0xFFFF) << 4) | SPH_ENH_USR_CFG2_1;
    LOG_LAD(AUDMSG"LAD_Set_Speech_Enhancement_Info (0x%x),(0x%x) ",A2M_SetSphEnhancementSpecific, param);
    retval &= pCCCI->SendMessageInQueue(CCCI_MSG1(A2M_SetSphEnhancementSpecific, param));

    /// (Ctrl 4) set spe_usr_subFunc_mask[16:31] to decide each enhancement function to be on/off
    param = ((eSphEnhInfo.spe_usr_subFunc_mask >> 16) << 4) | SPH_ENH_USR_CFG2_2;
    LOG_LAD(AUDMSG"LAD_Set_Speech_Enhancement_Info (0x%x),(0x%x) ",A2M_SetSphEnhancementSpecific, param);
    retval &= pCCCI->SendMessageInQueue(CCCI_MSG1(A2M_SetSphEnhancementSpecific, param));

    /// (Ctrl 5) after ENUM_SPH_ENH_CTRL={2,3,4} is done, apply these configure
    LOG_LAD(AUDMSG"LAD_Set_Speech_Enhancement_Info (0x%x),(0x%x) ",A2M_SetSphEnhancementSpecific, SPH_ENH_APPLY);
    retval &= pCCCI->SendMessageInQueue(CCCI_MSG1(A2M_SetSphEnhancementSpecific, SPH_ENH_APPLY));

    /// update new config
    mSphEnhInfo = eSphEnhInfo;

    FILE *f_sph_enh = fopen(SPH_ENH_INFO_FILENAME, "wb");
    if(f_sph_enh == NULL)
    {
        LOGE("cannot open VCE file %s in LAD_Set_Speech_Enhancement_Info", SPH_ENH_INFO_FILENAME);
    }
    else
    {
        chmod(SPH_ENH_INFO_FILENAME, 0644);
        fwrite(&mSphEnhInfo, sizeof(SPH_ENH_INFO_T), 1, f_sph_enh);
        fclose(f_sph_enh);
    }

    return retval;
}

int LAD::LAD_Set_Speech_Enhancement_MainFunction(uint32 MainConfig)
{
    int ret  = true;
    /// (Ctrl 2) set spe_usr_mask to decide each enhancement function to be on/off
    MainConfig= (MainConfig << 4) | SPH_ENH_USR_CFG1;
    LOG_LAD(AUDMSG"LAD_Set_Speech_Enhancement_MainFunction (0x%x),(0x%x) ",A2M_SetSphEnhancementSpecific, MainConfig);
    ret &= pCCCI->SendMessageInQueue(CCCI_MSG1(A2M_SetSphEnhancementSpecific, MainConfig));
    return ret;
}


int LAD::LAD_Set_Speech_Enhancement_SubFunctionHiBits(uint32 SubConfig)
{
    int ret  = true;
    /// (Ctrl 4) set spe_usr_subFunc_mask[16:31] to decide each enhancement function to be on/off
    SubConfig = ((SubConfig >> 16) << 4) | SPH_ENH_USR_CFG2_2;
    LOG_LAD(AUDMSG"LAD_Set_Speech_Enhancement_Info (0x%x),(0x%x) ",A2M_SetSphEnhancementSpecific, SubConfig);
    ret &= pCCCI->SendMessageInQueue(CCCI_MSG1(A2M_SetSphEnhancementSpecific, SubConfig));
    return ret;
}


int LAD::LAD_Set_Speech_Enhancement_SubFunctionLowBits(uint32 SubConfig)
{
    int ret  = true;
    /// (Ctrl 3) set spe_usr_subFunc_mask[0:15] to decide each enhancement function to be on/off
    SubConfig = ((SubConfig & 0xFFFF) << 4) | SPH_ENH_USR_CFG2_1;
    LOG_LAD(AUDMSG"LAD_Set_Speech_Enhancement_Info (0x%x),(0x%x) ",A2M_SetSphEnhancementSpecific, SubConfig);
    ret &= pCCCI->SendMessageInQueue(CCCI_MSG1(A2M_SetSphEnhancementSpecific, SubConfig));
    return ret;
}

int LAD::LAD_Set_Speech_Enhancement_FunctionEnable( )
{
    int ret=0;
    /// (Ctrl 5) after ENUM_SPH_ENH_CTRL={2,3,4} is done, apply these configure
    LOG_LAD(AUDMSG"LAD_Set_Speech_Enhancement_Info (0x%x),(0x%x) ",A2M_SetSphEnhancementSpecific, SPH_ENH_APPLY);
    ret &= pCCCI->SendMessageInQueue(CCCI_MSG1(A2M_SetSphEnhancementSpecific, SPH_ENH_APPLY));
    return ret;
}

#endif //MTK_SPH_EHN_CTRL_SUPPORT


bool LAD::LAD_SetAfeLoopback(bool bLoopbackOn, bool bSwapADDA, bool bConnectAllADDA)
{
    uint8 param = (bConnectAllADDA << 2) | (bSwapADDA << 1) | bLoopbackOn;
    // 0: afe loopback (ADC_1 => DAC_1,   ADC_2 => DAC_2)   off
    // 1: afe loopback (ADC_1 => DAC_1,   ADC_2 => DAC_2)   on

    // 2: afe loopback (ADC_1 => DAC_2,   ADC_2 => DAC_1)   off
    // 3: afe loopback (ADC_1 => DAC_2,   ADC_2 => DAC_1)   on

    // 4: afe loopback (ADC_1 => DAC_1+2, ADC_2 => DAC_1+2) off
    // 5: afe loopback (ADC_1 => DAC_1+2, ADC_2 => DAC_1+2) on

    LOG_LAD(AUDMSG"LAD_SetAfeLoopback, A2M_SetAfeLoopback param:%d", param);
    return pCCCI->SendMessageInQueue(CCCI_MSG1(A2M_SetAfeLoopback, param));
}


bool LAD::LAD_SetAcousticLoopback(bool bLoopbackOn, bool bUseDMNR)
{
    uint8 param = (bUseDMNR << 1) | bLoopbackOn;
    // 0: acoustic loopback (without DMNR) off
    // 1: acoustic loopback (without DMNR) on
    // 2: acoustic loopback (with    DMNR) off
    // 3: acoustic loopback (with    DMNR) on

    LOG_LAD(AUDMSG"LAD_SetAcousticLoopback, A2M_SetAcousticLoopback param:%d", param);
    return pCCCI->SendMessageInQueue(CCCI_MSG1(A2M_SetAcousticLoopback, param));
}


bool LAD::LAD_SetReceiverTest(bool turn_ON)
{
   LOG_LAD(AUDMSG"LAD_SetReceiverTest, turn_ON:%x",turn_ON);
   return pCCCI->SendMessage(  CCCI_MSG1(A2M_SetReceiverTest, turn_ON));
}

bool LAD::LAD_Set_DAI_Mode(bool turn_ON)
{
   LOG_LAD(AUDMSG"LAD_Set_DAI_Mode, turn_ON:%x",turn_ON);
   return pCCCI->SendMessage(  CCCI_MSG1(A2M_Set_DAI_Mode, turn_ON));
}

bool LAD::LAD_SwitchVCM(int32 TurnOn)
{
   LOGD(AUDMSG"LAD_SwitchVCM, TurnOn:%x",TurnOn);
   return pCCCI->SendMessage(CCCI_MSG1(A2A_SetVCM, TurnOn));
}

bool LAD::LAD_Set_LoudSPK_RefMicMode(bool TurnOn)
{
   LOGD(AUDMSG"LAD_Set_LoudSPK_RefMicMode, TurnOn:%x",TurnOn);
   return pCCCI->SendMessage(CCCI_MSG1(A2M_SetLoudSPK_RefMicMode, TurnOn));
}

/*-----------------Proprietary Control-----------------------*/
bool LAD::LAD_SwitchMicBias(int32 TurnOn)
{
   LOGD(AUDMSG"LAD_SwitchMicBias (0x%x), TurnOn:%d",A2M_SetMicBias,TurnOn);
   return pCCCI->SendMessageInQueue(CCCI_MSG1(A2M_SetMicBias, TurnOn));
}


/*-----------------Set Speech Parameters-----------------------*/
int LAD::LAD_SetSpeechParameters(AUDIO_CUSTOM_PARAM_STRUCT* pParam)
{
   int32 offset = (pCCCI->GetM2AShareBufLen()>=4096)?SPH_PAR_A2M_SHARED_BUFFER_OFFSET:0;
   int32 len = LAD_SHARE_HEADER_LEN + sizeof(AUDIO_CUSTOM_PARAM_STRUCT);
   int8 *pShareBuf = (int8 *)pCCCI->GetA2MShareBufAddress() + offset;
   //get buffer lock to prevent overwrite other's data
   if(pCCCI->A2MBufLock() == true){
       // write header
       WriteShareBufHeader(pShareBuf, (int16)LADBUFID_CCCI_EM_PARAM, (int16)sizeof(AUDIO_CUSTOM_PARAM_STRUCT), (int16)A2M_BUF_HEADER);

       // increment for the header
       pShareBuf += LAD_SHARE_HEADER_LEN;

       // write parameters
       memcpy((void*)pShareBuf, (void*)pParam, sizeof(AUDIO_CUSTOM_PARAM_STRUCT));

       // send message to modem
       LOGD(AUDMSG"LAD_SetSpeechParameters (0x%x)",AUDIO_CUSTOM_PARAMETER);
       int32 ret = pCCCI->SendMessageInQueue(CCCI_MSG2(AUDIO_CUSTOM_PARAMETER, offset, len));
       if(ret<=0)
           LOGE(AUDMSG"LAD_SetSpeechParameters, Send CCCI Msg failed!\n");
       return ret;
   }
   else{
       LOGE(AUDMSG"LAD_SetSpeechParameters fail due to unalbe get A2M Buffer\n");
       return false;
   }
}

#if defined(MTK_DUAL_MIC_SUPPORT)
int LAD::LAD_SetDualMicParameters(AUDIO_CUSTOM_EXTRA_PARAM_STRUCT* pParam)
{
// NVRAM always contain (44+76), for WB we send (44+76), for NB we send (44)
// define the data length that actually sent to modem base on if WB is supported
#if defined(MTK_WB_SPEECH_SUPPORT)
   int32 sendDataLen = (NUM_ABF_PARAM+NUM_ABFWB_PARAM) * sizeof(unsigned short);
#else
   int32 sendDataLen = NUM_ABF_PARAM * sizeof(unsigned short);
#endif

   int32 offset = (pCCCI->GetM2AShareBufLen()>=4096)? SPH_PAR_A2M_SHARED_BUFFER_OFFSET : 0;;
   int32 len = LAD_SHARE_HEADER_LEN + sendDataLen;
   int8 *pShareBuf = (int8 *)pCCCI->GetA2MShareBufAddress() + offset;
   if(pCCCI->A2MBufLock() == true){
       // write header
       WriteShareBufHeader(pShareBuf, (int16)LADBUFID_CCCI_EM_PARAM, (int16)sendDataLen, (int16)A2M_BUF_HEADER);

       // increment for the header
       pShareBuf += LAD_SHARE_HEADER_LEN;

       // write parameters
       memcpy((void*)pShareBuf, (void*)pParam, sendDataLen);

       {
          unsigned short *ptr;
          int i = 0;
          ptr = (unsigned short*)pShareBuf;
          i = 0;
          LOGD(AUDMSG"%d, %d, %d, %d, %d, %d, %d, %d, %d, %d", ptr[i+0], ptr[i+1], ptr[i+2], ptr[i+3], ptr[i+4], ptr[i+5], ptr[i+6], ptr[i+7], ptr[i+8], ptr[i+9]);
          i = 10;
          LOGD(AUDMSG"%d, %d, %d, %d, %d, %d, %d, %d, %d, %d", ptr[i+0], ptr[i+1], ptr[i+2], ptr[i+3], ptr[i+4], ptr[i+5], ptr[i+6], ptr[i+7], ptr[i+8], ptr[i+9]);
          i = 20;
          LOGD(AUDMSG"%d, %d, %d, %d, %d, %d, %d, %d, %d, %d", ptr[i+0], ptr[i+1], ptr[i+2], ptr[i+3], ptr[i+4], ptr[i+5], ptr[i+6], ptr[i+7], ptr[i+8], ptr[i+9]);
          i = 30;
          LOGD(AUDMSG"%d, %d, %d, %d, %d, %d, %d, %d, %d, %d", ptr[i+0], ptr[i+1], ptr[i+2], ptr[i+3], ptr[i+4], ptr[i+5], ptr[i+6], ptr[i+7], ptr[i+8], ptr[i+9]);
          i = 40;
          LOGD(AUDMSG"%d, %d, %d, %d, %d, %d, %d, %d, %d, %d", ptr[i+0], ptr[i+1], ptr[i+2], ptr[i+3], ptr[i+4], ptr[i+5], ptr[i+6], ptr[i+7], ptr[i+8], ptr[i+9]);
          i = 50;
          LOGD(AUDMSG"%d, %d, %d, %d, %d, %d, %d, %d, %d, %d", ptr[i+0], ptr[i+1], ptr[i+2], ptr[i+3], ptr[i+4], ptr[i+5], ptr[i+6], ptr[i+7], ptr[i+8], ptr[i+9]);
          i = 60;
          LOGD(AUDMSG"%d, %d, %d, %d, %d, %d, %d, %d, %d, %d", ptr[i+0], ptr[i+1], ptr[i+2], ptr[i+3], ptr[i+4], ptr[i+5], ptr[i+6], ptr[i+7], ptr[i+8], ptr[i+9]);
          i = 70;
          LOGD(AUDMSG"%d, %d, %d, %d, %d, %d, %d, %d, %d, %d", ptr[i+0], ptr[i+1], ptr[i+2], ptr[i+3], ptr[i+4], ptr[i+5], ptr[i+6], ptr[i+7], ptr[i+8], ptr[i+9]);
          i = 80;
          LOGD(AUDMSG"%d, %d, %d, %d, %d, %d, %d, %d, %d, %d", ptr[i+0], ptr[i+1], ptr[i+2], ptr[i+3], ptr[i+4], ptr[i+5], ptr[i+6], ptr[i+7], ptr[i+8], ptr[i+9]);
          i = 90;
          LOGD(AUDMSG"%d, %d, %d, %d, %d, %d, %d, %d, %d, %d", ptr[i+0], ptr[i+1], ptr[i+2], ptr[i+3], ptr[i+4], ptr[i+5], ptr[i+6], ptr[i+7], ptr[i+8], ptr[i+9]);
          i = 100;
          LOGD(AUDMSG"%d, %d, %d, %d, %d, %d, %d, %d, %d, %d", ptr[i+0], ptr[i+1], ptr[i+2], ptr[i+3], ptr[i+4], ptr[i+5], ptr[i+6], ptr[i+7], ptr[i+8], ptr[i+9]);
          i = 110;
          LOGD(AUDMSG"%d, %d, %d, %d, %d, %d, %d, %d, %d, %d", ptr[i+0], ptr[i+1], ptr[i+2], ptr[i+3], ptr[i+4], ptr[i+5], ptr[i+6], ptr[i+7], ptr[i+8], ptr[i+9]);
       }


       // send message to modem
       LOGD(AUDMSG"LAD_SetDualMicParameters (0x%x), number of para(%d)",AUDIO_DUAL_MIC_CUSTOM_PARAMETER, sendDataLen);
       int32 ret = pCCCI->SendMessageInQueue(CCCI_MSG2(AUDIO_DUAL_MIC_CUSTOM_PARAMETER, offset, len));
       if(ret<=0)
           LOGE(AUDMSG"LAD_SetDualMicParameters, Send CCCI Msg failed!\n");
       return ret;
   }
   else{
       LOGE(AUDMSG"LAD_SetDualMicParameters fail due to unalbe get A2M Buffer\n");
       return false;
   }
}
#endif

#if defined(MTK_WB_SPEECH_SUPPORT)
/*-----------------Set WB Speech Parameters-----------------------*/
int LAD::LAD_SetWBSpeechParameters(AUDIO_CUSTOM_WB_PARAM_STRUCT* pWBParam)
{
   int   ret = true;
   int32 offset = (pCCCI->GetM2AShareBufLen()>=4096)? SPH_PAR_A2M_SHARED_BUFFER_OFFSET : 0;;
   int32 len = LAD_SHARE_HEADER_LEN + sizeof(AUDIO_CUSTOM_WB_A2M_PARAM_STRUCT);
   int8 *pShareBuf = (int8 *)pCCCI->GetA2MShareBufAddress() + offset;
   AUDIO_CUSTOM_WB_A2M_PARAM_STRUCT *pA2M_Cust_Wb_Msg;
   //get buffer lock to prevent overwrite other's data
   if(pCCCI->A2MBufLock() == true){
       // write header
       WriteShareBufHeader(pShareBuf, (int16)LADBUFID_CCCI_EM_PARAM, (int16)sizeof(AUDIO_CUSTOM_WB_A2M_PARAM_STRUCT), (int16)A2M_BUF_HEADER);

       // increment for the header
       pShareBuf += LAD_SHARE_HEADER_LEN;

       pA2M_Cust_Wb_Msg = (AUDIO_CUSTOM_WB_A2M_PARAM_STRUCT *)pShareBuf;

       // write input fir parameters
       memcpy((void*)pA2M_Cust_Wb_Msg->speech_mode_wb_para, (void*)pWBParam->speech_mode_wb_para, sizeof(pWBParam->speech_mode_wb_para));
       memcpy((void*)pA2M_Cust_Wb_Msg->sph_wb_fir, (void*)pWBParam->sph_wb_in_fir, sizeof(pWBParam->sph_wb_in_fir));
       pA2M_Cust_Wb_Msg->input_out_fir_flag= 0;

       // send message to modem
       LOGD(AUDMSG"LAD_SetWBSpeechParameters(0x%x) + input FIR",AUDIO_WB_CUSTOM_PARAMETER);
       ret = pCCCI->SendMessageInQueue(CCCI_MSG2(AUDIO_WB_CUSTOM_PARAMETER, offset, len));
       if(ret<=0)
           LOGE(AUDMSG"LAD_SetWBSpeechParameters, Send CCCI Msg failed!\n");

       //get buffer lock to prevent overwrite other's data
       if(pCCCI->A2MBufLock() == true){
           // (ALPS00274550) update header info to avoid other messages overwrite it after WB input FIR is done
           pShareBuf = (int8 *)pCCCI->GetA2MShareBufAddress() + offset;
           WriteShareBufHeader(pShareBuf, (int16)LADBUFID_CCCI_EM_PARAM, (int16)sizeof(AUDIO_CUSTOM_WB_A2M_PARAM_STRUCT), (int16)A2M_BUF_HEADER);

           // write output fir parameters
           pA2M_Cust_Wb_Msg = (AUDIO_CUSTOM_WB_A2M_PARAM_STRUCT *)(pShareBuf + LAD_SHARE_HEADER_LEN);
           memcpy((void*)pA2M_Cust_Wb_Msg->speech_mode_wb_para, (void*)pWBParam->speech_mode_wb_para, sizeof(pWBParam->speech_mode_wb_para));
           memcpy((void*)pA2M_Cust_Wb_Msg->sph_wb_fir, (void*)pWBParam->sph_wb_out_fir, sizeof(pWBParam->sph_wb_out_fir));
           pA2M_Cust_Wb_Msg->input_out_fir_flag= 1;
           LOGD(AUDMSG"LAD_SetWBSpeechParameters(0x%x) + output FIR",AUDIO_WB_CUSTOM_PARAMETER);
           ret &= pCCCI->SendMessageInQueue(CCCI_MSG2(AUDIO_WB_CUSTOM_PARAMETER, offset, len));
           if(ret<=0)
               LOGE(AUDMSG"LAD_SetWBSpeechParameters, Send CCCI Msg failed!\n");
       }
       else{
           LOGE(AUDMSG"LAD_SetWBSpeechParameters fail due to unalbe get A2M Buffer #2\n");
           ret = false;
       }
   }
   else{
       LOGE(AUDMSG"LAD_SetWBSpeechParameters fail due to unalbe get A2M Buffer #1\n");
       ret = false;
   }

   return ret;
}
#endif

/*-----------------Set HD Record Parameters-----------------------*/
#if defined(MTK_AUDIO_HD_REC_SUPPORT)
int LAD::LAD_SetHdRecordParameters(AUDIO_HD_RECORD_PARAM_STRUCT* pParam)
{
    const int32 len_hd_record_param = CFG_FILE_AUDIO_HD_REC_PAR_SIZE;

    const int32 offset = (pCCCI->GetM2AShareBufLen() >= 4096) ? SPH_PAR_A2M_SHARED_BUFFER_OFFSET : 0;
    const int32 len = LAD_SHARE_HEADER_LEN + len_hd_record_param;

    int8 *pShareBuf = (int8 *)pCCCI->GetA2MShareBufAddress() + offset;

    // get buffer lock to prevent overwrite other's data
    if(pCCCI->A2MBufLock() == false){
       LOGE(AUDMSG"LAD_SetHdRecordParameters fail due to unalbe get A2M Buffer\n");
       return false;
    }
    else{
        // write header
        WriteShareBufHeader(pShareBuf, LADBUFID_CCCI_HD_RECORD, (int16)len_hd_record_param, (int16)A2M_BUF_HEADER);

        // increment for the header
        pShareBuf += LAD_SHARE_HEADER_LEN;

        // write hd record parameters
        memcpy((void*)pShareBuf, (void*)pParam, len_hd_record_param);

#if 0 // Debug Info
        LOG_LAD("-LAD_SetHdRecordParameters, hd_rec_mode_num=%u", pParam->hd_rec_mode_num);

        LOG_LAD("-LAD_SetHdRecordParameters, hd_rec_fir_num=%u", pParam->hd_rec_fir_num);

        for(int i=0;i<pParam->hd_rec_mode_num;i++)
            for(int j=0;j<16;j++)
                LOG_LAD("-LAD_SetHdRecordParameters, hd_rec_speech_mode_para[%d][%d]=%u", i,j,pParam->hd_rec_speech_mode_para[i][j]);

        for(int i=0;i<pParam->hd_rec_fir_num;i++)
            for(int j=0;j<90;j++)
                LOG_LAD("-LAD_SetHdRecordParameters, hd_rec_fir[%d][%d]=%u", i,j,pParam->hd_rec_fir[i][j]);

        for(int i=0;i<pParam->hd_rec_mode_num;i++)
            LOG_LAD("-LAD_SetHdRecordParameters, hd_rec_map_to_fir_for_ch1[%d]=%u", i,pParam->hd_rec_map_to_fir_for_ch1[i]);

        for(int i=0;i<pParam->hd_rec_mode_num;i++)
            LOG_LAD("-LAD_SetHdRecordParameters, hd_rec_map_to_fir_for_ch2[%d]=%u", i,pParam->hd_rec_map_to_fir_for_ch2[i]);

        for(int i=0;i<pParam->hd_rec_mode_num;i++)
            LOG_LAD("-LAD_SetHdRecordParameters, hd_rec_map_to_dev_mode[%d]=%u", i,pParam->hd_rec_map_to_dev_mode[i]);

        for(int i=0;i<pParam->hd_rec_mode_num;i++)
            LOG_LAD("-LAD_SetHdRecordParameters, hd_rec_map_to_input_src[%d]=%u", i,pParam->hd_rec_map_to_input_src[i]);

        for(int i=0;i<pParam->hd_rec_mode_num;i++)
            LOG_LAD("-LAD_SetHdRecordParameters, hd_rec_map_to_stereo_flag[%d]=%u", i,pParam->hd_rec_map_to_stereo_flag[i]);

        uint16 *dbg_ptr = (uint16 *) pShareBuf;
        LOG_LAD("-LAD_SetHdRecordParameters, [pShareBuf]hd_rec_mode_num=%u", dbg_ptr[0]);
        LOG_LAD("-LAD_SetHdRecordParameters, [pShareBuf]hd_rec_fir_num=%u", dbg_ptr[1]);
        LOG_LAD("-LAD_SetHdRecordParameters, [pShareBuf]hd_rec_speech_mode_para[0][0] = %u", dbg_ptr[2]);
#endif

        // send message to modem
        LOGD(AUDMSG"LAD_SetHdRecordParameters(0x%x), size of param: %d", AUDIO_HD_RECORD_PARAMETER, len_hd_record_param);
        return pCCCI->SendMessageInQueue(CCCI_MSG2(AUDIO_HD_RECORD_PARAMETER, offset, len));
    }
}
#endif

int LAD::LAD_GetMDCapability(void)
{
    int ret = true;
    ret = pCCCI->SendMessageInQueue(CCCI_MSG1(A2M_GetMDCapability, 0));
    return ret;
}

int LAD::LAD_SetMDCapability(int32 value)
{
    int ret = true;
    ret = pCCCI->SendMessageInQueue(CCCI_MSG1(A2M_SetMDCapability, value));
    return ret;
}

int LAD::LAD_LogMDSphCoeff(void)
{
    int vflag = 0;
    int ret   = true;
    char value[PROPERTY_VALUE_MAX];
    //get this property to dump speech log
    property_get("md.speechcoef.dump", value, "0");
    vflag=atoi(value);
    if (vflag){
        LOGD(AUDMSG"LAD_LogMDSphCoeff(0x%x)\n", A2M_LogMDSphCoeff);
        ret = pCCCI->SendMessageInQueue(CCCI_MSG1(A2M_LogMDSphCoeff, 0));
    }
    return ret;
}
int32 WriteShareBufHeader(int8 *ptr, int16 type, int16 len, int16 sync )
{
   int16 *ptr16 = (int16*)ptr;
   ptr16[0] = (int16)sync;
   ptr16[1] = (int16)type;
   ptr16[2] = (int16)len;
   return LAD_SHARE_HEADER_LEN;
}


/*----------------- Set TTY-CTM -----------------------*/
bool LAD::LAD_TtyCtmOn(int tty_mode)
{
   LOGD(AUDMSG"LAD_TtyCtmOn, tty_mode:%d",tty_mode);

//   typedef enum{
//      DIRECT_MODE,
//      BAUDOT_MODE
//   } L1Ctm_Interface;

   bool ret = pCCCI->SendMessageInQueue(CCCI_MSG1(A2M_CTM_Open, 1));  // 0: Direct Mode. 1: Baudot Mode
   mTty_used_in_phone_call = true;
   return ret;
}

bool LAD::LAD_TtyCtmOff()
{
   LOGD(AUDMSG"LAD_TtyCtmOff");
   bool ret = pCCCI->SendMessageInQueue(CCCI_MSG1(A2M_CTM_Close, 0));
   mTty_used_in_phone_call = false;
   return ret;
}

}; // namespace android
