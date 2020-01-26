#include <jni.h>
#include "MyConsts.h"
#include "com_liuy_myffmpegplayer_ffmpegplayer_MediaPlayerJNI.h"
#include "FFMpegJniCall.h"
#include "MyFFmpeg.h"

// 在 c++ 中采用 c 的这种编译方式
extern "C" {
#include "libavformat/avformat.h"
#include "libswresample/swresample.h"
}

FFMpegJniCall *pJniCall;
MyFFmpeg *pFFmpeg;

JavaVM *pJavaVM = NULL;

// 重写 so 被加载时会调用的一个方法
// 小作业，去了解动态注册
extern "C" JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *javaVM, void *reserved) {
    LOGI("JNI_OnLoad");
    pJavaVM = javaVM;
    JNIEnv *env;
    if (javaVM->GetEnv((void **) &env, JNI_VERSION_1_4) != JNI_OK) {
        return -1;
    }
    return JNI_VERSION_1_4;
}

extern "C" JNIEXPORT void JNICALL
Java_com_liuy_myffmpegplayer_ffmpegplayer_MediaPlayerJNI_nPlay(JNIEnv *env, jobject instance) {
    LOGI("nPlay");
    if (pFFmpeg != NULL) {
        LOGI("nPlay1");
        pFFmpeg->play();
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_liuy_myffmpegplayer_ffmpegplayer_MediaPlayerJNI_nPrepareAsync(JNIEnv *env,
                                                                       jobject instance,
                                                                       jstring url_) {
    LOGI("nPrepareAsync");
    const char *url = env->GetStringUTFChars(url_, 0);
    if (pFFmpeg == NULL) {
        pJniCall = new FFMpegJniCall(pJavaVM, env, instance);
        pFFmpeg = new MyFFmpeg(pJniCall, url);
        pFFmpeg->prepare();
    }
    env->ReleaseStringUTFChars(url_, url);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_liuy_myffmpegplayer_ffmpegplayer_MediaPlayerJNI_nPrepare(JNIEnv *env, jobject instance,
                                                                  jstring url_) {
    LOGI("nPrepare");
    const char *url = env->GetStringUTFChars(url_, 0);
    if (pFFmpeg == NULL) {
        pJniCall = new FFMpegJniCall(pJavaVM, env, instance);
        pFFmpeg = new MyFFmpeg(pJniCall, url);
        pFFmpeg->prepareAsync();
    }
    env->ReleaseStringUTFChars(url_, url);
}
extern "C"
JNIEXPORT void JNICALL Java_com_liuy_myffmpegplayer_ffmpegplayer_MediaPlayerJNI_nPlay2
        (JNIEnv *env, jobject instance, jstring urlSrc) {
    LOGI("nPlay2");
//    player(env, instance, url);
    pJniCall=new FFMpegJniCall(pJavaVM,env,instance);
    const char *url=env->GetStringUTFChars(urlSrc,0);
    LOGI("enter jni Play2 %s",url);
    pFFmpeg=new MyFFmpeg(pJniCall,url);
    pFFmpeg->play();
    env->ReleaseStringUTFChars(urlSrc,url);

}