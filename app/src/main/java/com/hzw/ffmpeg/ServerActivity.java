package com.hzw.ffmpeg;

import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.v7.app.AppCompatActivity;
import android.widget.TextView;

import com.koushikdutta.async.AsyncServer;
import com.koushikdutta.async.http.server.AsyncHttpServer;
import com.koushikdutta.async.http.server.AsyncHttpServerRequest;
import com.koushikdutta.async.http.server.AsyncHttpServerResponse;
import com.koushikdutta.async.http.server.HttpServerRequestCallback;

import java.io.BufferedInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;

import Utils.WifiUtils;

/**
 * Created by huzw on 2018/1/22.
 */

public class ServerActivity extends AppCompatActivity{
    private AsyncServer mAsyncServer = new AsyncServer();
    private AsyncHttpServer mAsyncHttpServer = new AsyncHttpServer();

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_server);
        TextView tv = findViewById(R.id.tv);
        tv.setText(WifiUtils.getWifiIp(this));
        mAsyncHttpServer.get("/", new HttpServerRequestCallback() {
            @Override
            public void onRequest(AsyncHttpServerRequest request, AsyncHttpServerResponse response) {
                try {
                    response.send(getContent());
                } catch (Exception e) {
                    e.printStackTrace();
                    response.code(500).end();
                }
            }
        });

        mAsyncHttpServer.listen(mAsyncServer,12345);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        if (mAsyncHttpServer != null){
            mAsyncHttpServer.stop();
        }
        if (mAsyncServer != null){
            mAsyncServer.stop();
        }
    }

    private String getContent() throws IOException {
        BufferedInputStream bis = null;
        try {
            bis = new BufferedInputStream(getAssets().open("test.html"));
            ByteArrayOutputStream baos = new ByteArrayOutputStream();
            int len = 0;
            byte[] temp = new byte[1024];
            while ((len = bis.read(temp)) > 0){
                baos.write(temp,0,len);
            }
            return new String(baos.toByteArray(),"utf-8");
        } catch (IOException e){
            e.printStackTrace();
            throw e;
        } finally {
            if (bis != null){
                try {
                    bis.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }
    }
}
