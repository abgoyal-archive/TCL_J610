#  only if use yusu audio will build this.
ifeq ($(strip $(BOARD_USES_YUSU_AUDIO)),true)

LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

AudioDriverIncludePath := aud_drv

ifeq ($(AUDIO_POLICY_TEST),true)
  ENABLE_AUDIO_DUMP := true
endif

ifeq ($(strip $(TARGET_BUILD_VARIANT)),eng)
  LOCAL_CFLAGS += -DDEBUG_AUDIO_PCM
endif


ifeq ($(strip $(MTK_PLATFORM)),MT6577)
  LOCAL_CFLAGS += -DMT6577
  AudioDriverName := mt6577
endif

ifeq ($(strip $(BOARD_USES_GENERIC_AUDIO)),true)
  LOCAL_CFLAGS += -DGENERIC_AUDIO
else
  LOCAL_CFLAGS += -DMTK_AUDIO  
endif

ifeq ($(MTK_DIGITAL_MIC_SUPPORT),yes)
  LOCAL_CFLAGS += -DMTK_DIGITAL_MIC_SUPPORT
endif

ifeq ($(strip $(BOARD_USES_YUSU_AUDIO)),true)
LOCAL_GENERATE_CUSTOM_FOLDER := custom:hal/audioflinger
LOCAL_C_INCLUDE += $(LOCAL_PATH)/custom
-include $(TOPDIR)$(MTK_PATH_SOURCE)/hardware/mtk/audio/mtk_cust.mk
endif

ifeq ($(strip $(BOARD_USES_YUSU_AUDIO)),true)
  LOCAL_C_INCLUDES:= \
    $(TOPDIR)/hardware/libhardware_legacy/include \
    $(TOPDIR)/frameworks/base/include \
    $(MTK_PATH_PLATFORM)/hardware/audio/aud_drv \
    $(MTK_PATH_PLATFORM)/hardware/audio/LAD \
    $(MTK_PATH_SOURCE)/external/nvram/libnvram \
    $(MTK_PATH_SOURCE)/external/AudioCompensationFilter \
    $(MTK_PATH_SOURCE)/external/HeadphoneCompensationFilter \
    $(MTK_PATH_SOURCE)/frameworks/base/include/media \
    $(MTK_PATH_SOURCE)/frameworks/base/include \
    $(MTK_PATH_SOURCE)/external/audiodcremoveflt \
    $(MTK_PATH_SOURCE)/external/audiocustparam \
    $(MTK_PATH_SOURCE)/kernel/include \
    $(MTK_PATH_SOURCE)/kernel/drivers/hdmitx
endif

LOCAL_SRC_FILES+= \
    AudioHardwareGeneric.cpp \
    AudioHardwareStub.cpp \
    AudioHardwareInterface.cpp \
    audio_hw_hal.cpp
   
ifeq ($(strip $(BOARD_USES_YUSU_AUDIO)),true)
  LOCAL_SRC_FILES+= \
      $(AudioDriverIncludePath)/AudioAnalogAfe.cpp \
      $(AudioDriverIncludePath)/AudioAfe.cpp \
      $(AudioDriverIncludePath)/AudioFtm.cpp \
      $(AudioDriverIncludePath)/AudioI2S.cpp \
      $(AudioDriverIncludePath)/AudioYusuHeadsetMessage.cpp \
      $(AudioDriverIncludePath)/AudioYusuHardware.cpp \
      $(AudioDriverIncludePath)/AudioYusuStreamHandler.cpp \
      $(AudioDriverIncludePath)/AudioYusuVolumeController.cpp \
      $(AudioDriverIncludePath)/AudioYusuStreamOut.cpp \
      $(AudioDriverIncludePath)/AudioYusuStreamIn.cpp \
      $(AudioDriverIncludePath)/AudioYusuApStreamIn.cpp \
      $(AudioDriverIncludePath)/AudioYusuI2SStreamIn.cpp \
      $(AudioDriverIncludePath)/AudioYusuDef.cpp \
      $(AudioDriverIncludePath)/AudioStreamInHandler.cpp \
      $(AudioDriverIncludePath)/AudioAMPControlInterface.cpp \
      $(AudioDriverIncludePath)/HeadSetDetect.cpp \
      audio_hw_I2S_hal.cpp \
      LAD/AudioYusuCcci.cpp \
      LAD/AudioYusuLad.cpp \
      LAD/AudioYusuLadPlayer.cpp \
      LAD/AudioPcm2way.cpp      

  ifeq ($(EXT_DAC_SUPPORT),yes)
  LOCAL_SRC_FILES+= \
      $(AudioDriverIncludePath)/AudioCustomizationBase.cpp \
      custom/audio_customization.cpp
  LOCAL_CFLAGS += -DENABLE_EXT_DAC
  endif 
       
  ifeq ($(MTK_AUDIO_GAIN_TABLE_SUPPORT),yes)
  LOCAL_SRC_FILES+= \
      $(AudioDriverIncludePath)/AudioUcm.cpp
  endif
       
  ifeq ($(MTK_EXT_AMP_SUPPORT),yes)
    LOCAL_CFLAGS += -DENABLE_EXT_AMP
  endif    
