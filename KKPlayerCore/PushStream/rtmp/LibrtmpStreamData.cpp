#include "LibrtmpStreamData.h"


RTMPStream::RTMPStream(void)
{
	m_pRtmp = NULL;
	InitSockets();
	m_pRtmp = RTMP_Alloc();  
	RTMP_Init(m_pRtmp);
}
RTMPStream::~RTMPStream(void)
{
}

int RTMPStream::InitSockets()  
{  
#ifdef WIN32  
	WORD version;  
	WSADATA wsaData;  
	version = MAKEWORD(1, 1);  
	return (WSAStartup(version, &wsaData) == 0);  
#else  
	return TRUE;  
#endif  
}  


void RTMPStream::CleanupSockets()  
{  
#ifdef WIN32  
	WSACleanup();  
#endif  
}  

char * RTMPStream::put_byte( char *output, uint8_t nVal )  
{  
	output[0] = nVal;  
	return output+1;  
}  
char * RTMPStream::put_be16(char *output, uint16_t nVal )  
{  
	output[1] = nVal & 0xff;  
	output[0] = nVal >> 8;  
	return output+2;  
}  
char * RTMPStream::put_be24(char *output,uint32_t nVal )  
{  
	output[2] = nVal & 0xff;  
	output[1] = nVal >> 8;  
	output[0] = nVal >> 16;  
	return output+3;  
}  
char * RTMPStream::put_be32(char *output, uint32_t nVal )  
{  
	output[3] = nVal & 0xff;  
	output[2] = nVal >> 8;  
	output[1] = nVal >> 16;  
	output[0] = nVal >> 24;  
	return output+4;  
}  
char *  RTMPStream::put_be64( char *output, uint64_t nVal )  
{  
	output=put_be32( output, nVal >> 32 );  
	output=put_be32( output, nVal );  
	return output;  
}  
char * RTMPStream::put_amf_string( char *c, const char *str )  
{  
	uint16_t len = strlen( str );  
	c=put_be16( c, len );  
	memcpy(c,str,len);  
	return c+len;  
}  
char * RTMPStream::put_amf_double( char *c, double d )  
{  
	*c++ = AMF_NUMBER;  /* type: Number */  
	{  
		unsigned char *ci, *co;  
		ci = (unsigned char *)&d;  
		co = (unsigned char *)c;  
		co[0] = ci[7];  
		co[1] = ci[6];  
		co[2] = ci[5];  
		co[3] = ci[4];  
		co[4] = ci[3];  
		co[5] = ci[2];  
		co[6] = ci[1];  
		co[7] = ci[0];  
	}  
	return c+8;  
}


