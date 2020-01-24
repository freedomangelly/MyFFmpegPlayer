#include <jni.h>
#include "DZJNICall.h"
#include "DZFFmpeg.h"
#include "DZConstDefine.h"
#include "com_liuy_myffmpegplayer_ffmpegplayer_MediaPlayerJNI.h"

// 在 c++ 中采用 c 的这种编译方式
extern "C" {
#include "libavformat/avformat.h"
#include "libswresample/swresample.h"
}

DZJNICall *pJniCall;
DZFFmpeg *pFFmpeg;

JavaVM *pJavaVM = NULL;

jobject initCreateAudioTrack(JNIEnv *env) {
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
    int mode = 1;//短声音用0 长生意用1
    //static public int getMinBufferSize(int sampleRateInHz, int channelConfig, int audioFormat)
    jmethodID getMinBufferSizeMid = env->GetStaticMethodID(jAudioTrackClass, "getMinBufferSize",
                                                           "(III)I");
    int bufferSizeInBytes = env->CallStaticIntMethod(jAudioTrackClass, getMinBufferSizeMid,
                                                      sampleRateInHz,
                                                     channelConfig, audioFormat);
    LOGI("bufferSizeInbytes = %d", bufferSizeInBytes);
    jobject jAudioTrackObj = env->NewObject(jAudioTrackClass, jAudioTackCMid, streamType,
                                            sampleRateInHz, channelConfig, audioFormat,
                                            bufferSizeInBytes, mode);

    //play
    jmethodID playMid = env->GetMethodID(jAudioTrackClass, "play", "()V");
    env->CallVoidMethod(jAudioTrackObj, playMid);
    return jAudioTrackObj;
}

