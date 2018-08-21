package com.hzw.ffmpeg;

import android.os.Bundle;
import android.os.Handler;
import android.support.v7.app.AppCompatActivity;
import android.text.TextUtils;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;

import opengles.MyGlSurfaceView;

public class MainActivity extends AppCompatActivity {

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
    }

    private MyGlSurfaceView mGlSurfaceView;
    private EditText mEtInput;
    private Button mBtStart;

    Handler mHandler = new Handler();

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // Example of a call to a native method
        TextView tv = findViewById(R.id.sample_text);
        tv.setText(stringFromJNI());
        mEtInput = findViewById(R.id.et_input);
        mBtStart = findViewById(R.id.bt_start);
        mGlSurfaceView = findViewById(R.id.gl_surfaceview);

        mEtInput.setText("/storage/emulated/0/test.264");
    }

    public void decCallBack(int w, int h, byte[] y, byte[] u, byte[] v){
//        Log.e("decCallBack","数据来啦：");
        mGlSurfaceView.setFrameData(w,h,y,u,v);
    }

    public void onClick(View v){
        switch (v.getId()){
            case R.id.bt_start:
                if (!TextUtils.isEmpty(mEtInput.getText())){
                    init(mEtInput.getText().toString());
                    start();
                }
                break;
                default:break;
        }
    }


    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    public native String stringFromJNI();

    public native void init(String url);

    public native void start();
}
