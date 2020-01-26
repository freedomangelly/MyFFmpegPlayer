//
// Created by freed on 2020/1/25.
//

#ifndef MYFFMPEGPLAYER_MYFFMPEG_H
#define MYFFMPEGPLAYER_MYFFMPEG_H

#include <pthread.h>
#include "FFMpegJniCall.h"
#include "FFmpegAudio.h"

extern "C"{
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
};
class MyFFmpeg {
public:
    AVFormatContext *pFormatContext = NULL;
//    AVCodecContext *pCodecContext=NULL;
//    SwrContext *swrContext = NULL;

    uint8_t *resampleOutBuffer=NULL;
    char* url=NULL;
    FFMpegJniCall *jniCall=NULL;
    FFmpegAudio *pAudio=NULL;

public :
    MyFFmpeg(FFMpegJniCall *jniCall ,const char* url);
    ~MyFFmpeg();

public:
    void play();
    void prepare();
    void prepareAsync();
    void prepare(ThreadMode threadMode);
    void callPlayerJniError(ThreadMode threadMode,int code,char* msg);
    void release();
};


#endif //MYFFMPEGPLAYER_MYFFMPEG_H
