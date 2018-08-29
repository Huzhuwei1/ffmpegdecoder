package opengles;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.util.AttributeSet;

/**
 * Created by huzw on 2018/1/10.
 */

public class MyGlSurfaceView extends GLSurfaceView{
    private MyGlRender mGlRender;

    public MyGlSurfaceView(Context context) {
        this(context, null);
    }

    public MyGlSurfaceView(Context context, AttributeSet attrs) {
        super(context, attrs);
        mGlRender = new MyGlRender(context);
        //设置egl版本为2.0
        setEGLContextClientVersion(2);
        //设置render
        setRenderer(mGlRender);
        //设置为手动刷新模式
        setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);
    }


    public void setFrameData(int w, int h, byte[] y, byte[] u, byte[] v) {
        if(mGlRender != null) {
            mGlRender.setFrameData(w, h, y, u, v);
            requestRender();
        }
    }

    public void capture() {
        if(mGlRender != null) {
            mGlRender.capture();
            requestRender();
        }
    }

    public void setOnCaptureListener(MyGlRender.ScreenCaptureListener listener) {
        if(listener != null) {
            mGlRender.setListener(listener);
        }
    }
}
