LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := ffmpegtest
LOCAL_SRC_FILES := draw_yuv.c encode_mp4_to_yuv.c encode_yuv_to_mp4.c
LOCAL_LDLIBS := -llog -ljnigraphics -lz 
LOCAL_SHARED_LIBRARIES := libavfilter libavformat libavcodec libswscale libavutil libswresample

include $(BUILD_SHARED_LIBRARY)
$(call import-module,ffmpeg-3.1.2/android/arm)
