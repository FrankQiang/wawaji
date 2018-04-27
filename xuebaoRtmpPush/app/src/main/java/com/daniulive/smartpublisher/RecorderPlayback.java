/*
 * RecorderPlayback.java
 * RecorderPlayback
 * 
 * Github: https://github.com/daniulive/SmarterStreaming
 * 
 * Created by DaniuLive on 2015/09/20.
 * Copyright © 2014~2016 DaniuLive. All rights reserved.
 */

package com.daniulive.smartpublisher;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.widget.VideoView;
import android.widget.TextView;
import android.util.Log;
import android.widget.MediaController;

import com.xuebao.rtmpPush.CameraPublishActivity;
import com.xuebao.rtmpPush.R;

public class RecorderPlayback extends Activity {

	private final String Tag = "RecorderPlayback";
	
	private String recorderFilePath = null;
	private TextView filePathTextView = null;
	private VideoView playVideoView = null;
	
	@Override
    public void onCreate(Bundle savedInstanceState) 
    {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_recorder_playback);
        
        Intent intent = getIntent();
        recorderFilePath = intent.getStringExtra("RecorderFilePath");
        
        
        filePathTextView = (TextView) findViewById(R.id.textViewRecoderPlaybackFilePath);
        if (recorderFilePath != null  )
        {
        	filePathTextView.setText(recorderFilePath);
        }
        else
        {
            if(CameraPublishActivity.DEBUG) Log.i(Tag, "recorderFilePath is null");
        }
       
           
        playVideoView  = (VideoView) findViewById(R.id.VideoViewRecoderPlayback);
        
        if ( recorderFilePath != null && !recorderFilePath.isEmpty() )
        {
        	playVideoView.setVideoPath(recorderFilePath);
        	playVideoView.setMediaController(new MediaController(this));
        	playVideoView.requestFocus();  
        	playVideoView.start();
        	
        }
    }	
}
