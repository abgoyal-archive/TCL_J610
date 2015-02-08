
#ifndef __HEADSET_DETECT_H__
#define __HEADSET_DETECT_H__

#include <utils/threads.h>

namespace android {
    class HeadSetDetect : public Thread
    {
    public:
        enum{
            NO_WIRED_LINE='0',
            WIRE_HEADSET='1',
            BIT_HEADSET_NO_MIC='2'
            };
        typedef void (*callback_t)(void* user, bool on);
        HeadSetDetect(void * observer, callback_t cblk);
        ~HeadSetDetect();
         void stop();
         bool isPlaying();
         static void handler (void * data,const char * msg,int msg_len);
    private:
        HeadSetDetect(const HeadSetDetect &);
        HeadSetDetect & operator=(const HeadSetDetect&);
        virtual bool		threadLoop();
        virtual status_t	readyToRun();
        virtual void		onFirstRef();
        int detect();
        status_t init();
        //bool checkValue(char *buffer, int len);
        int parseState(char *buffer, int len);
        int readStateFromFile();
        bool headsetConnect(int newState);
        int socketInit();
		int socketNextEvent(char* buffer, int buffer_length);
    private:
        volatile bool        mActive;
        bool        mOn;
        Mutex		mLock;
        void *      mObserver;
        callback_t  mCblk;
        int         mFd;
    };
}
#endif
