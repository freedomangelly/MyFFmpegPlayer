//
// Created by freed on 2020/1/25.
//

#include "MyFFmpeg.h"
#include "MyConsts.h"

MyFFmpeg::MyFFmpeg(FFMpegJniCall *jniCall, const char *url) {
    this->jniCall=jniCall;
//    this->url=url;
    this->url= static_cast<char *>(malloc(strlen(url) + 1));
    LOGI("enter MyFFmpeg Play2 %s",url);
    memcpy((void *) this->url, url, strlen(url) + 1);
}

MyFFmpeg::~MyFFmpeg() {
    release();
}

//void *threadPlay(void *context){
//    MyFFmpeg *pFFmpeg= (MyFFmpeg *)(context);
//    pFFmpeg->prepare(THREAD_CHILD);
//    return 0;
//}

void MyFFmpeg::play() {
    //主线程
//    MyFFmpeg *pFFmpeg= (MyFFmpeg *)(this);
//    pFFmpeg->prepare();

//子线程
//    pthread_t playThreadT;
//    pthread_create(&playThreadT,NULL,threadPlay,this);
//    pthread_detach(playThreadT);
    LOGI("nPlay 2");
    if(pAudio!=NULL){
        LOGI("nPlay 3");
        pAudio->play();
    }

}

void MyFFmpeg::prepare(ThreadMode threadMode) {


    av_register_all();
    avformat_network_init();

    int formatOpenInputRes = 0;
    int formatFindStreamInfoRes = 0;
    int audioStramIndex = -1;
//    AVCodecParameters *pCodecParameters;
//    AVCodec *pCodec = NULL;
//    int codecParametersToContextRes = -1;
//    int codecOpenRes = -1;
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
//    LOGI("1111111111111111111111114");
//    // 查找解码
//    pCodecParameters = pFormatContext->streams[audioStramIndex]->codecpar;
//    pCodec = avcodec_find_decoder(pCodecParameters->codec_id);
//    if (pCodec == NULL) {
//        LOGE("codec find audio decoder error");
//        // 这种方式一般不推荐这么写，但是的确方便
//        callPlayerJniError(threadMode,CODEC_FIND_DECODER_ERROR_CODE, "codec find audio decoder error");
//        return;
//    }
//    LOGI("1111111111111111111111115");
//    // 打开解码器
//    pCodecContext = avcodec_alloc_context3(pCodec);
//    if (pCodecContext == NULL) {
//        LOGE("codec alloc context error");
//        // 这种方式一般不推荐这么写，但是的确方便
//        callPlayerJniError(threadMode,CODEC_ALLOC_CONTEXT_ERROR_CODE, "codec alloc context error");
//        return;
//    }
//    LOGI("1111111111111111111111116");
//    codecParametersToContextRes = avcodec_parameters_to_context(pCodecContext, pCodecParameters);
//    if (codecParametersToContextRes < 0) {
//        LOGE("codec parameters to context error: %s", av_err2str(codecParametersToContextRes));
//        callPlayerJniError(threadMode,codecParametersToContextRes, av_err2str(codecParametersToContextRes));
//        return;
//    }
//    LOGI("1111111111111111111111117");
//    codecOpenRes = avcodec_open2(pCodecContext, pCodec, NULL);
//    if (codecOpenRes != 0) {
//        LOGI("codec audio open error: %s", av_err2str(codecOpenRes));
//        callPlayerJniError(threadMode,codecOpenRes, av_err2str(codecOpenRes));
//        return;
//    }
//    LOGI("1111111111111111111111118");
//    // ---------- 重采样 start ----------
//    int64_t out_ch_layout = AV_CH_LAYOUT_STEREO;
//    enum AVSampleFormat out_sample_fmt = AVSampleFormat::AV_SAMPLE_FMT_S16;
//    int out_sample_rate = AUDIO_SAMPLE_RATE;
//    int64_t in_ch_layout = pCodecContext->channel_layout;
//    enum AVSampleFormat in_sample_fmt = pCodecContext->sample_fmt;
//    int in_sample_rate = pCodecContext->sample_rate;
//    swrContext = swr_alloc_set_opts(NULL, out_ch_layout, out_sample_fmt,
//                                    out_sample_rate, in_ch_layout, in_sample_fmt, in_sample_rate, 0, NULL);
//    if (swrContext == NULL) {
//        // 提示错误
//        callPlayerJniError(threadMode,SWR_ALLOC_SET_OPTS_ERROR_CODE, "swr alloc set opts error");
//        return;
//    }
//    LOGI("1111111111111111111111119");
//    int swrInitRes = swr_init(swrContext);
//    if (swrInitRes < 0) {
//        callPlayerJniError(threadMode,SWR_CONTEXT_INIT_ERROR_CODE, "swr context swr init error");
//        return;
//    }
//    LOGI("1111111111111111111111111 10");
    pAudio=new FFmpegAudio(audioStramIndex,jniCall, NULL,pFormatContext);
    LOGI("1111111111111111111111115");
    pAudio->analysisStream(threadMode,pFormatContext->streams);
    LOGI("1111111111111111111111116");
    jniCall->callPlayerPrepared(threadMode);
    LOGI("1111111111111111111111117");

}
void *threadPrepare(void *context) {
    MyFFmpeg *pFFmpeg = (MyFFmpeg *) context;
    pFFmpeg->prepare(THREAD_CHILD);
    return 0;
}
void MyFFmpeg::prepareAsync() {
    pthread_t prepareThreadT;
    pthread_create(&prepareThreadT, NULL, threadPrepare, this);
    pthread_detach(prepareThreadT);
//    this->prepare(THREAD_MAIN);
}

void MyFFmpeg::callPlayerJniError(ThreadMode threadMode,int code, char *msg) {
    release();
    jniCall->callPlayerError(threadMode,code, msg);
}

void MyFFmpeg::release() {
//    if(pCodecContext !=NULL){
//        avcodec_close(pCodecContext);
//        avcodec_free_context(&pCodecContext);
//        pCodecContext=NULL;
//    }

    if(pFormatContext!=NULL){
        avformat_close_input(&pFormatContext);
        avformat_free_context(pFormatContext);
        pFormatContext=NULL;
    }

//    if(swrContext!=NULL){
//        swr_free(&swrContext);
//        free(swrContext);
//        swrContext=NULL;
//    }
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

void MyFFmpeg::prepare() {
    prepare(THREAD_MAIN);
}


