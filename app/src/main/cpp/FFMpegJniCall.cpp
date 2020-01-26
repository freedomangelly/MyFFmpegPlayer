//
// Created by freed on 2020/1/24.
//

#include "FFMpegJniCall.h"
#include "MyConsts.h"

FFMpegJniCall::FFMpegJniCall(JavaVM *javaVm, JNIEnv *jniEnv , jobject jPlayerObj) {
    this->javaVM=javaVm;
    this->jniEnv=jniEnv;
    this->jPlayerObj=jniEnv->NewGlobalRef(jPlayerObj);

    jclass jPlayerClass=jniEnv->GetObjectClass(jPlayerObj);
    jPlayerErrorMid=jniEnv->GetMethodID(jPlayerClass,"onError","(ILjava/lang/String;)V");
    jPlayPrepareMid=jniEnv->GetMethodID(jPlayerClass,"onPrepared","()V");
}

FFMpegJniCall::~FFMpegJniCall(){
    jniEnv->DeleteGlobalRef(jPlayerObj);
}




void FFMpegJniCall::callPlayerError(ThreadMode  threadMode,int code, char *msg) {
    if(threadMode==THREAD_MAIN){
        jstring jmsg=jniEnv->NewStringUTF(msg);
        jniEnv->CallVoidMethod(jPlayerObj,jPlayerErrorMid,code,jmsg);
        jniEnv->DeleteLocalRef(jmsg);
    } else{
        JNIEnv *env;
        if(javaVM->AttachCurrentThread(&env,0)!=JNI_OK){
            LOGE("get child thread jniEnv error!");
            return;
        }
        jstring jmsg=env->NewStringUTF(msg);
        env->CallVoidMethod(jPlayerObj,jPlayerErrorMid,code,jmsg);
        env->DeleteLocalRef(jmsg);
        javaVM->DetachCurrentThread();
    }

}

void FFMpegJniCall::callPlayerPrepared(ThreadMode mode) {
    LOGI("111111111111111111111111callPlayerPrepared1");
    //子线程用不了主线程jniEnv（native线程）
    //子线程是不共享jniEnv，他们有自己所独有的
    if(mode==THREAD_MAIN){
        LOGI("111111111111111111111111callPlayerPrepared2");
        jniEnv->CallVoidMethod(jPlayerObj,jPlayPrepareMid);
    } else {
        LOGI("111111111111111111111111callPlayerPrepared3");
        JNIEnv *env;
        if (javaVM->AttachCurrentThread(&env, 0) != JNI_OK) {
            LOGE("get child thread jniEnv error!");
            return;
        }
        env->CallVoidMethod(jPlayerObj, jPlayPrepareMid);
        javaVM->DetachCurrentThread();
    }
}
