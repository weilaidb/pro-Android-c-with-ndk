LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := WAVPlayer
LOCAL_SRC_FILES := WAVPlayer.cpp

# Use WAVLib static library
LOCAL_STATIC_LIBRARIES += wavlib_static

# Link with OpenSL ES
LOCAL_LDLIBS += -lOpenSLES

include $(BUILD_SHARED_LIBRARY)

# Import WAVLib library module
$(call import-module, transcode-1.1.5/avilib)
