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
    iPlayerErrorMid=jniEnv->GetMethodID(jPlayerClass,"onError","(ILjava/lang/String;)V");
}

FFMpegJniCall::~FFMpegJniCall(){
    jniEnv->DeleteGlobalRef(jPlayerObj);
}




void FFMpegJniCall::callPlayerError(ThreadMode  threadMode,int code, char *msg) {
    if(threadMode==THREAD_MAIN){
        jstring jmsg=jniEnv->NewStringUTF(msg);
        jniEnv->CallVoidMethod(jPlayerObj,iPlayerErrorMid,code,jmsg);
        jniEnv->DeleteLocalRef(jmsg);
    } else{
        JNIEnv *env;
        if(javaVM->AttachCurrentThread(&env,0)!=JNI_OK){
            LOGE("get child thread jniEnv error!");
            return;
        }
        jstring jmsg=env->NewStringUTF(msg);
        env->CallVoidMethod(jPlayerObj,iPlayerErrorMid,code,jmsg);
        env->DeleteLocalRef(jmsg);
        javaVM->DetachCurrentThread();
    }

}
