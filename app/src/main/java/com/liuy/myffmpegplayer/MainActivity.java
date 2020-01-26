package com.liuy.myffmpegplayer;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.os.Environment;
import android.util.Log;
import android.widget.TextView;

import com.liuy.myffmpegplayer.ffmpegplayer.MediaPlayerJNI;
import com.liuy.myffmpegplayer.ffmpegplayer.listener.MediaErrorListener;
import com.liuy.myffmpegplayer.ffmpegplayer.listener.MediaPreparedListener;

import java.io.File;

public class MainActivity extends AppCompatActivity {
    File mMusicFile = new File(Environment.getExternalStorageDirectory(), "ental.mp3");
    // Used to load the 'native-lib' library on application startup.
    private MediaPlayerJNI mPlayer;

    long tetet=0;

    public static void getLong(long te){

    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        Log.e("TAG", "file is exist: " + mMusicFile.exists());

        mPlayer = new MediaPlayerJNI();
        mPlayer.setDataSource(mMusicFile.getAbsolutePath());
//
        mPlayer.setOnErrorListener(new MediaErrorListener() {
            @Override
            public void onError(int code, String msg) {
                Log.e("TAG", "error code: " + code);
                Log.e("TAG", "error msg: " + msg);
                // Java 的逻辑代码
            }
        });
//
        mPlayer.setOnPreparedListener(new MediaPreparedListener() {
            @Override
            public void onPrepared() {
                Log.e("TAG", "准备完毕");
                mPlayer.play();
            }
        });

//        mPlayer.prepare();
//        mPlayer.play();
        mPlayer.prepareAsync();

    }
}
