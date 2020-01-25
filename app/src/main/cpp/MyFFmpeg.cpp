//
// Created by freed on 2020/1/25.
//

#include "MyFFmpeg.h"
#include "MyConsts.h"

MyFFmpeg::MyFFmpeg(FFMpegJniCall *jniCall, const char *url) {
    this->jniCall=jniCall;
//    this->url=url;
    this->url= static_cast<const char *>(malloc(strlen(url) + 1));
    LOGI("enter MyFFmpeg Play2 %s",url);
    memcpy((void *) this->url, url, strlen(url) + 1);
}

MyFFmpeg::~MyFFmpeg() {
    release();
}

void *threadPlay(void *context){
    MyFFmpeg *pFFmpeg= (MyFFmpeg *)(context);
    pFFmpeg->prepare(THREAD_CHILD);
    return 0;
}

void MyFFmpeg::play() {
//    LOGI("enter play %s",url);
    pthread_t playThreadT;
    pthread_create(&playThreadT,NULL,threadPlay,this);
    pthread_detach(playThreadT);
//    MyFFmpeg *pFFmpeg= (MyFFmpeg *)(this);
//    pFFmpeg->prepare();
}

void MyFFmpeg::prepare(ThreadMode threadMode) {


    av_register_all();
    avformat_network_init();

    int formatOpenInputRes = 0;
    int formatFindStreamInfoRes = 0;
    int audioStramIndex = -1;
    AVCodecParameters *pCodecParameters;
    AVCodec *pCodec = NULL;
    int codecParametersToContextRes = -1;
    int codecOpenRes = -1;
    int index = 0;
    AVPacket *pPacket = NULL;
    AVFrame *pFrame = NULL;
    LOGE(" url= %s", this->url);
//    const char*url2="/storage/emulated/0/ental.mp3";
    LOGI("1111111111111111111111111");
    formatOpenInputRes = avformat_open_input(&pFormatContext, url, NULL, NULL);
    if (formatOpenInputRes != 0) {
        // 第一件事，需要回调给 Java 层(下次课讲)
        // 第二件事，需要释放资源
        LOGE("format open input error: %s == %d", av_err2str(formatOpenInputRes),formatOpenInputRes);
        LOGE("format open input error: url= %s", url);
        callPlayerJniError(threadMode,formatOpenInputRes, av_err2str(formatOpenInputRes));
        return;
    }
    LOGI("1111111111111111111111112");
    formatFindStreamInfoRes = avformat_find_stream_info(pFormatContext, NULL);
    if (formatFindStreamInfoRes < 0) {
        LOGE("format find stream info error: %s", av_err2str(formatFindStreamInfoRes));
        // 这种方式一般不推荐这么写，但是的确方便
        callPlayerJniError(threadMode,formatFindStreamInfoRes, av_err2str(formatFindStreamInfoRes));
        return;
    }
    LOGI("1111111111111111111111113");
    // 查找音频流的 index
    audioStramIndex = av_find_best_stream(pFormatContext, AVMediaType::AVMEDIA_TYPE_AUDIO, -1, -1,
                                          NULL, 0);
    if (audioStramIndex < 0) {
        LOGE("format audio stream error: %s");
        // 这种方式一般不推荐这么写，但是的确方便
        callPlayerJniError(threadMode,FIND_STREAM_ERROR_CODE, "format audio stream error");
        return;
    }
    LOGI("1111111111111111111111114");
    // 查找解码
    pCodecParameters = pFormatContext->streams[audioStramIndex]->codecpar;
    pCodec = avcodec_find_decoder(pCodecParameters->codec_id);
    if (pCodec == NULL) {
        LOGE("codec find audio decoder error");
        // 这种方式一般不推荐这么写，但是的确方便
        callPlayerJniError(threadMode,CODEC_FIND_DECODER_ERROR_CODE, "codec find audio decoder error");
        return;
    }
    LOGI("1111111111111111111111115");
    // 打开解码器
    pCodecContext = avcodec_alloc_context3(pCodec);
    if (pCodecContext == NULL) {
        LOGE("codec alloc context error");
        // 这种方式一般不推荐这么写，但是的确方便
        callPlayerJniError(threadMode,CODEC_ALLOC_CONTEXT_ERROR_CODE, "codec alloc context error");
        return;
    }
    LOGI("1111111111111111111111116");
    codecParametersToContextRes = avcodec_parameters_to_context(pCodecContext, pCodecParameters);
    if (codecParametersToContextRes < 0) {
        LOGE("codec parameters to context error: %s", av_err2str(codecParametersToContextRes));
        callPlayerJniError(threadMode,codecParametersToContextRes, av_err2str(codecParametersToContextRes));
        return;
    }
    LOGI("1111111111111111111111117");
    codecOpenRes = avcodec_open2(pCodecContext, pCodec, NULL);
    if (codecOpenRes != 0) {
        LOGI("codec audio open error: %s", av_err2str(codecOpenRes));
        callPlayerJniError(threadMode,codecOpenRes, av_err2str(codecOpenRes));
        return;
    }
    LOGI("1111111111111111111111118");
    // ---------- 重采样 start ----------
    int64_t out_ch_layout = AV_CH_LAYOUT_STEREO;
    enum AVSampleFormat out_sample_fmt = AVSampleFormat::AV_SAMPLE_FMT_S16;
    int out_sample_rate = AUDIO_SAMPLE_RATE;
    int64_t in_ch_layout = pCodecContext->channel_layout;
    enum AVSampleFormat in_sample_fmt = pCodecContext->sample_fmt;
    int in_sample_rate = pCodecContext->sample_rate;
    swrContext = swr_alloc_set_opts(NULL, out_ch_layout, out_sample_fmt,
                                    out_sample_rate, in_ch_layout, in_sample_fmt, in_sample_rate, 0, NULL);
    if (swrContext == NULL) {
        // 提示错误
        callPlayerJniError(threadMode,SWR_ALLOC_SET_OPTS_ERROR_CODE, "swr alloc set opts error");
        return;
    }
    LOGI("1111111111111111111111119");
    int swrInitRes = swr_init(swrContext);
    if (swrInitRes < 0) {
        callPlayerJniError(threadMode,SWR_CONTEXT_INIT_ERROR_CODE, "swr context swr init error");
        return;
    }
    LOGI("1111111111111111111111120");
    // size 是播放指定的大小，是最终输出的大小
    int outChannels = av_get_channel_layout_nb_channels(out_ch_layout);
    LOGI("11111111111111111111111201");
    int dataSize = av_samples_get_buffer_size(NULL, outChannels, pCodecParameters->frame_size,
                                              out_sample_fmt, 0);
    LOGI("11111111111111111111111202");
    uint8_t *resampleOutBuffer = (uint8_t *) malloc(dataSize);
    // ---------- 重采样 end ----------
    LOGI("11111111111111111111111203");//jniEnv主线程 现在是子线程

    JNIEnv* env;
    int status;
    bool isAttach = false;
    JavaVM *javaVms=NULL;
            LOGI("111111111111111111111112031");
    if(threadMode==THREAD_CHILD){
        LOGI("1111111111111111111111120312");
        javaVms=jniCall->javaVM;
        LOGI("1111111111111111111111120313");
        status = javaVms->AttachCurrentThread(&env, 0);
        LOGI("111111111111111111111112032 %d",status);
        if (status < 0) {
                LOGE("MediaRenderer::DoJavaCallback Failed: %d", status);
                return ;
            }
            isAttach = true;
        javaVms->DetachCurrentThread();
    }
    LOGI("11111111111111111111111204 %d",isAttach);
    jbyteArray jPcmByteArray = NULL;
    if(isAttach){
        jPcmByteArray = jniCall->jniEnv->NewByteArray(dataSize);
    } else{
        jPcmByteArray = env->NewByteArray(dataSize);
    }
    // native 创建 c 数组
    LOGI("11111111111111111111111204");
    jbyte *jPcmData = jniCall->jniEnv->GetByteArrayElements(jPcmByteArray, NULL);
    LOGI("11111111111111111111111205");
    pPacket = av_packet_alloc();
    pFrame = av_frame_alloc();
    LOGI("11111111111111111111111121");
    while (av_read_frame(pFormatContext, pPacket) >= 0) {
        LOGI("111111111111111111111111211");
        if (pPacket->stream_index == audioStramIndex) {
            LOGI("111111111111111111111111212");
            // Packet 包，压缩的数据，解码成 pcm 数据
            int codecSendPacketRes = avcodec_send_packet(pCodecContext, pPacket);
            if (codecSendPacketRes == 0) {
                LOGI("111111111111111111111111213");
                int codecReceiveFrameRes = avcodec_receive_frame(pCodecContext, pFrame);
                if (codecReceiveFrameRes == 0) {
                    LOGI("111111111111111111111111214");
                    // AVPacket -> AVFrame
                    index++;
                    LOGE("解码第 %d 帧", index);

                    // 调用重采样的方法
                    swr_convert(swrContext, &resampleOutBuffer, pFrame->nb_samples,
                                (const uint8_t **) pFrame->data, pFrame->nb_samples);
                    LOGI("111111111111111111111111215");
                    // write 写到缓冲区 pFrame.data -> javabyte
                    // size 是多大，装 pcm 的数据
                    // 1s 44100 点  2通道 ，2字节    44100*2*2
                    // 1帧不是一秒，pFrame->nb_samples点
                    memcpy(jPcmData, resampleOutBuffer, dataSize);
                    LOGI("111111111111111111111111216");
                    // 0 把 c 的数组的数据同步到 jbyteArray , 然后释放native数组
                    jniCall->jniEnv->ReleaseByteArrayElements(jPcmByteArray, jPcmData, JNI_COMMIT);
                    LOGI("111111111111111111111111217");
                    // TODO
                    jniCall->callAudioTrackWrite(jPcmByteArray, 0, dataSize);
                    LOGI("111111111111111111111111218");
                }
            }
        }
        // 解引用
        av_packet_unref(pPacket);
        av_frame_unref(pFrame);
    }
    LOGI("1111111111111111111111122");

    // 1. 解引用数据 data ， 2. 销毁 pPacket 结构体内存  3. pPacket = NULL
    av_packet_free(&pPacket);
    av_frame_free(&pFrame);
    // 解除 jPcmDataArray 的持有，让 javaGC 回收
    jniCall->jniEnv->ReleaseByteArrayElements(jPcmByteArray, jPcmData, 0);
    jniCall->jniEnv->DeleteLocalRef(jPcmByteArray);
}

void MyFFmpeg::prepareAsync() {

}

void MyFFmpeg::callPlayerJniError(ThreadMode threadMode,int code, char *msg) {
    release();
    jniCall->callPlayerError(threadMode,code, msg);
}

void MyFFmpeg::release() {
    if(pCodecContext !=NULL){
        avcodec_close(pCodecContext);
        avcodec_free_context(&pCodecContext);
        pCodecContext=NULL;
    }

    if(pFormatContext!=NULL){
        avformat_close_input(&pFormatContext);
        avformat_free_context(pFormatContext);
        pFormatContext=NULL;
    }

    if(swrContext!=NULL){
        swr_free(&swrContext);
        free(swrContext);
        resampleOutBuffer=NULL;
    }
    if(resampleOutBuffer!=NULL){
        free(resampleOutBuffer);
        resampleOutBuffer=NULL;
    }
    avformat_network_deinit();
    if(url!=NULL){
        free((void *) url);
        url=NULL;
    }

}


