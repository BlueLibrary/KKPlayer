typedef unsigned char      uint8_t;
typedef long long          int64_t;
#ifndef KKPlugin_H_
#define KKPlugin_H_
extern "C"
{   typedef char (*fGetPtlHeader)(char *buf,char len);
    /****���غ���****/
    typedef char (*fKKDownAVFile)(char *strUrl);
	/********ֹͣ���غ���**********/
	typedef char (*fKKStopDownAVFile)(char *str);
	
	
	/************�����ٶȻص�����*********/
    //typedef char (*fKKDownAVFileSpeed)(char *str,int DownSpeed);
    //ˢ�¶���
	typedef void (*fFlushPlayerQue)(void *opaque);
	//�����ӳٻص�����AVType 0,��Ƶ��1��Ƶ
	typedef void (*fCalPlayerDelay)(void *opaque,int64_t Pts,int AVType);
	typedef int  (*fIo_read_packet)(void *opaque, uint8_t *buf, int buf_size);
    //�ļ�seek
	typedef int64_t  (*fIo_seek)(void *opaque, int64_t offset, int whence);
	//
	typedef int  (*fKKIRQ)(void *opaque);  //ǿ���жϺ���

	typedef struct __KKPlugin
	{
        void *opaque;                 //�û�����
		fIo_read_packet kkread;
		fIo_seek kkseek;
		fFlushPlayerQue FlushQue;    //�������� kkirqOpaque,ˢ�¶���
		fCalPlayerDelay CalPlayerDelay;
		fKKIRQ kkirq;                //�ⲿ����,�������ڲ��������Ӧ���ã�����1�жϣ���������kkirqOpaque
        void *PlayerOpaque;          //����������
		char *URL;                   //ȥ��Э��ͷ�ĵ�ַ
		int RealTime;                //1 Ϊʵʱ��������Ϊ0
	}KKPlugin;
	
	//����һ�����ʵ��
	typedef KKPlugin*  (*fCreateKKPlugin)();
	typedef void (*fDeleteKKPlugin)(KKPlugin*p);
	typedef struct __KKPluginInfo
	{
		char ptl[32];
		fCreateKKPlugin CreKKP;
		fDeleteKKPlugin DelKKp;
		fKKDownAVFile   KKDownAVFile;
		fKKStopDownAVFile KKStopDownAVFile;
	//	fKKDownAVFileSpeed KKDownAVFileSpeed;
#ifdef WIN32
		HMODULE	Handle;
#else
		void*  Handle;/*****************/
#endif
	}KKPluginInfo;

	
};
#endif