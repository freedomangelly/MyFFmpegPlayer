//
// Created by freed on 2020/1/24.
//

#include "FFMpegJniCall.h"
#include "MyConsts.h"

FFMpegJniCall::FFMpegJniCall(JavaVM *javaVm, JNIEnv *jniEnv , jobject jPlayerObj) {
    this->javaVM=javaVm;
    this->jniEnv=jniEnv;
    this->jPlayerObj=jPlayerObj;
    initCreateAudioTrack();

    jclass jPlayerClass=jniEnv->GetObjectClass(jPlayerObj);
    iPlayerErrorMid=jniEnv->GetMethodID(jPlayerClass,"onError","(ILjava/lang/String;)V");
}

FFMpegJniCall::~FFMpegJniCall(){
    jniEnv->DeleteLocalRef(jAudioTrackOjb);
    jniEnv->DeleteGlobalRef(jPlayerObj);
}

void FFMpegJniCall::initCreateAudioTrack() {
    //public AudioTrack(int streamType, int sampleRateInHz, int channelConfig, int audioFormat,
    //            int bufferSizeInBytes, int mode)
    jclass jAudioTrackClass = jniEnv->FindClass("android/media/AudioTrack");
    jmethodID jAudioTackCMid = jniEnv->GetMethodID(jAudioTrackClass, "<init>", "(IIIIII)V");
    //public static final int STREAM_MUSIC = AudioSystem.STREAM_MUSIC;
    jclass jAudioManager = jniEnv->FindClass("android/media/AudioManager");
    jfieldID jfieldId=jniEnv->GetStaticFieldID(jAudioManager, "STREAM_MUSIC", "I");
    int type = jniEnv->GetStaticIntField(jAudioManager,jfieldId);

    int streamType = type;
    int sampleRateInHz = AUDIO_SAMPLE_RATE;
    int channelConfig = (0x4 | 0x8);
    int audioFormat = 2;
    jmethodID jWriteMid;
    int mode = 1;//短声音用0 长生意用1
    //static public int getMinBufferSize(int sampleRateInHz, int channelConfig, int audioFormat)
    jmethodID getMinBufferSizeMid = jniEnv->GetStaticMethodID(jAudioTrackClass, "getMinBufferSize",
                                                           "(III)I");
    int bufferSizeInBytes = jniEnv->CallStaticIntMethod(jAudioTrackClass, getMinBufferSizeMid,
                                                     sampleRateInHz,
                                                     channelConfig, audioFormat);
    LOGI("bufferSizeInbytes = %d", bufferSizeInBytes);
    jAudioTrackOjb = jniEnv->NewObject(jAudioTrackClass, jAudioTackCMid, streamType,
                                            sampleRateInHz, channelConfig, audioFormat,
                                            bufferSizeInBytes, mode);

    //play
    jmethodID playMid = jniEnv->GetMethodID(jAudioTrackClass, "play", "()V");
    jniEnv->CallVoidMethod(jAudioTrackOjb, playMid);

    //write method

    jAudioTrackWriteMid = jniEnv->GetMethodID(jAudioTrackClass, "write", "([BII)I");
}

void
FFMpegJniCall::callAudioTrackWrite(jbyteArray audioData, int offsetInBytes, int sizeInBytes) {
    jniEnv->CallIntMethod(jAudioTrackOjb,jAudioTrackWriteMid,audioData,offsetInBytes,sizeInBytes);
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
