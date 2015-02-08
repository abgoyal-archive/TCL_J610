
#ifndef MTKOVERLAY_H
#define MTKOVERLAY_H


//--------------------------------------------------------------------------------------------------
#include <hardware/hwcomposer.h>
#include <utils/KeyedVector.h>
#include <ui/Rect.h>
#include <ui/Region.h>
#include <utils/Singleton.h>
#include <utils/threads.h>
#include "Transform.h"

#include "mtkfb.h"
#include "MediaHal.h"
#include <semaphore.h>  //clouds:for semaphore in FBThread


using namespace android;


//--------------------------------------------------------------------------------------------------
namespace MtkOverlay {


//---------------------------------------------
// global variables and functions
//----------------------------------------------
static void dump_layer(hwc_layer_t const* l);


//----------------------------------------------
// overlay node for data and device connection
//----------------------------------------------
class Node {
private:
    struct Buffer {
        sp<GraphicBuffer> srcGraphicBuffer; // source buffer handle

        uint32_t stride;                // buffer stride
        uint32_t transform;             // for rotation while conversion

        Rect srcFrame;                  // source buffer frame
        Rect srcCrop;                   // source buffer crop

        Rect dstFrame;                  // destination display frame
        Rect dstCrop;                   // destination crop

        uint8_t *data;                  // buffer data here
        mHalRegisterLoopMemoryObj_t *mva;   // mva register object

        Buffer()
            : srcGraphicBuffer(NULL)
            , stride(0)
            , transform(Transform::ROT_INVALID)
            , data(NULL)
            , mva(NULL) { }
    };
    Buffer mBuffers[3];                 // triple bufferring
    Buffer *mBackBuffer;
    Buffer *mFrontBuffer;
    Buffer *mDisplayBuffer;

    bool mIsDirty;                      // dirty to check if update for overlay is required
    bool mIsVisible;                    // visible to decide if have to send node data into overlay

    status_t createBuffer(Buffer *buf);
    status_t releaseBuffer(Buffer *buf);

    uint32_t convertToMHalRotation(uint32_t src);
    bool     mapFormatForMHal(uint32_t src, uint32_t &MHalSrc, uint32_t &MHalDst);
    bool     mapFormatForFB(uint32_t src, uint32_t &FBSrc);
    bool     mapFormatForTVOut(uint32_t src, uint32_t &TVOutSrc);

    status_t mapCropRect(               // for mapping crop region linearly
        const Rect &inBase, const Rect &inCrop,
        const Rect &outBase, Rect &outCrop);

public:
    uint32_t mIdentity;                 // related layer identity
    bool     mIsLinked;                 // linked to hwcomposer layer
    bool     mIsPQNeeded;               // true if use PQ in MDP image Transform
    Mutex    *mLock;                    // for MDP control

    Node();
    ~Node();

    void dump(struct hwc_composer_device* dev, char *buff, int buff_len);

    status_t updateInfo(hwc_layer_t *hwcl);
    status_t updateImage();
    status_t flip();

    status_t setToOverlay(fb_overlay_layer* overlay);
    status_t setToTVOut();
    status_t leaveVideoMode();
};


//---------------------------------------------
// overlay manager
//---------------------------------------------
class Manager : public Singleton<Manager> {
private:
    int  mTVOut;                                    // tv out device
    int  mFB;                                       // framebuffer device
    int  mOverlayCount;                             // available overlay count
    bool mIsCameraLayerExisting;                    // true if hwc take camera layer

    DefaultKeyedVector<uint32_t, Node*> mNodesMap;  // for mapping layer to overlay Node
    Vector<uint32_t> mKeysForUnlinkedNodes;         // for unlinked node ready to be removed
    Vector<Node*> mSortedNodes;                     // keep nodes by z order
    Node *mTVOutNode;                               // node to send to TV out

    fb_overlay_layer *mOverlays[2];                 // overlay info for overlay device
    fb_overlay_layer *mBackOverlays;                // double buffering
    fb_overlay_layer *mFrontOverlays;
    Mutex   mLock;                                  // [mtk04189] used to lock set() and prepare()

    Node     *getAvailableNode(uint32_t identity);
    status_t processHWComposerLayers(hwc_layer_list_t *list);

    status_t setUnlinkedNodes();
    status_t releaseUnlinkedNodes();

public:
    Manager();
    virtual ~Manager();

    int getFB() { return mFB; }
    int getTVOut() { return mTVOut; }

    void     dump(struct hwc_composer_device* dev, char *buff, int buff_len);
    status_t checkSatus();

    status_t prepare(hwc_layer_list_t *list);       // prepare and set for HWComposer
    status_t set();
};


}


//------------------------------------------------------------------------------------------------
ANDROID_SINGLETON_STATIC_INSTANCE(MtkOverlay::Manager);


//------------------------------------------------------------------------------------------------
#endif
