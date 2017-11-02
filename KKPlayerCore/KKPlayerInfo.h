///һЩ��������ݶ���
#ifndef KKPlayerInfo_H_
#define KKPlayerInfo_H_
typedef struct AVCACHE_INFO
{
	int VideoSize;
	int AudioSize;
	int MaxTime;
}AVCACHE_INFO;
typedef struct AVCODE_INFO
{
  ///����������
  char codecname[32];
  //ƽ������
  int bitrate;
  int framerate;
  int sample_rate;
  int channels;//�� �� ��
}AVCODE_INFO;
//��ȡ��������Ϣ
typedef struct MEDIA_INFO 
{
	//�ֱ���
	char              AVRes[1024];
	//������Ϣ
	AVCODE_INFO       videoinfo;
	AVCODE_INFO       audioinfo;
	const char*       AvFile;
	int               FileSize;
	int               CurTime;
	int               TotalTime;//��ʱ��
	int               serial;
    int               serial1;
	bool              Open;
	int               KKState;
    short             SegId;
	char              SpeedInfo[1024];     ///�����ٶ�
	AVCACHE_INFO      CacheInfo;
}MEDIA_INFO ;

///��Ƶͼ����Ϣ
typedef struct kkAVPicInfo
{
    unsigned char *data[8];
    int      linesize[8];
	int      picformat;
	int      width;
	int      height;

}kkAVPicInfo;

///�����滻URL
typedef int (*fpKKPlayerGetUrl)(char* url,char** outurlstr);
#endif