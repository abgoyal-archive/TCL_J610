

#define LOG_TAG "AudioPolicyManagerDefault"
//#define LOG_NDEBUG 0

#include "AudioPolicyManagerDefault.h"
#include "AudioYusuPolicyManager.h"

namespace android_audio_legacy {

extern "C" AudioPolicyInterface* createAudioPolicyManager(AudioPolicyClientInterface *clientInterface)
{
    //return new AudioPolicyManagerDefault(clientInterface);
    LOGD("new AudioYusuPolicyManager(clientInterface);");
    return new AudioYusuPolicyManager(clientInterface);
}

extern "C" void destroyAudioPolicyManager(AudioPolicyInterface *interface)
{
    delete interface;
}

}; // namespace android