endif

ifeq ($(ENABLE_AUDIO_DUMP),true)
  LOCAL_SRC_FILES += AudioDumpInterface.cpp
  LOCAL_CFLAGS += -DENABLE_AUDIO_DUMP
endif

LOCAL_STATIC_LIBRARIES := \
    libmedia_helper \
    libaudiodcrflt

LOCAL_SHARED_LIBRARIES += \
    libmedia \
    libcutils \
    libutils \
    libbinder \
    libhardware_legacy \
    libhardware \
    libblisrc \
    libdl \
    libnvram \
    libaudiocompensationfilter \
    libheadphonecompensationfilter \
    libpowermanager \
    libaudiocustparam
    
ifeq ($(HAVE_MATV_FEATURE),yes)
  ifeq ($(MTK_MATV_ANALOG_SUPPORT),yes)
    LOCAL_CFLAGS += -DMATV_AUDIO_LINEIN_PATH
  endif
endif

ifeq ($(MTK_DUAL_MIC_SUPPORT),yes)
  LOCAL_CFLAGS += -DMTK_DUAL_MIC_SUPPORT
endif

ifeq ($(MTK_WB_SPEECH_SUPPORT),yes)
  LOCAL_CFLAGS += -DMTK_WB_SPEECH_SUPPORT
endif

ifeq ($(strip $(MTK_FM_SUPPORT)),yes)
ifeq ($(strip $(MTK_FM_TX_SUPPORT)),yes)
ifeq ($(strip $(MTK_FM_TX_AUDIO)),FM_DIGITAL_OUTPUT)
  LOCAL_CFLAGS += -DFM_DIGITAL_OUT_SUPPORT
endif
ifeq ($(strip $(MTK_FM_TX_AUDIO)),FM_ANALOG_OUTPUT)
  LOCAL_CFLAGS += -DFM_ANALOG_OUT_SUPPORT
endif
endif

ifeq ($(strip $(MTK_FM_RX_SUPPORT)),yes)
ifeq ($(strip $(MTK_FM_RX_AUDIO)),FM_DIGITAL_INPUT)
  LOCAL_CFLAGS += -DFM_DIGITAL_IN_SUPPORT
endif
ifeq ($(strip $(MTK_FM_RX_AUDIO)),FM_ANALOG_INPUT)
  LOCAL_CFLAGS += -DFM_ANALOG_IN_SUPPORT
endif
endif

endif

ifeq ($(MTK_DUAL_MIC_SUPPORT),yes)
  LOCAL_CFLAGS += -DMTK_DUAL_MIC_SUPPORT
endif

ifeq ($(MTK_WB_SPEECH_SUPPORT),yes)
  LOCAL_CFLAGS += -DMTK_WB_SPEECH_SUPPORT
endif

ifeq ($(MTK_SPH_EHN_CTRL_SUPPORT),yes)
  LOCAL_CFLAGS += -DMTK_SPH_EHN_CTRL_SUPPORT
endif

ifeq ($(MTK_BT_SUPPORT),yes)
  ifeq ($(MTK_BT_PROFILE_A2DP),yes)
  LOCAL_CFLAGS += -DWITH_A2DP
  endif
else
  ifeq ($(strip $(BOARD_HAVE_BLUETOOTH)),yes)
    LOCAL_CFLAGS += -DWITH_A2DP
  endif  
endif

# SRS Processing
ifeq ($(strip $(HAVE_SRSAUDIOEFFECT_FEATURE)),yes)
LOCAL_CFLAGS += -DHAVE_SRSAUDIOEFFECT
endif
# SRS Processing

# Audio HD Record
ifeq ($(MTK_AUDIO_HD_REC_SUPPORT),yes)
LOCAL_CFLAGS += -DMTK_AUDIO_HD_REC_SUPPORT
endif
# Audio HD Record

LOCAL_ARM_MODE := arm
LOCAL_MODULE := libaudio.primary.default
LOCAL_MODULE_TAGS := user

include $(BUILD_SHARED_LIBRARY)


# The default audio policy, for now still implemented on top of legacy
# policy code
include $(CLEAR_VARS)

AudioDriverName := default
  
ifeq ($(strip $(BOARD_USES_GENERIC_AUDIO)),true)
  LOCAL_CFLAGS += -DGENERIC_AUDIO
else
  LOCAL_CFLAGS += -DMTK_AUDIO
endif

ifeq ($(HAVE_MATV_FEATURE),yes)
  ifeq ($(MTK_MATV_ANALOG_SUPPORT),yes)
    LOCAL_CFLAGS += -DMATV_AUDIO_LINEIN_PATH
  endif
endif

