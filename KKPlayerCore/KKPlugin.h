typedef unsigned char      uint8_t;
typedef long long          int64_t;
#ifndef KKPlugin_H_
#define KKPlugin_H_
extern "C"
{  
	
	//�õ�Э��ͷ
	typedef char (*fGetPtlHeader)(char *buf,char len);
	/***********�����ͷ��ڴ�**************/
	typedef void (*fFree)(void*);
    /****���غ���****/
    typedef char (*fKKDownAVFile)(char *strUrl);
	/********ֹͣ���غ���**********/
	typedef char (*fKKStopDownAVFile)(char *str);
	
	/***********�����ٶ���Ϣ***************/
	typedef struct KK_DOWN_SPEED_INFO{
		char IpPort[126];//��|�ָ�˿�
        unsigned int Speed;
		unsigned int AcSize;
		unsigned int FileSize;
		int Tick;
		KK_DOWN_SPEED_INFO *Next;
	}KK_DOWN_SPEED_INFO;
	/************�õ������ٶ���Ϣ*********/
    typedef KK_DOWN_SPEED_INFO* (*fKKDownAVFileSpeedInfo)(char *strurl,int *TotalSpeed);
    //ˢ�¶���
	typedef void (*fFlushPlayerQue)(void *opaque);
	//�����ӳٻص�����AVType 0,��Ƶ��1��Ƶ
	typedef void (*fCalPlayerDelay)(void *opaque,int64_t Pts,int AVType);

	typedef int  (*fIo_read_packet)(void *opaque, uint8_t *buf, int buf_size);
    //�ļ�seek
	typedef int64_t  (*fIo_seek)(void *opaque, int64_t offset, int whence);
	//�õ�����ʱ��
	typedef unsigned int     (*fGetCacheTime)(void *opaque);
	//
	typedef int  (*fKKIRQ)(void *opaque);  //ǿ���жϺ���

	typedef struct __KKPlugin
	{
        void *opaque;                        //�û�����
		fIo_read_packet kkread;              
		fIo_seek kkseek;
		fFlushPlayerQue FlushQue;           //�������� kkirqOpaque,ˢ�¶��к���
		fCalPlayerDelay CalPlayerDelay;     //�ⲿ�ص�������Player����
		fGetCacheTime GetCacheTime;
		fKKIRQ kkirq;                       //�ⲿ����,�������ڲ��������Ӧ���ã�����1�жϣ���������kkirqOpaque
        void *PlayerOpaque;                 //����������
		char *URL;                          //ȥ��Э��ͷ�ĵ�ַ
		int RealTime;                       //1Ϊʵʱ������ӦΪ0
		int FirstRead;                      //��һ�ζ���1,����Ϊ0�����ֶ�
	}KKPlugin;
	
	//����һ�����ʵ��
	typedef KKPlugin*  (*fCreateKKPlugin)();
	typedef void (*fDeleteKKPlugin)(KKPlugin*p);
	typedef struct __KKPluginInfo
	{
		char ptl[32];
		/******����һ�����******/
		fCreateKKPlugin CreKKP;
		/**********ɾ��һ�����**************/
		fDeleteKKPlugin DelKKp;
		/***********�����ļ�*****************/
		fKKDownAVFile   KKDownAVFile;
		/************ֹͣ����*****************/
		fKKStopDownAVFile KKStopDownAVFile;
		/*************�õ������ٶ���Ϣ************/
		fKKDownAVFileSpeedInfo KKDownAVFileSpeedInfo;
		/*****************�ͷ��ڴ�*************/
		fFree KKFree;
#ifdef WIN32
		HMODULE	Handle;
#else
		void*  Handle;/*****************/
#endif
	}KKPluginInfo;

	
};
#endif