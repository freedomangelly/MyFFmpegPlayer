//
// Created by freed on 2020/1/26.
//

#include "FFmpegPacketQueue.h"

FFmpegPacketQueue::FFmpegPacketQueue() {
    pPacketQueue=new std::queue<AVPacket *>;
    pthread_mutex_init(&packetMutex,NULL);
    pthread_cond_init(&packetCond,NULL);
}

FFmpegPacketQueue::~FFmpegPacketQueue() {
    if(pPacketQueue!=NULL){
        clear();
        delete(pPacketQueue);
        pPacketQueue=NULL;
    }
    pthread_mutex_destroy(&packetMutex);
    pthread_cond_destroy(&packetCond);
}

void FFmpegPacketQueue::push(AVPacket *pPacket) {
    pthread_mutex_lock(&packetMutex);
    pPacketQueue->push(pPacket);
    pthread_cond_signal(&packetCond);
    pthread_mutex_unlock(&packetMutex);
}

AVPacket *FFmpegPacketQueue::pop() {
    AVPacket *pPacket;
    pthread_mutex_lock(&packetMutex);
    while (pPacketQueue->empty()){
        pthread_cond_wait(&packetCond,&packetMutex);
    }
    pPacket=pPacketQueue->front();
    pPacketQueue->pop();
    pthread_mutex_unlock(&packetMutex);
    return pPacket;
}

void FFmpegPacketQueue::clear() {
// 需要清除队列，还需要清除每个 AVPacket* 的内存数据
}
