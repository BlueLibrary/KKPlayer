package com.ic70.kkplayer.kkplayer;

/**
 * Created by saint on 2017/7/7.
 */

public class CkkMediaInfo {


        public CkkMediaInfo()
        {

        }

        //分辨率
        public String              AVRes =new String();
        ///解码器名称
        public String              VideoCodecname=new String();
        //平均码率
        public int                 VideoBitrate=0;
        public int                 VideoFramerate=0;

        public String              AudioCodecname=new String();
        public int                 AudioSample_rate=0;///音频采样率
        public int                 AudioChannels=0;//声 道 数

        public int                 FileSize=0;
        public int                 CurTime=0;
        public int                 TotalTime=0;//总时长
        public int                 Serial=0;
        public int                 Serial1=0;
        public int                 Open=0;
        public int                 KKState=0;
        public int                 SegId=0;
        public String               SpeedInfo=new String();    ///下载速度
        public int CacheVideoSize=0;
        public int CacheAudioSize=0;
        public int CacheMaxTime=0;
}
