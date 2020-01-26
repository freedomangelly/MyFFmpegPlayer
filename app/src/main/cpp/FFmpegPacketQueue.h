//
// Created by freed on 2020/1/26.
//

#ifndef MYFFMPEGPLAYER_FFMPEGPACKETQUEUE_H
#define MYFFMPEGPLAYER_FFMPEGPACKETQUEUE_H

#include <queue>
#include <pthread.h>
extern  "C"{
#include <libavcodec/avcodec.h>
};


class FFmpegPacketQueue {
public:
    std::queue<AVPacket *> *pPacketQueue;
    pthread_mutex_t packetMutex;
    pthread_cond_t packetCond;

public:
    FFmpegPacketQueue();
    ~FFmpegPacketQueue();

public :
    void push(AVPacket *pPacket);

    AVPacket *pop();

    void clear();
};


#endif //MYFFMPEGPLAYER_FFMPEGPACKETQUEUE_H
