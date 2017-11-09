package com.ic70.kkplayer.kkplayer;

import android.app.Activity;
import android.opengl.GLES11Ext;
import android.opengl.GLSurfaceView;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;
import android.graphics.SurfaceTexture;
import android.media.MediaCodec;
import android.view.Surface;
import android.opengl.GLES10Ext; //GLES11Ext.GL_BLEND_EQUATION_RGB_OES;

/**
 * Created by saint on 2016/3/9.
 */

public class CKKPlayerGlRender implements GLSurfaceView.Renderer,SurfaceTexture.OnFrameAvailableListener
{

    private CJniKKPlayer          m_JniKKPlayer;
    private int                   m_nKKPlayer=0;
    private SurfaceTexture        m_mediacodecSurfaceTexture=null;
   // private android.view.Surface  m_ViewSurface=null;
    private boolean      m_ReOpen=false;
    private CkkMediaInfo info= new CkkMediaInfo();
    private String m_url;
    private boolean mUpdateST = false;
    private boolean m_GlIniAfterOpen = false;
    public CKKPlayerGlRender()
    {
        m_JniKKPlayer = new CJniKKPlayer();
        m_nKKPlayer   = m_JniKKPlayer.IniKK(0);
       // MediaCodec mcodec = new android.media.MediaCodec()

    }
    @Override
    public void onFrameAvailable(SurfaceTexture surfaceTexture)
    {

        if(m_nKKPlayer!=0)
            m_JniKKPlayer.OnSurfaceTextureFrameAailable(m_nKKPlayer);
        Log.i("xxxx", "onFrameAvailable");
    }
    ///获取播放器实例
    public int GetPlayerIns() {
        return m_nKKPlayer;
    }
    @Override
    public void onDrawFrame(GL10 gl)
    {
        // Log.v("m_nKKPlayer", "="+m_nKKPlayer);
        if(m_nKKPlayer!=0){
            if(m_mediacodecSurfaceTexture!=null)
            m_mediacodecSurfaceTexture.updateTexImage();
            m_JniKKPlayer.GlRender(m_nKKPlayer);
        }
    }
    @Override
    public void onSurfaceChanged(GL10 gl, int width, int height)
    {
        mUpdateST = true;
        if(m_nKKPlayer!=0)
            m_JniKKPlayer.OnSize(m_nKKPlayer,width,height);
    }
    //当窗口被创建时需要调用 onSurfaceCreate ，我们可以在这里对 OpenGL 做一些初始化工作，例如：
    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config)
    {
        if(m_nKKPlayer!=0) {
            String glv = gl.glGetString(GL10.GL_VERSION);
            ///
            Log.v("Gl", "Gl Init");
            ///初始化会，初始化一个全局的SurfaceTexture出来
            m_JniKKPlayer.IniGl(m_nKKPlayer);
            ///
            m_mediacodecSurfaceTexture= m_JniKKPlayer.GetSurfaceTexture( m_nKKPlayer);
            if( m_mediacodecSurfaceTexture!=null){
                m_mediacodecSurfaceTexture.setOnFrameAvailableListener(this);
                Log.v("Gl", "new Surface");
                //m_ViewSurface = new Surface(m_mediacodecSurfaceTexture);
            }else{
                Log.v("Gl", "m_mediacodecSurfaceTexture =0 ");
            }
            if(m_GlIniAfterOpen) {
                    m_GlIniAfterOpen=false;
                    m_JniKKPlayer.KKCloseMedia(m_nKKPlayer);
                    m_JniKKPlayer.KKOpenMedia(m_url, m_nKKPlayer);
            }
        }
    }
    //暂停
    public void Pause()
    {
        if(m_nKKPlayer!=0) {
            m_JniKKPlayer.Pause(m_nKKPlayer);
        }
    }
    //
    public void Seek(int time)
    {
        if(m_nKKPlayer!=0) {
            m_JniKKPlayer.Seek(m_nKKPlayer, time);
        }
    }
    public CkkMediaInfo GetCMediaInfo()
    {
        if(m_nKKPlayer!=0) {
            m_JniKKPlayer.GetkkMediaInfo(m_nKKPlayer, info);
        }
        return  info;
    }

    public void SetDecoderMethod( int method)
    {
        if(m_nKKPlayer!=0) {
            m_JniKKPlayer.SetDecoderMethod(m_nKKPlayer, method);
        }
    }
    public int OpenMedia(String str, boolean GlIniAfterOpen)
    {
        m_url=str;
        String ll;
        ll=m_nKKPlayer+";";
        Log.v("MoviePath",str);
        m_GlIniAfterOpen=GlIniAfterOpen;
        if(m_GlIniAfterOpen==false) {
            if (m_nKKPlayer != 0) {
                m_JniKKPlayer.KKCloseMedia(m_nKKPlayer);
                return m_JniKKPlayer.KKOpenMedia(str, m_nKKPlayer);
            }
        }
        return 2;
    }
    public int OpenMedia(String str)
    {

        return OpenMedia( str, false);
    }
    public void SetKeepRatio( int KeepRatio)
    {
        if(m_nKKPlayer!=0) {
            m_JniKKPlayer.SetKeepRatio(m_nKKPlayer, KeepRatio);
        }
    }

    public void KKDel()
    {
        if(m_nKKPlayer!=0)
        {
            int ll=m_nKKPlayer;
            m_nKKPlayer=0;
            m_JniKKPlayer.DelKK(ll);

        }
    }

    /***
     *播放器状态
     * @return -1 播放器关闭状态
     ***/
     public enum EKKPlayerErr {

        KKOpenUrlOk,       //播发器打开成功
        KKOpenUrlOkFailure,   //播发器打开失败
        KKAVNotStream,
        KKAVReady,            //缓冲已经准备就绪
        KKAVWait,             //需要缓冲
        KKRealTimeOver,
        KKEOF,                //文件结束了。
        KKAVOver,            //视频结束
    }
    public EKKPlayerErr GetPlayerState2(int err)
    {
        if(err==0)
            return EKKPlayerErr.KKOpenUrlOk;
        else if(err==1)
            return EKKPlayerErr.KKOpenUrlOkFailure;
        else if(err==2)
            return EKKPlayerErr.KKAVNotStream;
        else if(err==3)
            return EKKPlayerErr.KKAVReady;
        else if(err==4)
            return EKKPlayerErr.KKAVWait;
        else if(err==5)
            return EKKPlayerErr.KKRealTimeOver;
        else if(err==6)
            return EKKPlayerErr.KKEOF;
        else if(err==7)
            return EKKPlayerErr.KKAVOver;
        return         EKKPlayerErr.KKOpenUrlOk;
    }
    public EKKPlayerErr GetPlayerState()
    {
        if(m_nKKPlayer!=0) {
            int err =m_JniKKPlayer.KKGetPlayerState(m_nKKPlayer);
            if(err==0)
                return EKKPlayerErr.KKOpenUrlOk;
            else if(err==1)
                return EKKPlayerErr.KKOpenUrlOkFailure;
            else if(err==2)
                return EKKPlayerErr.KKAVNotStream;
            else if(err==3)
                return EKKPlayerErr.KKAVReady;
            else if(err==4)
                return EKKPlayerErr.KKAVWait;
            else if(err==5)
                return EKKPlayerErr.KKRealTimeOver;
            else if(err==6)
                return EKKPlayerErr.KKEOF;
            else if(err==7)
                return EKKPlayerErr.KKAVOver;
        }
        return         EKKPlayerErr.KKOpenUrlOk;
    }
    public int GetReady()
    {
        if(m_nKKPlayer!=0) {
            return m_JniKKPlayer.KKIsReady(m_nKKPlayer);
        }
        return 0;
    }
    public int GetRealtime() {
        if(m_nKKPlayer!=0) {
            return m_JniKKPlayer.KKGetRealtime(m_nKKPlayer);
        }
        return 0;
    }
    public int GetRealtimeDelay()
    {
       // SurfaceTexture xx = new SurfaceTexture()
        if(m_nKKPlayer!=0) {
            return m_JniKKPlayer.KKGetRealtimeDelay(m_nKKPlayer);
        }
        return 0;
    }
    public int SetMinRealtimeDelay(int value)
    {
        if(m_nKKPlayer!=0) {
            return m_JniKKPlayer.KKSetMinRealtimeDelay(m_nKKPlayer, value);
        }
        return 0;
    }
    public int GetNeedReConnect()
    {
        if(m_nKKPlayer!=0) {
            return  m_JniKKPlayer.KKIsNeedReConnect(m_nKKPlayer);
        }
        return 0;
    }

}
