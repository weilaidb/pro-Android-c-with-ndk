LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := AVIPlayer
LOCAL_SRC_FILES := \
	Common.cpp \
	com_apress_aviplayer_AbstractPlayerActivity.cpp \
	com_apress_aviplayer_BitmapPlayerActivity.cpp \
	com_apress_aviplayer_OpenGLPlayerActivity.cpp \
	com_apress_aviplayer_NativeWindowPlayerActivity.cpp

# Use AVILib static library 
LOCAL_STATIC_LIBRARIES += avilib_static

# Link with JNI graphics
LOCAL_LDLIBS += -ljnigraphics

# Enable GL ext prototypes
LOCAL_CFLAGS += -DGL_GLEXT_PROTOTYPES

# Link with OpenGL ES
LOCAL_LDLIBS += -lGLESv1_CM

# Link with Android library
LOCAL_LDLIBS += -landroid

include $(BUILD_SHARED_LIBRARY)

# Import AVILib library module
$(call import-module, transcode-1.1.5/avilib)