// ���ӵ�RTMP Server
bool RTMPStream::Connect(const char* url)
{
	if(RTMP_SetupURL(m_pRtmp, (char*)url)<0)
	{
		return FALSE;
	}
	RTMP_EnableWrite(m_pRtmp);
	if(RTMP_Connect(m_pRtmp, NULL)<0)
	{
		return FALSE;
	}
	if(RTMP_ConnectStream(m_pRtmp,0)<0)
	{
		return FALSE;
	}
	return TRUE;
}
// �Ͽ�����
void RTMPStream::Close()
{
	if(m_pRtmp)
	{

		int x=RTMP_IsConnected(m_pRtmp);
		WSACleanup();
		
		RTMP_Close(m_pRtmp);
		RTMP_Free(m_pRtmp);

		m_pRtmp = NULL;
		  
	}
}
// ����MetaData
bool RTMPStream::SendMetadata(LPRTMPMetadata lpMetaData)
{
	if(lpMetaData == NULL)
	{
		return false;
	}

	char body[1024] = {0};

	char * p = (char *)body;  
	p = put_byte(p, AMF_STRING );
	p = put_amf_string(p , "@setDataFrame" );

	p = put_byte( p, AMF_STRING );
	p = put_amf_string( p, "onMetaData" );

	p = put_byte(p, AMF_OBJECT );  
	p = put_amf_string( p, "copyright" );  
	p = put_byte(p, AMF_STRING );  
	p = put_amf_string( p, "firehood" );  
#if 0
	//��Ƶm_nCurPos
	p =put_amf_string( p, "width");
	p =put_amf_double( p, lpMetaData->nWidth);

	p =put_amf_string( p, "height");
	p =put_amf_double( p, lpMetaData->nHeight);

	p =put_amf_string( p, "framerate" );
	p =put_amf_double( p, lpMetaData->nFrameRate); 

	p =put_amf_string( p, "videodatarate" );
	p =put_amf_double( p, lpMetaData->nVideoDataRate); 

	p =put_amf_string( p, "videocodecid" );
	p =put_amf_double( p, FLV_CODECID_H264 );
#else
	//��Ƶ

	p =put_amf_string( p, "audiodatarate");
	p =put_amf_double( p, lpMetaData->nAudioDatarate);

	p =put_amf_string( p, "audiosamplerate");
	p =put_amf_double( p, lpMetaData->nAudioSampleRate);

	p =put_amf_string( p, "audiosamplesize" );
	p =put_amf_double( p, lpMetaData->nAudioSampleSize); 

	p =put_amf_string( p, "stereo" );
	p =put_amf_double( p, lpMetaData->nAudioChannels); 


	p =put_amf_string( p, "audiocodecid" );
	p =put_amf_double( p, lpMetaData->nAudioFmt); 

#endif
	p =put_amf_string( p, "" );
	p =put_byte( p, AMF_OBJECT_END  );

	int index = p-body;

	return SendPacket(RTMP_PACKET_TYPE_INFO,(unsigned char*)body,p-body,0);

	int i = 0;
	body[i++] = 0x17; // 1:keyframe  7:AVC
	body[i++] = 0x00; // AVC sequence header

	body[i++] = 0x00;
	body[i++] = 0x00;
	body[i++] = 0x00; // fill in 0;

	// AVCDecoderConfigurationRecord.
	body[i++] = 0x01; // configurationVersion
	body[i++] = lpMetaData->Sps[1]; // AVCProfileIndication
	body[i++] = lpMetaData->Sps[2]; // profile_compatibility
	body[i++] = lpMetaData->Sps[3]; // AVCLevelIndication 
	body[i++] = 0xff; // lengthSizeMinusOne  

	// sps nums
	body[i++] = 0xE1; //&0x1f
	// sps data length
	body[i++] = lpMetaData->nSpsLen>>8;
	body[i++] = lpMetaData->nSpsLen&0xff;
	// sps data
	memcpy(&body[i],lpMetaData->Sps,lpMetaData->nSpsLen);
	i= i+lpMetaData->nSpsLen;

	// pps nums
	body[i++] = 0x01; //&0x1f
	// pps data length 
	body[i++] = lpMetaData->nPpsLen>>8;
	body[i++] = lpMetaData->nPpsLen&0xff;
	// sps data
	memcpy(&body[i],lpMetaData->Pps,lpMetaData->nPpsLen);
	i= i+lpMetaData->nPpsLen;

	return SendPacket(RTMP_PACKET_TYPE_VIDEO,(unsigned char*)body,i,0);

}
// ����H264����֡
bool RTMPStream::SendH264Packet(unsigned char *data,unsigned int size,bool bIsKeyFrame,unsigned int nTimeStamp)
{
	if(data == NULL && size<11)
	{
		return false;
	}

	unsigned char *body = new unsigned char[size+9];

	int i = 0;

	if(bIsKeyFrame)
		body[i++] = 0x17;// 1:Iframe  7:AVC
	else
		body[i++] = 0x27;// 2:Pframe  7:AVC

	body[i++] = 0x01;// AVC NALU
	body[i++] = 0x00;
	body[i++] = 0x00;
	body[i++] = 0x00;

	// NALU size
	body[i++] = size>>24;
	body[i++] = size>>16;
	body[i++] = size>>8;
	body[i++] = size&0xff;

	// NALU data
	memcpy(&body[i],data,size);
	bool bRet = SendPacket(RTMP_PACKET_TYPE_VIDEO,body,i+size,nTimeStamp);
	delete[] body;
	return bRet;
}
//����AAC����
bool RTMPStream::SendAACPacket(unsigned char* data,unsigned int size,unsigned int nTimeStamp )
{
	if(m_pRtmp == NULL)
		return FALSE;

	if (size > 0) 
	{
		RTMPPacket * packet;
		unsigned char * body;

		packet = (RTMPPacket *)malloc(RTMP_HEAD_SIZE+size+2);
		memset(packet,0,RTMP_HEAD_SIZE);

		packet->m_body = (char *)packet + RTMP_HEAD_SIZE;
		body = (unsigned char *)packet->m_body;

		/*AF 01 + AAC RAW data*/
		body[0] = 0xAF;
		body[1] = 0x01;
		memcpy(&body[2],data,size);

		packet->m_packetType = RTMP_PACKET_TYPE_AUDIO;
		packet->m_nBodySize = size+2;
		packet->m_nChannel = 0x04;
		packet->m_nTimeStamp = nTimeStamp;
		packet->m_hasAbsTimestamp = 0;
		packet->m_headerType = RTMP_PACKET_SIZE_MEDIUM;
		packet->m_nInfoField2 = m_pRtmp->m_stream_id;

		/*���÷��ͽӿ�*/
		RTMP_SendPacket(m_pRtmp,packet,TRUE);
		free(packet);
	}
	return true;
}

// ��������
int RTMPStream::SendPacket(unsigned int nPacketType,unsigned char *data,unsigned int size,unsigned int nTimestamp)
{
	if(m_pRtmp == NULL)
		return FALSE;


	RTMPPacket packet;
	RTMPPacket_Reset(&packet);
	RTMPPacket_Alloc(&packet,size);

	packet.m_packetType = nPacketType;
	packet.m_nChannel = 0x04;  
	packet.m_headerType = /*RTMP_PACKET_SIZE_MEDIUM*/RTMP_PACKET_SIZE_LARGE;  
	packet.m_nTimeStamp = nTimestamp;  
	packet.m_nInfoField2 = m_pRtmp->m_stream_id;
	packet.m_nBodySize = size;
	packet.m_hasAbsTimestamp = 0;
	memcpy(packet.m_body,data,size);

	//�����ݰ��ŵ����Ͷ���
	int nRet = RTMP_SendPacket(m_pRtmp,&packet,TRUE);

	RTMPPacket_Free(&packet);

	return nRet;
}