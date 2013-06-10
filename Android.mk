LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := main

SDL_PATH := SDL
SDL_IMAGE_PATH := SDL_image
CHIPMUNK_PATH := Chipmunk/include/chipmunk
MXML_PATH := mxml
ZZIP_PATH := zzip

LOCAL_CFLAGS    := -DANDROID

LOCAL_C_INCLUDES := $(SDL_PATH)/include
LOCAL_C_INCLUDES += $(SDL_IMAGE_PATH)
LOCAL_C_INCLUDES += $(CHIPMUNK_PATH)
LOCAL_C_INCLUDES += $(MXML_PATH)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/lib/LList
LOCAL_C_INCLUDES += $(ZZIP_PATH)


LOCAL_SHARED_LIBRARIES := SDL2
LOCAL_SHARED_LIBRARIES += SDL2_image
LOCAL_SHARED_LIBRARIES += Chipmunk
LOCAL_SHARED_LIBRARIES += mxml
LOCAL_SHARED_LIBRARIES += zzip

# Add your application source files here...
LOCAL_SRC_FILES := lib/SDL2/android/SDL_android_main.cpp \
	$(subst $(LOCAL_PATH)/,,$(wildcard $(LOCAL_PATH)/src/*.c)) \
	lib/LList/llist.c

LOCAL_CFLAGS += -DGL_GLEXT_PROTOTYPES
LOCAL_LDLIBS := -ldl -lGLESv1_CM -lGLESv2 -llog -landroid -lc -lz

include $(BUILD_SHARED_LIBRARY)
