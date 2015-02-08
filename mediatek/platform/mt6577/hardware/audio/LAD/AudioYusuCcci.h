
#ifndef _AUDIO_YUSU_CCCI_H_
#define _AUDIO_YUSU_CCCI_H_


#include <stdio.h>
#include "ccci.h"
#include <pthread.h>

#include "AudioYusuDef.h"


#if defined(PC_EMULATION)
#include "windows.h"
#endif

#if defined(FAKE_CCCI)
#include "fake_ccci.h"
#endif

namespace android {

/* constant definition */
#define CCCI_PCM_SHARE_BUF_LEN      16384
#define M2A_SHARE_BUF_LEN           ( CCCI_PCM_SHARE_BUF_LEN >> 1 )
#define A2M_SHARE_BUF_LEN           ( CCCI_PCM_SHARE_BUF_LEN >> 1 )
#define A2M_SHARE_BUF_EMPAREA_LEN   512

#define A2M_SHARE_BUF_PCMAREA_LEN   A2M_SHARE_BUF_LEN


//For VT case, the CCCI message for every 20ms, UL/DL have 2 CCCI message (Put to Speaker / Get from Mic)
//For BGS off ack message, the worst case maybe pending 150 ms. And for other change device control. (BGSoff,2WAY off,SPH off,...)
//The total message maybe > 20 for this period. So enlarge the total CCCI message queue.
//For the CCCI queue in CCCI kernel driver, the size is 60.
//For the CCCI queue in Modem side, the size is 32.
//Modem side would keep on optimized the BGS off ack period.
#define AUDCCCI_MAX_QUEUE_LEN       60

   // for VT + BG Sound case (A2M Shared buffer usage)
   // BG Sound use address 0~1408(BGS use 1408 bytes). PCM4WAY_play use address 1408~2048. (4WAY playback path: (640)320+320 bytes)
   // But for the better solution, A2M shared buffer should use ring buffer.
#define A2M_SHARED_BUFFER_OFFSET  (1408)

// for VT + BG Sound + Set Speech Parameter case (A2M Shared buffer usage)
// BG Sound use address 0~1408(BGS use 1408 bytes). PCM4WAY_play use address 1408~2048. (4WAY playback path: (640)320+320 bytes)
// Speech Par use address1408~3455. (The overlap of 2way and sph par should be safe.)
// But for the better solution, A2M shared buffer should use ring buffer.
#define SPH_PAR_A2M_SHARED_BUFFER_OFFSET  (1408)

/* used to pack CCCI messages, MSG1 corresponds to 1-parameter, MSG2 corresponds to 2-parameter messages */

//Pack CCCI Message
extern uint32 CCCI_MSG1(uint32 Function, uint32 Param1);
extern uint32 CCCI_MSG2(uint32 Function, uint32 Param1, uint32 Param2);
extern uint32 CCCI_MSG3(uint32 Function, uint32 Param1, uint32 Param2);

//Parse CCCI Message
extern uint32 CCCI_MESSAGE_TYPE_PARSER(uint32 CCCI_MESSAGE);
extern uint32 CCCI_MESSAGE_FUNCTION_PARSER(uint32 CCCI_MESSAGE);
extern uint32 CCCI_MESSAGE_OFFSET_PARSER(uint32 CCCI_MESSAGE);
extern uint32 CCCI_MESSAGE_LENGTH_PARSER(uint32 CCCI_MESSAGE);
extern uint32 CCCI_MESSAGE_PARAM1_PARSER(uint32 CCCI_MESSAGE);


class AudioYusuHardware;

#if defined(FAKE_CCCI)
class FakeCCCI;
#endif

struct qele {
   uint32 msg;
   uint32 ack;
};

class AudioCCCI
{
private:

   // file handle for CCCI user space interface
   int32 fHdl;                   //for file read
   int32 fHdlAck;                //for file write
   int32 hPcmRx;                 //for mmap

   // share buffer base and len
   uint32 mShareBufAdd;
   uint32 mShareBufLen;

   // for message queue
   qele pQueue[AUDCCCI_MAX_QUEUE_LEN];
   int32 qLen;
   int32 iQRead;
   int32 iQWrite;


#if defined(PC_EMULATION)
   CRITICAL_SECTION fQMutex;
   CRITICAL_SECTION mStreaminMutex;
   HANDLE hReadThread;
#else
   pthread_mutex_t fQMutex;
   pthread_mutex_t mStreaminMutex;
   pthread_t hReadThread;
#endif

#if defined(FAKE_CCCI)
   FakeCCCI *pFakeCCCI;     /* a pointer to a fake ccci */
#endif

   pthread_mutex_t m2WayRecMutex;
   pthread_mutex_t m2WayPlayMutex;
   pthread_mutex_t mA2MShareBufMutex;
   pthread_mutex_t mSetSpeechCoeffMutex;

   pthread_t hSendSphThread;

public:

   int8 *pShareR;
   int8 *pShareW;
   bool CCCIEnable;
   bool b_modem_reset;
   int  mFd;
   AudioYusuHardware *mHw;    /* pointer to HW */
   uint32 m_last_write_ccci_msg;
   uint32 m_last_read_ccci_msg;
   int32 m_MDReset_recover_record;

   AudioCCCI(AudioYusuHardware *hw);
   ~AudioCCCI();
   bool Initial();
   bool Deinitial();
   //uint32 GetBaseAddress();
   //uint32 GetShareBufLen();
   uint32 GetM2AShareBufLen();
   uint32 GetA2MShareBufLen();
   uint32 GetM2AShareBufAddress();
   uint32 GetA2MShareBufAddress();
   int SendMessage(uint32 MSG);
   int ReadMessage(uint32 *msg);
   bool CreateReadingThread();
   int SendMessageInQueue(uint32 MSG);
   int32 GetQueueCount();
   bool CheckOffsetandLength(uint32 MSG);

   bool MDReset_CheckMessageInQueue();
   bool MDReset_FlushMessageInQueue();
   bool ConsumeMessageInQueue();
   bool BGS_On_MSG_ACK_CheckMessageInQueue();
   int32 searchPrevAckQElement(int32 idx);
   int32 JudgeAckOfMsg(int32 MSG);
   void MsgQueueLock();
   void MsgQueueUnLock();
   void RecordLock();
   void RecordUnLock();

   void Record2WayLock();
   void Record2WayUnLock();
   void Play2WayLock();
   void Play2WayUnLock();

   bool A2MBufLock();
   void A2MBufUnLock();

   bool SpeechCoeffLock();
   void SpeechCoeffUnLock();

   int GetTxFd();
   int GetRxFd();

   bool CreateSendSphParaThread();
#if defined(MTK_DUAL_MIC_SUPPORT)||defined(MTK_AUDIO_HD_REC_SUPPORT)
   void DualMic_WriteDataToFile(int32 msg);
   void DualMic_ReadDataFromFile();
   void DualMic_WriteDataToRecBuf(int32 msg);
   void DualMic_ReadDataFromPlayBuf();
#endif

};

}; // namespace android

#endif   //_AUDIO_YUSU_CCCI_H_