void player(JNIEnv *env, jobject instance, jstring url_) {
    const char *url = env->GetStringUTFChars(url_, 0);
    //1初始化一些组件
    av_register_all();
    avformat_network_init();

    AVFormatContext *pFormatContext = NULL;
    AVCodecContext *pCodecContext = NULL;
    //2打开视频文件
    int formatOpenInputRes ;
    int find_stream_info = 0;
    int audioStreamIndex = 0;
    int codecParametersToContextRes = -1;
    int codecOpenRes = 0;
    AVCodecParameters *pCodecParameters = NULL;
    jmethodID jWriteMid;
    jobject jAudioTrackObj;
    jclass jAudioTrackClass;
    AVCodec *pCodec = NULL;
    int index;
    AVPacket *pPacket;
    AVFrame *pFrame;
    AVCodecID id;

    LOGI("11111111111111111111");
    formatOpenInputRes = avformat_open_input(&pFormatContext, url, NULL, NULL);
    if (formatOpenInputRes != 0) {
        //打开资源文件失败
        //第一件事，需要会调给java层
        //第二件事，需要释放资源
        return;
    }
    LOGI("11111111111111111112");
    //3获取视频的信息
    find_stream_info = avformat_find_stream_info(pFormatContext, NULL);
    if (find_stream_info < 0) {
        LOGI("format find stream info error: %s", av_err2str(find_stream_info));
        return;
    }
    LOGI("11111111111111111113");
    //4查找流的解码器，找到视频流
    audioStreamIndex = av_find_best_stream(pFormatContext, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
    if (audioStreamIndex < 0) {
        return;
    }
    LOGI("11111111111111111114");
    pCodecParameters = pFormatContext->streams[audioStreamIndex]->codecpar;
    id = pCodecParameters->codec_id;
    pCodec = avcodec_find_decoder(id);
    if (pCodec == NULL) {
        return;
    }
    LOGI("11111111111111111115");
    pCodecContext = avcodec_alloc_context3(pCodec);
    if (pCodecContext == NULL) {
        return;
    }
    LOGI("11111111111111111116");
    codecParametersToContextRes = avcodec_parameters_to_context(pCodecContext, pCodecParameters);
    if (codecParametersToContextRes < 0) {
        return;
    }

    LOGI("11111111111111111117");
    codecOpenRes = avcodec_open2(pCodecContext, pCodec, NULL);
    if (codecOpenRes != 0) {
        return;
    }

    LOGI("22222222222222222222222222");
    pPacket = av_packet_alloc();
    pFrame = av_frame_alloc();

    jAudioTrackClass = env->FindClass("android/media/AudioTrack");
    jWriteMid = env->GetMethodID(jAudioTrackClass, "write", "([BII)I");
    jAudioTrackObj = initCreateAudioTrack(env);

    //重新采样
    int64_t out_ch_layou = AV_CH_LAYOUT_STEREO;//输出的通道

    enum AVSampleFormat out_sample_fmt=AVSampleFormat ::AV_SAMPLE_FMT_S16;//输出的格式
     int out_sample_rate = AUDIO_SAMPLE_RATE;//输出的频率
    int64_t  in_ch_layout = pCodecContext->channel_layout; //输入的通道
    enum AVSampleFormat  in_sample_fmt=pCodecContext->sample_fmt;
    int  in_sample_rate=pCodecContext->sample_rate;
    SwrContext *swrContext=swr_alloc_set_opts(NULL,out_ch_layou,out_sample_fmt,out_sample_rate,in_ch_layout,in_sample_fmt,in_sample_rate,0,NULL);
    swr_init(swrContext);
    if(swrContext == NULL){
        //提示错误
        return;
    }
    int swrInitRes=swr_init(swrContext);
    if(swrInitRes<0){
        return;
    }

//size是播放指定的大小，是最终输出的大小
    int outChannels=av_get_channel_layout_nb_channels(out_ch_layou);

    int dataSize = av_samples_get_buffer_size(NULL, outChannels,
                                              pCodecParameters->frame_size,
                                              pCodecContext->sample_fmt, 0);
    uint8_t *resampleOutBuffer= static_cast<uint8_t *>(malloc(dataSize));

    jbyteArray jPcmByteArray = env->NewByteArray(dataSize);
    jbyte *jPcmData = env->GetByteArrayElements(jPcmByteArray, NULL);
    LOGI("33333333333333333333333333333");
    while (av_read_frame(pFormatContext, pPacket) <= 0) {
        if (pPacket->stream_index == audioStreamIndex) {
            //packet 包，压缩的数据，解码成pcm数据
            int codecSendPacketRes = avcodec_send_packet(pCodecContext, pPacket);
            if (codecSendPacketRes == 0) {
                int codeReceiverFrame = avcodec_receive_frame(pCodecContext, pFrame);
                if (codeReceiverFrame == 0) {
                    index++;
                    LOGI("解码第%d帧", index);
                    //调用重采样的方法
                    swr_convert(swrContext, &resampleOutBuffer, pFrame->nb_samples,
                                (const uint8_t **)(pFrame->data), pFrame->nb_samples);

                    // write 写到缓冲区 pFrame.data -> javabyte
                    // size 是多大，装 pcm 的数据
                    // 1s 44100 点  2通道 ，2字节    44100*2*2
                    // 1帧不是一秒，pFrame->nb_samples点


                    memcpy(jPcmData, resampleOutBuffer, dataSize);
                    env->ReleaseByteArrayElements(jPcmByteArray, jPcmData, JNI_COMMIT);
                    env->CallIntMethod(jAudioTrackObj, jWriteMid, jPcmByteArray, 0, dataSize);


                }
            }
        }

        av_packet_unref(pPacket);
        av_frame_unref(pFrame);
    }

    //解引用 数据data 2,销毁pPacket结构体内存 3.pPacket=NULL
    av_packet_free(&pPacket);
    av_frame_free(&pFrame);
    env->DeleteLocalRef(jAudioTrackObj);
    env->DeleteLocalRef(jPcmByteArray);
    env->ReleaseByteArrayElements(jPcmByteArray,jPcmData,0);
    __av_resources_destroy:
    LOGI("444444444444444444444444");
    if (pCodecContext != NULL) {
        avcodec_close(pCodecContext);
        avcodec_free_context(&pCodecContext);
        pCodecContext = NULL;
    }
    if (pFormatContext != NULL) {
        avformat_close_input(&pFormatContext);
        avformat_free_context(pFormatContext);
        pFormatContext = NULL;
    }

    avformat_network_deinit();
    env->ReleaseStringUTFChars(url_, url);
}

// 重写 so 被加载时会调用的一个方法
// 小作业，去了解动态注册
extern "C" JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *javaVM, void *reserved) {
    pJavaVM = javaVM;
    JNIEnv *env;
    if (javaVM->GetEnv((void **) &env, JNI_VERSION_1_4) != JNI_OK) {
        return -1;
    }
    return JNI_VERSION_1_4;
}

extern "C" JNIEXPORT void JNICALL
Java_com_liuy_myffmpegplayer_ffmpegplayer_MediaPlayerJNI_nPlay(JNIEnv *env, jobject instance) {
    if (pFFmpeg != NULL) {
        pFFmpeg->play();
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_liuy_myffmpegplayer_ffmpegplayer_MediaPlayerJNI_nPrepareAsync(JNIEnv *env,
                                                                       jobject instance,
                                                                       jstring url_) {
    const char *url = env->GetStringUTFChars(url_, 0);
    if (pFFmpeg == NULL) {
        pJniCall = new DZJNICall(pJavaVM, env, instance);
        pFFmpeg = new DZFFmpeg(pJniCall, url);
        pFFmpeg->prepare();
    }
    env->ReleaseStringUTFChars(url_, url);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_liuy_myffmpegplayer_ffmpegplayer_MediaPlayerJNI_nPrepare(JNIEnv *env, jobject instance,
                                                                  jstring url_) {
    const char *url = env->GetStringUTFChars(url_, 0);
    if (pFFmpeg == NULL) {
        pJniCall = new DZJNICall(pJavaVM, env, instance);
        pFFmpeg = new DZFFmpeg(pJniCall, url);
        pFFmpeg->prepareAsync();
    }
    env->ReleaseStringUTFChars(url_, url);
}
extern "C"
JNIEXPORT void JNICALL Java_com_liuy_myffmpegplayer_ffmpegplayer_MediaPlayerJNI_nPlay2
        (JNIEnv *env, jobject instance, jstring url) {
    LOGI("enter jni Play2");
    player(env, instance, url);

}