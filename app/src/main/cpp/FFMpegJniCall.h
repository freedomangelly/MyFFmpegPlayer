//
// Created by freed on 2020/1/24.
//

#ifndef MYFFMPEGPLAYER_FFMPEGJNICALL_H
#define MYFFMPEGPLAYER_FFMPEGJNICALL_H

#include <jni.h>

enum ThreadMode{
    THREAD_CHILD,THREAD_MAIN
};

class FFMpegJniCall {
public :
    jobject jAudioTrackOjb;
    jmethodID  jAudioTrackWriteMid;
    JavaVM *javaVM;
    JNIEnv *jniEnv;
    jmethodID  iPlayerErrorMid;
    jobject  jPlayerObj;
public:
    FFMpegJniCall(JavaVM *javaVm ,JNIEnv *jniEnv,jobject jPlayerObj);
    ~FFMpegJniCall();

private:
    void initCreateAudioTrack();

public :
    void callAudioTrackWrite(jbyteArray audioData,int offsetInBytes,int sizeInBytes);

    void callPlayerError(ThreadMode threadMode,int code, char *msg);
};


#endif //MYFFMPEGPLAYER_FFMPEGJNICALL_H
