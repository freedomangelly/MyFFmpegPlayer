//
// Created by freed on 2020/1/25.
//

#ifndef MYFFMPEGPLAYER_MYCONSTS_H
#define MYFFMPEGPLAYER_MYCONSTS_H

#include <android/log.h>
#define TAG "MY_FFMPEG_TAG"
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,TAG,__VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,TAG,__VA_ARGS__)

#define AUDIO_SAMPLE_RATE 44100

//播放错误码
#define FIND_STREAM_ERROR_CODE -0x10
#define CODEC_FIND_DECODER_ERROR_CODE -0x11
#define CODEC_ALLOC_CONTEXT_ERROR_CODE -0x12
#define SWR_ALLOC_SET_OPTS_ERROR_CODE -0x13
#define SWR_CONTEXT_INIT_ERROR_CODE -0x14
//播放错误码结束
#endif //MYFFMPEGPLAYER_MYCONSTS_H
