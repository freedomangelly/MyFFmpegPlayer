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
    JavaVM *javaVM;
    JNIEnv *jniEnv;
    jmethodID  jPlayerErrorMid;
    jmethodID  jPlayPrepareMid;
    jobject  jPlayerObj;
public:
    FFMpegJniCall(JavaVM *javaVm ,JNIEnv *jniEnv,jobject jPlayerObj);
    ~FFMpegJniCall();

private:


public :


    void callPlayerError(ThreadMode threadMode,int code, char *msg);

    void callPlayerPrepared(ThreadMode mode);
};


#endif //MYFFMPEGPLAYER_FFMPEGJNICALL_H
