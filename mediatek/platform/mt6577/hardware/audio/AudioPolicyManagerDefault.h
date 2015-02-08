


#include <stdint.h>
#include <stdbool.h>

#include <hardware_legacy/AudioPolicyManagerBase.h>

namespace android_audio_legacy {

class AudioPolicyManagerDefault: public AudioPolicyManagerBase
{

public:
                AudioPolicyManagerDefault(AudioPolicyClientInterface *clientInterface)
                : AudioPolicyManagerBase(clientInterface) {}

        virtual ~AudioPolicyManagerDefault() {}

protected:
        // true is current platform implements a back microphone
        virtual bool hasBackMicrophone() const { return false; }
#ifdef WITH_A2DP
        // true is current platform supports suplication of notifications and ringtones over A2DP output
        virtual bool a2dpUsedForSonification() const { return true; }
#endif

};
};
