//
// Created by freed on 2020/1/25.
//

#ifndef MYFFMPEGPLAYER_FFMPEGAUDIO_H
#define MYFFMPEGPLAYER_FFMPEGAUDIO_H

#include <pthread.h>

extern "C"{
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
};

#include "MyConsts.h"
#include "FFMpegJniCall.h"
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

class FFmpegAudio {
public:
    jobject jAudioTrackOjb;
    jmethodID  jAudioTrackWriteMid;
    AVFormatContext *pFormatContext=NULL;
    AVCodecContext *pCodecContext=NULL;
    SwrContext *swrContext=NULL;
    uint8_t  *resampleOutBuffer=NULL;
    FFMpegJniCall *pJniCall=NULL;
    int audioStreamIndex=-1;

public:
    FFmpegAudio(int audioStreamIndex, FFMpegJniCall *pJniCall, AVCodecContext *pCodecContext,
            AVFormatContext *pFormatContext);
    ~FFmpegAudio();
//    ~FFmpegAudio(JNIEnv *env);
    void play();

    void initCreateOpenSLES();
    void initCreateAudioTrack(JNIEnv *env);
    void callAudioTrackWrite(JNIEnv *env,jbyteArray audioData,int offsetInBytes,int sizeInBytes);
    int resampleAudio();
private:
//    void FFmpegAudio::PlayAudioTack();

};


#endif //MYFFMPEGPLAYER_FFMPEGAUDIO_H
