

#ifndef ANDROID_AUDIO_PCM2WAY_H
#define ANDROID_AUDIO_PCM2WAY_H


//#define DUMP_MODEM_VT_UL_DATA


#include <stdint.h>
#include <sys/types.h>




namespace android {

class AudioCCCI;



class Play2Way
{
public:
   Play2Way(AudioYusuHardware *hw);
   ~Play2Way();

   void Play2Way_BufLock();
   void Play2Way_BufUnlock();

   int Play2Way_Start(int sample_rate);
   int Play2Way_Stop();
   int Play2Way_Write(void *buffer, int size_bytes);
   int Play2Way_GetFreeBufferCount(void);
   int Play2Way_PutDataToSpeaker(uint32 pcm_dataRequest);

   AudioYusuHardware *mHw;
   AudioCCCI         *pCCCI;
   LAD               *pLad;

   pthread_mutex_t pPlay2Way_Mutex;     // Mutex to protect internal buffer
   char            *pPlay2Way_Buf;
   char            *pA2M_SharedBufPtr;  // A2M Share Buffer
   rb              m_OutputBuf;         // Internal Output Buffer for Put Data to Modem via Receive(Speaker)
   bool            m_Play2Way_Started;
};

class Record2Way
{
public:
   Record2Way(AudioYusuHardware *hw);
   ~Record2Way();

   void Record2Way_BufLock();
   void Record2Way_BufUnlock();
   void Get_M2A_ShareBufferPtr();

   int Record2Way_Start(int sample_rate);
   int Record2Way_Stop();
   int Record2Way_Read(void *buffer, int size_bytes);
   void Record2Way_GetDataFromMicrophone();

   AudioYusuHardware *mHw;
   AudioCCCI         *pCCCI;
   LAD               *pLad;

   pthread_mutex_t pRec2Way_Mutex;    // Mutex to protect internal buffer
   char            *pRecord2Way_Buf;
   rb              m_M2A_ShareBuf;      // M2A Share Buffer
   rb              m_InputBuf;          // Internal Input Buffer for Get From Microphone Data
   bool            m_Rec2Way_Started;
#ifdef DUMP_MODEM_VT_UL_DATA
   FILE *pVTMicFile;
#endif
};



}; // namespace android

#endif
