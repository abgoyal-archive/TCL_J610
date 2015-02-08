
#ifndef _FAKECCCI_H_
#define _FAKECCCI_H_



#include "AudioYusuDef.h"

namespace android {


#define FAKECCCI_PLAY_FRAME_LEN  320

/* constant definition */
extern const uint16 sineTable[320];

class FakeCCCI
{
private:

   uint32 fHdl;                  /* file handle for CCCI user space interface */
   uint32 mMsg[4];



public:

   uint32 mSendMsg;

#if defined(PC_EMULATION)
   CRITICAL_SECTION fMsgMutex;

   HANDLE hCCCIRecThread;
   HANDLE hCCCIPlayThread;

   HANDLE hEventDataOK;
   HANDLE hEventSendBack;
   HANDLE hEventPlayDataNotify;
#else
   pthread_mutex_t fMsgMutex;

   pthread_t hCCCIRecThread;
   pthread_t hCCCIPlayThread;

   pthread_cond_t  hEventSendBack;
   pthread_cond_t  hEventDataOK;
   pthread_cond_t  hEventPlayDataNotify;
   pthread_mutex_t fdataMutex;
   pthread_mutex_t fbkMutex;
   pthread_mutex_t fpdnMutex;
   int32 fbkMsgFlag;
   int32 fdataMsgFlag;
   int32 fpdnMsgFlag;
#endif

   // recording thread use
   int8  mRunFlag;

   // playback thread use
   int8  mpRunFlag;

   int8* pBufBase;     /* a pointer to fake share buffer */
   int8* pM2ABufBase;
   int8* pA2MBufBase;
   uint32 offset;
   uint32 validDataLen;

   // playback thread use
   uint32 playDataOffset;
   uint32 playDataLen;


   FakeCCCI();
   ~FakeCCCI();
   uint32 GetBaseAddress();
   bool SendMessage(uint32 *pMsg);
   bool ReadMessage(uint32 *pMsg);
   bool CreateCCCIFakeRecThread();
   bool CreateCCCIFakePlayThread();
   void MessageLock();
   void MessageUnLock();
   int32 FakeCCCI_RecSendbackEvent_Set();
   int32 FakeCCCI_RecSendbackEvent_Wait();
   int32 FakeCCCI_ModemSideEvent_Set();
   int32 FakeCCCI_ModemSideEvent_Wait();
   int32 FakeCCCI_PlayDataNotifyEvent_Set();
   int32 FakeCCCI_PlayDataNotifyEvent_Wait();
};

}; // namespace android

#endif   //_AUDIOCCCI_H_
