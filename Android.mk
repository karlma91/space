LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := main

LOCAL_CFLAGS    := -DANDROID
LOCAL_C_INCLUDES := $(LOCAL_PATH)/lib/LList

LOCAL_SHARED_LIBRARIES := SDL2
LOCAL_SHARED_LIBRARIES += SDL2_image
LOCAL_SHARED_LIBRARIES += Chipmunk
LOCAL_SHARED_LIBRARIES += mxml
LOCAL_SHARED_LIBRARIES += zzip

LOCAL_SRC_FILES := lib/SDL2/android/SDL_android_main.cpp \
	$(subst $(LOCAL_PATH)/,,$(wildcard $(LOCAL_PATH)/src/*.c)) \
	lib/LList/llist.c

LOCAL_CFLAGS += -DGL_GLEXT_PROTOTYPES
LOCAL_LDLIBS := -ldl -lGLESv1_CM -landroid -llog -lc -lz

include $(BUILD_SHARED_LIBRARY)