ifeq ($(strip $(BOARD_USES_YUSU_AUDIO)),true)
  LOCAL_C_INCLUDES:= \
    $(TOPDIR)/hardware/libhardware_legacy/include \
    $(TOPDIR)/frameworks/base/include \
    $(MTK_PATH_PLATFORM)/hardware/audio/aud_drv \
    $(MTK_PATH_PLATFORM)/hardware/audio/LAD \
    $(MTK_PATH_SOURCE)/external/nvram/libnvram \
    $(MTK_PATH_SOURCE)/external/AudioCompensationFilter \
    $(MTK_PATH_SOURCE)/external/HeadphoneCompensationFilter \
    $(MTK_PATH_SOURCE)/external/audiocustparam \
    $(MTK_PATH_SOURCE)/frameworks/base/include/media \
    $(MTK_PATH_SOURCE)/frameworks/base/include \
    $(MTK_PATH_SOURCE)/external/audiodcremoveflt \
    $(MTK_PATH_SOURCE)/kernel/include
endif

LOCAL_SRC_FILES := \
    AudioPolicyManagerBase.cpp \
    AudioPolicyCompatClient.cpp \
    audio_policy_hal.cpp \
    AudioPolicyManagerDefault.cpp \
    AudioYusuPolicyManager.cpp

LOCAL_SHARED_LIBRARIES := \
    libcutils \
    libutils \
    libmedia \
    libaudiocustparam

LOCAL_STATIC_LIBRARIES := \
    libmedia_helper

LOCAL_MODULE := audio_policy.$(AudioDriverName)
LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)/hw
LOCAL_MODULE_TAGS := user

ifeq ($(MTK_BT_SUPPORT),yes)
  ifeq ($(MTK_BT_PROFILE_A2DP),yes)
  LOCAL_CFLAGS += -DWITH_A2DP
  endif
else
  ifeq ($(strip $(BOARD_HAVE_BLUETOOTH)),yes)
    LOCAL_CFLAGS += -DWITH_A2DP
  endif  
endif

include $(BUILD_SHARED_LIBRARY)

# The a2dp hardware interface
include $(CLEAR_VARS)

ifeq ($(strip $(TARGET_BUILD_VARIANT)),eng)
  LOCAL_CFLAGS += -DDEBUG_AUDIO_PCM
endif

ifeq ($(strip $(BOARD_USES_GENERIC_AUDIO)),true)
  LOCAL_CFLAGS += -DGENERIC_AUDIO
else
  LOCAL_CFLAGS += -DMTK_AUDIO  
endif

ifeq ($(strip $(BOARD_USES_YUSU_AUDIO)),true)
  LOCAL_C_INCLUDES:= \
    $(TOPDIR)/hardware/libhardware_legacy/include \
    $(TOPDIR)/frameworks/base/include \
    $(MTK_PATH_PLATFORM)/hardware/audio/aud_drv \
    $(MTK_PATH_PLATFORM)/hardware/audio/LAD \
    $(MTK_PATH_SOURCE)/external/nvram/libnvram \
    $(MTK_PATH_SOURCE)/external/AudioCompensationFilter \
    $(MTK_PATH_SOURCE)/external/HeadphoneCompensationFilter \
    $(MTK_PATH_SOURCE)/frameworks/base/include/media \
    $(MTK_PATH_SOURCE)/frameworks/base/include \
    $(MTK_PATH_SOURCE)/external/audiodcremoveflt \
    $(MTK_PATH_SOURCE)/external/audiocustparam \
    $(MTK_PATH_SOURCE)/kernel/include
endif

LOCAL_SRC_FILES+= \
    A2dpAudioInterface.cpp\
    a2dpaudio_hw_hal.cpp \
    AudioHardwareGeneric.cpp \
    AudioHardwareStub.cpp \
    AudioHardwareInterface.cpp    

LOCAL_STATIC_LIBRARIES := \
    libmedia_helper

LOCAL_SHARED_LIBRARIES += \
    libmedia \
    libcutils \
    libutils \
    libbinder \
    libhardware_legacy \
    libhardware \
    libaudio.primary.default

ifeq ($(MTK_BT_SUPPORT),yes)
  ifeq ($(MTK_BT_PROFILE_A2DP),yes)
    LOCAL_SHARED_LIBRARIES += libmtkbtextadpa2dp
    LOCAL_CFLAGS += -DWITH_BLUETOOTH -D__BTMTK__
    LOCAL_CFLAGS += -DWITH_A2DP
    MTK_BT_PATH := $(MTK_PATH_SOURCE)/external/bluetooth/blueangel
    LOCAL_C_INCLUDES += $(call include-path-for, bluez) $(MTK_BT_PATH)/btadp_ext/include
  endif
else
  ifeq ($(BOARD_HAVE_BLUETOOTH),true)
    LOCAL_SHARED_LIBRARIES += liba2dp
    LOCAL_CFLAGS += -DWITH_BLUETOOTH
    LOCAL_CFLAGS += -DWITH_A2DP
    LOCAL_C_INCLUDES += $(call include-path-for, bluez)    
  endif
endif

LOCAL_ARM_MODE := arm
LOCAL_MODULE := libaudio.a2dp.default
LOCAL_MODULE_TAGS := user
include $(BUILD_SHARED_LIBRARY)

endif
