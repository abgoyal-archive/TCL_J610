



#define LOG_TAG  "AudioPcm2way"

//#define ENABLE_LOG_PCM2WAY
#ifdef ENABLE_LOG_PCM2WAY
#undef LOGV
#define LOGV(...) LOGD(__VA_ARGS__)
#endif

#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sched.h>
#include <fcntl.h>
#include <time.h>
#include <sys/ioctl.h>
#include <utils/Log.h>
#include <utils/String8.h>

#include "AudioYusuHardware.h"
#include "AudioYusuDef.h"
#include "AudioYusuLad.h"
#include "AudioYusuStreamHandler.h"
#include "AudioYusuLadPlayer.h"
#include "AudioYusuCcci.h"
#include "AudioPcm2way.h"

#define PCM2WAY_BUFFER_SIZE    2048
#define PCM2WAY_HEADER_LENGTH  6

#define AUDIO_INPUT_BUFFER_SIZE   (16 * 1024)
#define AUDIO_OUTPUT_BUFFER_SIZE  (16 * 1024)

   // for VT + BG Sound case (A2M Shared buffer usage)
   // BG Sound use address 0~1408(BGS use 1408 bytes). PCM4WAY_play use address 1408~2048. (4WAY playback path: (640)320+320 bytes)
   // But for the better solution, A2M shared buffer should use ring buffer.
#define A2M_SHARED_BUFFER_OFFSET  (1408)

static long long getTimeMs()
{
   struct timeval t1;
   long long ms;

   gettimeofday(&t1, NULL);
   ms = t1.tv_sec * 1000LL + t1.tv_usec / 1000;

   return ms;
}

