//
// Created by freed on 2020/1/25.
//

#include "FFmpegAudio.h"
#include "MyConsts.h"

FFmpegAudio::FFmpegAudio(int audioStreamIndex, FFMpegJniCall *pJniCall, AVCodecContext *pCodecContext,
            AVFormatContext *pFormatContext){
    this->audioStreamIndex=audioStreamIndex;
    this->pJniCall=pJniCall;
    this->pCodecContext = pCodecContext;
    this->pFormatContext=pFormatContext;
}

void *threadPlay2(void *context){
    FFmpegAudio *pFFmpeg= static_cast<FFmpegAudio *>(context);
    pFFmpeg->initCreateOpenSLES();
    return 0;
}

void FFmpegAudio::play() {
    pthread_t  playThreadT;
    pthread_create(&playThreadT,NULL,threadPlay2,this);
    pthread_detach(playThreadT);
}

void playerCallback(SLAndroidSimpleBufferQueueItf caller, void *pContext);

void FFmpegAudio::initCreateOpenSLES() {
    /*OpenSLES OpenGLES 都是自带的
    XXXES 与 XXX 之间可以说是基本没有区别，区别就是 XXXES 是 XXX 的精简
    而且他们都有一定规则，命名规则 slXXX() , glXXX3f*/
    // 3.1 创建引擎接口对象
    SLObjectItf engineObject = NULL;
    SLEngineItf engineEngine;
    slCreateEngine(&engineObject, 0, NULL, 0, NULL, NULL);
    // realize the engine
    (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    // get the engine interface, which is needed in order to create other objects
    (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineEngine);
    // 3.2 设置混音器
    static SLObjectItf outputMixObject = NULL;
    const SLInterfaceID ids[1] = {SL_IID_ENVIRONMENTALREVERB};
    const SLboolean req[1] = {SL_BOOLEAN_FALSE};
    (*engineEngine)->CreateOutputMix(engineEngine, &outputMixObject, 1, ids, req);
    (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
    SLEnvironmentalReverbItf outputMixEnvironmentalReverb = NULL;
    (*outputMixObject)->GetInterface(outputMixObject, SL_IID_ENVIRONMENTALREVERB,
                                     &outputMixEnvironmentalReverb);
    SLEnvironmentalReverbSettings reverbSettings = SL_I3DL2_ENVIRONMENT_PRESET_STONECORRIDOR;
    (*outputMixEnvironmentalReverb)->SetEnvironmentalReverbProperties(outputMixEnvironmentalReverb,
                                                                      &reverbSettings);
    // 3.3 创建播放器
    SLObjectItf pPlayer = NULL;
    SLPlayItf pPlayItf = NULL;
    SLDataLocator_AndroidSimpleBufferQueue simpleBufferQueue = {
            SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};
    SLDataFormat_PCM formatPcm = {
            SL_DATAFORMAT_PCM,
            2,
            SL_SAMPLINGRATE_44_1,
            SL_PCMSAMPLEFORMAT_FIXED_16,
            SL_PCMSAMPLEFORMAT_FIXED_16,
            SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT,
            SL_BYTEORDER_LITTLEENDIAN};
    SLDataSource audioSrc = {&simpleBufferQueue, &formatPcm};
    SLDataLocator_OutputMix outputMix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
    SLDataSink audioSnk = {&outputMix, NULL};
    SLInterfaceID interfaceIds[3] = {SL_IID_BUFFERQUEUE, SL_IID_VOLUME, SL_IID_PLAYBACKRATE};
    SLboolean interfaceRequired[3] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};
    (*engineEngine)->CreateAudioPlayer(engineEngine, &pPlayer, &audioSrc, &audioSnk, 3,
                                       interfaceIds, interfaceRequired);
    (*pPlayer)->Realize(pPlayer, SL_BOOLEAN_FALSE);
    (*pPlayer)->GetInterface(pPlayer, SL_IID_PLAY, &pPlayItf);
    // 3.4 设置缓存队列和回调函数
    SLAndroidSimpleBufferQueueItf playerBufferQueue;
    (*pPlayer)->GetInterface(pPlayer, SL_IID_BUFFERQUEUE, &playerBufferQueue);
    (*playerBufferQueue)->RegisterCallback(playerBufferQueue, playerCallback, NULL);
    // 3.5 设置播放状态
    (*pPlayItf)->SetPlayState(pPlayItf, SL_PLAYSTATE_PLAYING);
    // 3.6 调用回调函数
    playerCallback(playerBufferQueue, this);
}

void playerCallback(SLAndroidSimpleBufferQueueItf caller, void *pContext) {
    FFmpegAudio *pFFmepg = (FFmpegAudio *) pContext;
    int dataSize = pFFmepg->resampleAudio();
    // 这里为什么报错，留在后面再去解决
    (*caller)->Enqueue(caller, pFFmepg->resampleOutBuffer, dataSize);
}

int FFmpegAudio::resampleAudio() {
    int dataSize=0;
    AVPacket *pPacket=av_packet_alloc();
    AVFrame *pFrame=av_frame_alloc();

    while (av_read_frame(pFormatContext,pPacket)>=0){
        if(pPacket->stream_index==audioStreamIndex){
            //pPacket 包，压缩的数据，解码成pcm数据
            int codecSendPacketRes=avcodec_send_packet(pCodecContext,pPacket);
            if(codecSendPacketRes==0){
                int codecReceiveFrameRes=avcodec_receive_frame(pCodecContext,pFrame);
                if(codecReceiveFrameRes==0){
                    LOGI("解码音频帧");
                    //调用重采样的方法
                    dataSize=swr_convert(swrContext, &resampleOutBuffer, pFrame->nb_samples,
                                         (const uint8_t **) (pFrame->data), pFrame->nb_samples);
                    LOGI("解码音频帧");
                    break;
                }
            }

        }
        av_packet_unref(pPacket);
        av_frame_unref(pFrame);
    }
    av_packet_free(&pPacket);
    av_frame_free(&pFrame);
    return dataSize;
}


void FFmpegAudio::initCreateAudioTrack(JNIEnv *env) {
    //public AudioTrack(int streamType, int sampleRateInHz, int channelConfig, int audioFormat,
    //            int bufferSizeInBytes, int mode)
    jclass jAudioTrackClass = env->FindClass("android/media/AudioTrack");
    jmethodID jAudioTackCMid = env->GetMethodID(jAudioTrackClass, "<init>", "(IIIIII)V");
    //public static final int STREAM_MUSIC = AudioSystem.STREAM_MUSIC;
    jclass jAudioManager = env->FindClass("android/media/AudioManager");
    jfieldID jfieldId=env->GetStaticFieldID(jAudioManager, "STREAM_MUSIC", "I");
    int type = env->GetStaticIntField(jAudioManager,jfieldId);

    int streamType = type;
    int sampleRateInHz = AUDIO_SAMPLE_RATE;
    int channelConfig = (0x4 | 0x8);
    int audioFormat = 2;
    jmethodID jWriteMid;
    int mode = 1;//短声音用0 长生意用1
    //static public int getMinBufferSize(int sampleRateInHz, int channelConfig, int audioFormat)
    jmethodID getMinBufferSizeMid = env->GetStaticMethodID(jAudioTrackClass, "getMinBufferSize",
                                                              "(III)I");
    int bufferSizeInBytes = env->CallStaticIntMethod(jAudioTrackClass, getMinBufferSizeMid,
                                                        sampleRateInHz,
                                                        channelConfig, audioFormat);
    LOGI("bufferSizeInbytes = %d", bufferSizeInBytes);
    jAudioTrackOjb = env->NewObject(jAudioTrackClass, jAudioTackCMid, streamType,
                                       sampleRateInHz, channelConfig, audioFormat,
                                       bufferSizeInBytes, mode);

    //play
    jmethodID playMid = env->GetMethodID(jAudioTrackClass, "play", "()V");
    env->CallVoidMethod(jAudioTrackOjb, playMid);

    //write method

    jAudioTrackWriteMid = env->GetMethodID(jAudioTrackClass, "write", "([BII)I");
}

void
FFmpegAudio::callAudioTrackWrite(JNIEnv *env,jbyteArray audioData, int offsetInBytes, int sizeInBytes) {
    env->CallIntMethod(jAudioTrackOjb,jAudioTrackWriteMid,audioData,offsetInBytes,sizeInBytes);
}

FFmpegAudio::~FFmpegAudio() {
}

//void FFmpegAudio::PlayAudioTack(){
//    LOGI("1111111111111111111111120");
//    // size 是播放指定的大小，是最终输出的大小
//    int outChannels = av_get_channel_layout_nb_channels(out_ch_layout);
//    LOGI("11111111111111111111111201");
//    int dataSize = av_samples_get_buffer_size(NULL, outChannels, pCodecParameters->frame_size,
//                                              out_sample_fmt, 0);
//    LOGI("11111111111111111111111202");
//    uint8_t *resampleOutBuffer = (uint8_t *) malloc(dataSize);
//    // ---------- 重采样 end ----------
//    LOGI("11111111111111111111111203");//jniEnv主线程 现在是子线程
//
//    JNIEnv* env;
//    int status;
//    bool isAttach = false;
//    JavaVM *javaVms=NULL;
//    LOGI("111111111111111111111112031");
//    if(threadMode==THREAD_CHILD){
//        LOGI("1111111111111111111111120312");
//        javaVms=jniCall->javaVM;
//        LOGI("1111111111111111111111120313");
//        status = javaVms->AttachCurrentThread(&env, 0);
//        LOGI("111111111111111111111112032 %d",status);
//        if (status < 0) {
//            LOGE("MediaRenderer::DoJavaCallback Failed: %d", status);
//            return ;
//        }
//        isAttach = true;
//        javaVms->DetachCurrentThread();
//    }
//    LOGI("11111111111111111111111204 %d",isAttach);
//    jbyteArray jPcmByteArray = NULL;
//    if(isAttach){
//        jPcmByteArray = jniCall->jniEnv->NewByteArray(dataSize);
//    } else{
//        jPcmByteArray = env->NewByteArray(dataSize);
//    }
//    // native 创建 c 数组
//    LOGI("11111111111111111111111204");
//    jbyte *jPcmData = jniCall->jniEnv->GetByteArrayElements(jPcmByteArray, NULL);
//    LOGI("11111111111111111111111205");
//    pPacket = av_packet_alloc();
//    pFrame = av_frame_alloc();
//    LOGI("11111111111111111111111121");
//    while (av_read_frame(pFormatContext, pPacket) >= 0) {
//        LOGI("111111111111111111111111211");
//        if (pPacket->stream_index == audioStramIndex) {
//            LOGI("111111111111111111111111212");
//            // Packet 包，压缩的数据，解码成 pcm 数据
//            int codecSendPacketRes = avcodec_send_packet(pCodecContext, pPacket);
//            if (codecSendPacketRes == 0) {
//                LOGI("111111111111111111111111213");
//                int codecReceiveFrameRes = avcodec_receive_frame(pCodecContext, pFrame);
//                if (codecReceiveFrameRes == 0) {
//                    LOGI("111111111111111111111111214");
//                    // AVPacket -> AVFrame
//                    index++;
//                    LOGE("解码第 %d 帧", index);
//
//                    // 调用重采样的方法
//                    swr_convert(swrContext, &resampleOutBuffer, pFrame->nb_samples,
//                                (const uint8_t **) pFrame->data, pFrame->nb_samples);
//                    LOGI("111111111111111111111111215");
//                    // write 写到缓冲区 pFrame.data -> javabyte
//                    // size 是多大，装 pcm 的数据
//                    // 1s 44100 点  2通道 ，2字节    44100*2*2
//                    // 1帧不是一秒，pFrame->nb_samples点
//                    memcpy(jPcmData, resampleOutBuffer, dataSize);
//                    LOGI("111111111111111111111111216");
//                    // 0 把 c 的数组的数据同步到 jbyteArray , 然后释放native数组
//                    jniCall->jniEnv->ReleaseByteArrayElements(jPcmByteArray, jPcmData, JNI_COMMIT);
//                    LOGI("111111111111111111111111217");
//                    // TODO
//                    jniCall->callAudioTrackWrite(jPcmByteArray, 0, dataSize);
//                    LOGI("111111111111111111111111218");
//                }
//            }
//        }
//        // 解引用
//        av_packet_unref(pPacket);
//        av_frame_unref(pFrame);
//    }
//    LOGI("1111111111111111111111122");
//
//    // 1. 解引用数据 data ， 2. 销毁 pPacket 结构体内存  3. pPacket = NULL
//    av_packet_free(&pPacket);
//    av_frame_free(&pFrame);
//    // 解除 jPcmDataArray 的持有，让 javaGC 回收
//    jniCall->jniEnv->ReleaseByteArrayElements(jPcmByteArray, jPcmData, 0);
//    jniCall->jniEnv->DeleteLocalRef(jPcmByteArray);
//}


