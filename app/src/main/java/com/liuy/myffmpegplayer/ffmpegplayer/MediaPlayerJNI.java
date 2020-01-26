package com.liuy.myffmpegplayer.ffmpegplayer;

import android.text.TextUtils;
import android.util.Log;

import com.liuy.myffmpegplayer.ffmpegplayer.listener.MediaErrorListener;
import com.liuy.myffmpegplayer.ffmpegplayer.listener.MediaPreparedListener;

import java.util.concurrent.Executor;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

/**
 * description:
 * author: freed on 2020/1/18
 * email: 674919909@qq.com
 * version: 1.0
 */
public class MediaPlayerJNI {
    static {
        System.loadLibrary("music-player");
    }
    /**
     * url 可以是本地文件路径，也可以是 http 链接
     */
    private String url;

    private MediaErrorListener mErrorListener;

    private MediaPreparedListener mPreparedListener;

    public void setOnErrorListener(MediaErrorListener errorListener) {
        this.mErrorListener = errorListener;
    }

    public void setOnPreparedListener(MediaPreparedListener preparedListener) {
        this.mPreparedListener = preparedListener;
    }

    // called from jni
    private void onError(int code, String msg) {
        Log.i("MY_FFMPEG_TAG","onError");
        if (mErrorListener != null) {
            mErrorListener.onError(code, msg);
        }
    }

    // called from jni
    private void onPrepared() {
        Log.i("MY_FFMPEG_TAG","onPrepared");
        if (mPreparedListener != null) {
            mPreparedListener.onPrepared();
        }
    }

    public void setDataSource(String url) {
        this.url = url;
    }

    public void play() {
        if (TextUtils.isEmpty(url)) {
            throw new NullPointerException("url is null, please call method setDataSource");
        }
        nPlay();
    }

    public void play2(String url){
        nPlay2(url);
    }

    private native void nPlay();
    private native void nPlay2(String url);

    public void prepare() {
        if (TextUtils.isEmpty(url)) {
            throw new NullPointerException("url is null, please call method setDataSource");
        }
        nPrepare(url);
    }

    /**
     * 异步准备
     */
    public void prepareAsync() {
        if (TextUtils.isEmpty(url)) {
            throw new NullPointerException("url is null, please call method setDataSource");
        }
        nPrepareAsync(url);
    }

    private native void nPrepareAsync(String url);

    private native void nPrepare(String url);
}