namespace android {



Play2Way::Play2Way(AudioYusuHardware *hw)
{
   mHw = hw;
   LOGD("Play2Way Constructor hw: %p \n", mHw);

   pCCCI = mHw->pLad->pCCCI;
   pLad = mHw->pLad;

   memset((void*)&m_OutputBuf,0,sizeof(rb));

   pthread_mutex_init(&pPlay2Way_Mutex, NULL);

   // Internal Output Buffer Initialization
   m_OutputBuf.pBufBase = new char[AUDIO_OUTPUT_BUFFER_SIZE];
   m_OutputBuf.bufLen   = AUDIO_OUTPUT_BUFFER_SIZE;
   m_OutputBuf.pRead    = m_OutputBuf.pBufBase;
   m_OutputBuf.pWrite   = m_OutputBuf.pBufBase;

   memset(m_OutputBuf.pBufBase,0,m_OutputBuf.bufLen);

   // get share buffer base pointer
   pA2M_SharedBufPtr = (int8 *)pLad->pCCCI->GetA2MShareBufAddress();

   m_Play2Way_Started = false;
}

Play2Way::~Play2Way()
{

   if(pLad != NULL){
      int Queue_count = pLad->pCCCI->GetQueueCount();
      LOGD("~Play2Way, Queue_count= %d",Queue_count);
      pLad->pCCCI->Record2WayLock();
      Play2Way_BufLock();
   }

   if(m_OutputBuf.pBufBase != NULL){
      delete []m_OutputBuf.pBufBase;
      m_OutputBuf.pBufBase = NULL;
      m_OutputBuf.bufLen   = 0;
      m_OutputBuf.pRead    = NULL;
      m_OutputBuf.pWrite   = NULL;
   }

   if(pLad != NULL){
      Play2Way_BufUnlock();
      pLad->pCCCI->Record2WayUnLock();
   }

}

void Play2Way::Play2Way_BufLock()
{
   pthread_mutex_lock(&pPlay2Way_Mutex);
}

void Play2Way::Play2Way_BufUnlock()
{
   pthread_mutex_unlock(&pPlay2Way_Mutex);
}

int Play2Way::Play2Way_Start(int sample_rate)
{
   LOGD("Play2Way_Start \n");
   int ret;
   Play2Way_BufLock();

   m_Play2Way_Started = true;
   // Reset read and write pointer of Output buffer
   m_OutputBuf.bufLen   = AUDIO_OUTPUT_BUFFER_SIZE;
   m_OutputBuf.pRead    = m_OutputBuf.pBufBase;
   m_OutputBuf.pWrite   = m_OutputBuf.pBufBase;

// for UT
   // Set Input/output devices
   //pLad->LAD_MuteMicrophone(false);
   //pLad->LAD_SetOutputDevice(LADOUT_SPEAKER1);
   //pLad->LAD_SetInputSource(LADIN_Microphone1);
   //pLad->LAD_SetSpeechMode(SPH_MODE_NORMAL);
   //mHw->mVolumeController->SetLadMicGain(Normal_Mic);

   // Start PCM2WAY GetFromMicrophone
//   LOGD("Play2Way_Start LAD_PCM2WayOn \n");
//   ret =  pLad->LAD_PCM2WayOn();

   Play2Way_BufUnlock();
   return true;
}

int Play2Way::Play2Way_Stop()
{
   LOGD("Play2Way_Stop \n");
   int ret;
   Play2Way_BufLock();

   m_Play2Way_Started = false;
   // Stop PCM2WAY GetFromMicrophone
//   LOGD("Play2Way_Stop LAD_PCM2WayOff \n");
//   ret =  pLad->LAD_PCM2WayOff();

   Play2Way_BufUnlock();
   return true;
}

int Play2Way::Play2Way_Write(void *buffer, int size_bytes)
{
   int ret = 0;
   int mStreamMode;

   Play2Way_BufLock();

   mHw->getMode(&mStreamMode);
   if(mStreamMode == android_audio_legacy::AudioSystem::MODE_IN_CALL)  // VT phone call
   {
      if(m_Play2Way_Started == true)
      {
         rb_copyFromLinear(&m_OutputBuf,(char *)buffer,size_bytes);
         // You can write to AP side audio kernel driver. Playback via AP side.
         // bytes = ::write(mFd, buffer, bytes);
      }
      else
      {
         LOGD("Play2Way_Write, MODE_IN_CALL, not start");
         memset((void*)buffer,0,size_bytes);
         rb_copyFromLinear(&m_OutputBuf,(char *)buffer,size_bytes);
      }
   }
   else
   {
      LOGD("Play2Way_Write, not in MODE_IN_CALL");
      if(m_Play2Way_Started == true)
      {
         rb_copyFromLinear(&m_OutputBuf,(char *)buffer,size_bytes);
         // You can write to AP side audio kernel driver. Playback via AP side.
         // bytes = ::write(mFd, buffer, bytes);
      }
   }

   Play2Way_BufUnlock();
   return size_bytes;
}

int Play2Way::Play2Way_GetFreeBufferCount()
{
   int freeSpaceInpBuf=0;
   //get free space of internal buffer
   freeSpaceInpBuf = m_OutputBuf.bufLen - rb_getDataCount(&m_OutputBuf);
   LOGV("Play2Way_GetFreeBufCnt buf_cnt:%d, free_cnt:%d \n",rb_getDataCount(&m_OutputBuf),freeSpaceInpBuf);
   return freeSpaceInpBuf;
}


int Play2Way::Play2Way_PutDataToSpeaker(uint32 pcm_dataRequest)
{
   LOGV("Play2Way_PutToSpk pcm_dataRequest=%d",pcm_dataRequest );

   int OutputBufDataCount = 0;
   uint32 PCMdataToModemSize = 0;
   uint32 offset=0, len=0;
   int type = 0;
   int header_size = 0;
   int8* pA2M_BufPtr;
   int32 maxBufLen;

   Play2Way_BufLock();

   // for VT + BG Sound case (A2M Shared buffer usage)
   // BG Sound use address 0~1408(BGS use 1408 bytes). PCM4WAY_play use address 1408~2048. (4WAY playback path: 320+320 bytes)
   pA2M_BufPtr = (int8 *)pLad->pCCCI->GetA2MShareBufAddress() + A2M_SHARED_BUFFER_OFFSET;

   maxBufLen = pLad->pCCCI->GetA2MShareBufLen();

   // check if the pcm_dataRequest size is larger than modem/AP share buffer size
   if ( pcm_dataRequest > maxBufLen ){
      LOGD("Play2Way_PutToSpk pcm_dataRequest=%d",pcm_dataRequest );
      pcm_dataRequest = maxBufLen;
   }

   PCMdataToModemSize = pcm_dataRequest;

   // check the output buffer data count
   OutputBufDataCount = rb_getDataCount(&m_OutputBuf);
   LOGV("Play2Way_PutToSpk OutputBufDataCount=%d",OutputBufDataCount );

   // if output buffer's data is not enough, fill it with zero to PCMdataToModemSize (ex: 320 bytes)
   if ( OutputBufDataCount < (int) PCMdataToModemSize ) {
      rb_writeDataValue(&m_OutputBuf, 0, PCMdataToModemSize - OutputBufDataCount);
      LOGD("Play2Way_PutToSpk underflow OutBufSize:%d", OutputBufDataCount);
   }

   type = LADBUFID_PCM_FillSpk;
   header_size = WriteShareBufHeader(pA2M_BufPtr, (int16)LADBUFID_PCM_FillSpk, (int16)PCMdataToModemSize, (int16)A2M_BUF_HEADER);

   // header size (6 bytes)
   pA2M_BufPtr += header_size;

   rb_copyToLinear(pA2M_BufPtr, &m_OutputBuf, PCMdataToModemSize);

   offset = A2M_SHARED_BUFFER_OFFSET;
   len = PCMdataToModemSize + header_size;  // PCMdataToModemSize=320. header_size=LAD_SHARE_HEADER_LEN(6)
   pLad->LAD_PCM2WayDataNotify(offset, len);

   LOGV("OutputBuf B:0x%x, R:%d, W:%d, L:%d\n",m_OutputBuf.pBufBase, m_OutputBuf.pRead-m_OutputBuf.pBufBase, m_OutputBuf.pWrite-m_OutputBuf.pBufBase, m_OutputBuf.bufLen);
   LOGV("pA2M_BufPtr B:0x%x, Offset:%d, L:%d\n",pA2M_BufPtr, offset, len);

   Play2Way_BufUnlock();
   return PCMdataToModemSize;
}



Record2Way::Record2Way(AudioYusuHardware *hw)
{
   mHw = hw;
   LOGD("Record2Way Constructor hw: %p\n", mHw);

   pCCCI = mHw->pLad->pCCCI;
   pLad = mHw->pLad;

   pRecord2Way_Buf = new char[PCM2WAY_BUFFER_SIZE];
   memset((void*)&m_M2A_ShareBuf,0,sizeof(rb));
   memset((void*)&m_InputBuf,0,sizeof(rb));

   pthread_mutex_init(&pRec2Way_Mutex, NULL);

   // Internal Input Buffer Initialization
   m_InputBuf.pBufBase = new char[AUDIO_INPUT_BUFFER_SIZE];
   m_InputBuf.bufLen   = AUDIO_INPUT_BUFFER_SIZE;
   m_InputBuf.pRead    = m_InputBuf.pBufBase;
   m_InputBuf.pWrite   = m_InputBuf.pBufBase;

   memset(m_InputBuf.pBufBase,0,m_InputBuf.bufLen);

   // Set M2A Share Buffer Pointer/Size
   m_M2A_ShareBuf.pBufBase = (char *)pLad->pCCCI->GetM2AShareBufAddress();
   m_M2A_ShareBuf.bufLen   = pLad->pCCCI->GetM2AShareBufLen();

   m_Rec2Way_Started = false;
}

Record2Way::~Record2Way()
{
   LOGD("~Record2Way De-constructor \n");
   if(pRecord2Way_Buf != NULL){
      delete[] pRecord2Way_Buf;
      pRecord2Way_Buf = NULL;
   }

   if(pLad != NULL){
      int Queue_count = pLad->pCCCI->GetQueueCount();
      LOGD("~Record2Way, Queue_count= %d",Queue_count);
      pLad->pCCCI->Record2WayLock();
      Record2Way_BufLock();
   }

   if(m_InputBuf.pBufBase != NULL){
      delete []m_InputBuf.pBufBase;
      m_InputBuf.pBufBase = NULL;
      m_InputBuf.bufLen   = 0;
      m_InputBuf.pRead    = NULL;
      m_InputBuf.pWrite   = NULL;
   }

   if(pLad != NULL){
      Record2Way_BufUnlock();
      pLad->pCCCI->Record2WayUnLock();
   }

}

void Record2Way::Record2Way_BufLock()
{
   pthread_mutex_lock(&pRec2Way_Mutex);
}

void Record2Way::Record2Way_BufUnlock()
{
   pthread_mutex_unlock(&pRec2Way_Mutex);
}

void Record2Way::Get_M2A_ShareBufferPtr()
{
   LOGV("Get_M2A_ShareBufferPtr. pShareR:0x%x, pShareW:0x%x \n",pCCCI->pShareR,pCCCI->pShareW);
   m_M2A_ShareBuf.pRead  = pCCCI->pShareR;
   m_M2A_ShareBuf.pWrite = pCCCI->pShareW;
}

int Record2Way::Record2Way_Start(int sample_rate)
{
   LOGD("Record2Way_Start \n");
   int ret;
   Record2Way_BufLock();

#ifdef DUMP_MODEM_VT_UL_DATA
   pVTMicFile = fopen("/data/Rec_vt.pcm","wb");
   if( pVTMicFile == NULL)
   {
      LOGD("Fail to Open VT File ");
   }
#endif

   m_Rec2Way_Started = true;

   // Reset read and write pointer of Input buffer
   m_InputBuf.pRead  = m_InputBuf.pBufBase;
   m_InputBuf.pWrite = m_InputBuf.pBufBase;

   // Start PCM2WAY GetFromMicrophone
   //ret =  pLad->LAD_PCM2WayRecordOn();

   Record2Way_BufUnlock();
   return true;
}

int Record2Way::Record2Way_Stop()
{
   LOGD("Record2Way_Stop \n");
   int ret;
   Record2Way_BufLock();

   m_Rec2Way_Started = false;

   // Stop PCM2WAY GetFromMicrophone
   //ret =  pLad->LAD_PCM2WayRecordOff();

   Record2Way_BufUnlock();

#ifdef DUMP_MODEM_VT_UL_DATA
   fclose(pVTMicFile);
#endif

   return true;
}

#define READ_DATA_FROM_MODEM_FAIL_CNT 10

int Record2Way::Record2Way_Read(void *buffer, int size_bytes)
{
   int ret = 0;
   int InputBuf_dataCnt = 0;
   int ReadDataAgain = 0;
   int consume_byte = size_bytes;
   char *buf = (char*)buffer;

   if(m_Rec2Way_Started == false)
   {
      LOGD("Record2Way_Read, m_Rec2Way_Started=false \n");
      return 0;
   }

   // if internal input buffer has enough data for this read, do it and return
   Record2Way_BufLock();
   InputBuf_dataCnt = rb_getDataCount(&m_InputBuf);
   if ( InputBuf_dataCnt >= consume_byte )
   {
      rb_copyToLinear((char *)buf, &m_InputBuf, consume_byte);
      Record2Way_BufUnlock();

#ifdef DUMP_MODEM_VT_UL_DATA
      fwrite(buf, sizeof(char), consume_byte, pVTMicFile);
#endif

      return consume_byte;
   }
   Record2Way_BufUnlock();


   // if internal input buffer is not enough, keep on trying
   while (ReadDataAgain++ < READ_DATA_FROM_MODEM_FAIL_CNT)
   {
      if(ReadDataAgain > (READ_DATA_FROM_MODEM_FAIL_CNT-1) ){
         LOGD("Record2Way_Read, fail, No data from modem: %d (%d) \n",ReadDataAgain,InputBuf_dataCnt);
      }
      // Interrupt period of pcm2way driver is 20ms.
      // If wait too long time (150 ms),
      //  -- Modem side has problem, the no interrupt is issued.
      //  -- pcm2way driver is stop. So AP can't read the data from modem.

      //wait some time then get data again from modem.
      usleep(15*1000);
      //Read data from modem again
      Record2Way_BufLock();
      InputBuf_dataCnt = rb_getDataCount(&m_InputBuf);
      if ( InputBuf_dataCnt >= consume_byte )
      {
         rb_copyToLinear((char *)buf, &m_InputBuf, consume_byte);
         Record2Way_BufUnlock();

#ifdef DUMP_MODEM_VT_UL_DATA
         fwrite(buf, sizeof(char), consume_byte, pVTMicFile);
#endif

         return consume_byte;
      }
      Record2Way_BufUnlock();
   }

   LOGD("Record2Way_Read, Modem fail \n");
   return 0;
}

void Record2Way::Record2Way_GetDataFromMicrophone()
{
   int InpBuf_freeSpace=0;
   int ShareBuf_dataCnt=0;

   Record2Way_BufLock();

   // update pointers of share buffer
   Get_M2A_ShareBufferPtr();

   // get free space of internal input buffer
   InpBuf_freeSpace = m_InputBuf.bufLen - rb_getDataCount(&m_InputBuf);
   LOGV("Rec2Way_GetFromMic input_Buf data_cnt:%d, freeSpace:%d\n",rb_getDataCount(&m_InputBuf),InpBuf_freeSpace);

   // get data count in share buffer
   ShareBuf_dataCnt = rb_getDataCount(&m_M2A_ShareBuf);
   LOGV("Rec2Way_GetFromMic share_Buf data_count:%d \n",ShareBuf_dataCnt);

   // check the data count in share buffer
   if ( ShareBuf_dataCnt > 320 )
   {
      LOGV("m_M2A_ShareBuf size > 320 (%d)\n", ShareBuf_dataCnt);
   }

   // check free space for internal input buffer
   if ( ShareBuf_dataCnt  > InpBuf_freeSpace  )
   {
      LOGV("Rec2Way_GetFromMic, uplink buffer full\n");
      Record2Way_BufUnlock();
      return;
   }

   // copy data from modem share buffer to internal input buffer
   rb_copyEmpty(&m_InputBuf, &m_M2A_ShareBuf);

#if defined(MTK_DUAL_MIC_SUPPORT)||defined(MTK_AUDIO_HD_REC_SUPPORT)
   if(mHw->META_Get_DualMic_Test_Mode()==TRUE)
   {
      LOGD("Rec2Way_GetFromMic, save file (%d)\n",ShareBuf_dataCnt);
      rb_copyToLinear((char *)pRecord2Way_Buf, &m_InputBuf, ShareBuf_dataCnt);
      if(mHw->m_DualMic_pOutFile != NULL){
         fwrite(pRecord2Way_Buf, sizeof(char), ShareBuf_dataCnt, mHw->m_DualMic_pOutFile);
      }
   }
#endif

   LOGV("InputBuf B:0x%x, R:%d, W:%d, L:%d\n", m_InputBuf.pBufBase, m_InputBuf.pRead-m_InputBuf.pBufBase, m_InputBuf.pWrite-m_InputBuf.pBufBase, m_InputBuf.bufLen);
   LOGV("M2A_ShareBuf B:0x%x, R:%d, W:%d, L:%d\n", m_M2A_ShareBuf.pBufBase, m_M2A_ShareBuf.pRead-m_M2A_ShareBuf.pBufBase, m_M2A_ShareBuf.pWrite-m_M2A_ShareBuf.pBufBase, m_M2A_ShareBuf.bufLen);

   Record2Way_BufUnlock();
}




}

