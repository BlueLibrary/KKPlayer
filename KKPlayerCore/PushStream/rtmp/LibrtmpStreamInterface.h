
#ifndef __LIBRTMP_STREAM_INTERFACE__
#define __LIBRTMP_STREAM_INTERFACE__

#include "LibtrmpData.h"

class LibRtmpStream
{
public:
	// ���ӵ�RTMP Server    
	virtual bool Connect(const char* url) = 0;    
	// �Ͽ�����    
	virtual void Close() = 0;    
	// ����MetaData    
	virtual bool SendMetadata(LPRTMPMetadata lpMetaData) = 0;    
	// ����H264����֡    
	virtual bool SendH264Packet(unsigned char *data,unsigned int size,bool bIsKeyFrame,unsigned int nTimeStamp) = 0; 
	//����AAC����
	virtual bool SendAACPacket(unsigned char* data,unsigned int size,unsigned int nTimeStamp ) =0;
	
};

LibRtmpStream *LibrtmpGetInstance();
#endif