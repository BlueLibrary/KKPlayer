//��װflv�ļ�
#ifndef FlvEncode_H_
#define FlvEncode_H_
namespace KKMEDIA
{
#pragma pack(push,1)
typedef struct _FLV_HEADER
{
	char FLV[3];//={0x46,0x4c,0x56};
	char Ver;   //�汾��
	
	char StreamInfo;// ����Ƶ������Ƶ����0x01 | 0x04��0x05��
	int HeaderLen; 
} FLV_HEADER;

typedef struct _FLV_TAG_HEADER
{
	int  PreTagLen;  /*******��һ��Tag����********/         //   4
    char TagType;   //��Ƶ��0x8������Ƶ��0x9�����ű���0x12��  //  1
	char TagDtatLen[3];    /*******����������*******/         // 3
    char Timestamp[3];   //ʱ���                               //  3
	char ExpandTimeStamp;//����չʱ���                      //  1
	char streamID[3];    //streamID��������Ϊ0                   //  3
}FLV_TAG_HEADER;

typedef struct _METADATA
{

	double filesize;/**********�н����***********/
	double duration;/**********�н����***********/
	// video, must be h264 type
	unsigned int	nWidth;
	unsigned int	nHeight;
	unsigned int	nFrameRate;		// fps
	unsigned int	nVideoDataRate;	// bps
	unsigned int	nSpsLen;
	unsigned char	Sps[1024];
	unsigned int	nPpsLen;
	unsigned char	Pps[1024];
	unsigned int    nVideoCodecId;
	int nVideoFmt;

	// audio, must be aac type
	bool	        bHasAudio;
	unsigned int	nAudioDatarate;
	unsigned int	nAudioSampleRate;
	unsigned int	nAudioSampleSize;
	int				nAudioFmt;
	unsigned int	nAudioChannels;
	char		    pAudioSpecCfg;
	unsigned int	nAudioSpecCfgLen;
	unsigned int    nAudioCodecId;

}METADATA;
//AVCDecorderConfigurationRecord

//AVCVideoPacket Format�� AVCPacketType(8)+ CompostionTime(24) +Data
//AVCPacketTypeΪ�������ͣ�
//	���AVCPacketType=0x00��ΪAVCSequence Header��
//	���AVCPacketType=0x01��ΪAVC NALU��
//	���AVCPacketType=0x02��ΪAVC end ofsequence
//	CompositionTimeΪ���ʱ�����
//	���AVCPacketType=0x01�� Ϊ���ʱ�����
//	��������Ϊ0��
//	DataΪ�������ݣ�
//	���AVCPacketType=0x00��ΪAVCDecorderConfigurationRecord��
//	���AVCPacketType=0x01��ΪNALUs��
//	���AVCPacketType=0x02��Ϊ�ա�
typedef struct _AVC_DEC_CON_REC
{
	char cfgVersion;//configurationVersion �汾��
	char avcProfile;//AVCProfileIndication sps[1]
	char profileCompatibility;//profile_Compatibility sps[2]
	char avcLevel;//AVCLevelIndication     sps[3]
	//lengthSizeMinusOne:indicates the length in bytes of the NALUnitLength field in an AVC video
	char reserved6_lengthSizeMinusOne2;
	char reserved3_numOfSPS5;//����
	long spsLength;//sequenceParameterSetLength
	void *sps;
	char numOfPPS;//����
	long ppsLength;
	void *pps;
}AVC_DEC_CON_REC;

#pragma pack(pop)

struct flv_packet
{
      unsigned  char *buf;
	  int             bufLen;
	  int taglen;
};
class FlvEncode
{
public:
	     FlvEncode(void);
	     ~FlvEncode(void);
		 //����һ��FLVHeader
		 FLV_HEADER CreateFLVHeader(char StreamInfo);
		
		 void *CreateFLVMetaData(METADATA *lpMetaData,int &OutLen);
		 void *FlvMemcpy(void* dest,size_t destLen, const void* src, size_t n);
		 int GetNALULen(const unsigned  char *src,int srcLen);
		 //��ȡh264�ָ������
		 int GetH264SeparatorLen(const unsigned  char *src,int srcLen);
         flv_packet GetFlvHeader();
		 flv_packet GetVideoPacket(const unsigned  char *src,int srcLen,unsigned int pts,int &nPreTagLen);
		 
};
}
#endif