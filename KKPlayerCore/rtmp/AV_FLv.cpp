#include "FlvEncode.h"
#include "AV_FLv.h"
KKMEDIA::FlvEncode flvEc;
CAV_Flv::CAV_Flv(void)
{

	pTempCache=NULL;
	m_pFlvFile=NULL;
	m_LocalAVFirst=false;

    SPSFist=true;
	MetaDataPos=0;

	m_PreTagLen=0;
	//���¼���
	m_ReCalPreTagLen=0;
	pVideoInfo=NULL;
	pAudioInfo=NULL;

	m_LocalAVFirst=true;
	m_LocalAVOk=true;
	m_gnHear=0;
}

CAV_Flv::~CAV_Flv(void)
{
	
}
	
void CAV_Flv::CreateFlvHeadrInfo()
{

	
	KKMEDIA::FLV_HEADER flvheader=flvEc.CreateFLVHeader(0x05);
	char *pflvheader=(char*)::malloc(sizeof(flvheader));
	memcpy(pflvheader,&flvheader,sizeof(flvheader));

	S_Data_Info *pAVInfo2=(S_Data_Info *)::malloc(sizeof(S_Data_Info));
	pAVInfo2->AVType=2;
	pAVInfo2->pData=pflvheader;
	pAVInfo2->Len=sizeof(flvheader);
	m_AVQueueLock.Lock();
	m_VideoData_Queue.push(pAVInfo2);
	m_AVQueueLock.Unlock();/**/



	memset(&MetaData,0,sizeof(KKMEDIA::METADATA));
    /********************/
    MetaData.nVideoCodecId=7;
	MetaData.nAudioCodecId=2;
	MetaData.nAudioSampleRate=44100;
	MetaData.nAudioSampleSize=16;
	


	int OutLen=0;
	void *pMetaDatafo=flvEc.CreateFLVMetaData(&MetaData,OutLen);

	KKMEDIA::FLV_TAG_HEADER Tag_Head;
	memset(&Tag_Head,0,sizeof(Tag_Head));
	Tag_Head.TagType=0x12;
	flvEc.FlvMemcpy(&Tag_Head.TagDtatLen,3,&OutLen,3);

	m_ReCalPreTagLen=sizeof(KKMEDIA::FLV_TAG_HEADER)-4;

	int llxx=sizeof(KKMEDIA::FLV_TAG_HEADER)+OutLen;
	char *pData =(char*)::malloc(llxx);
	memcpy(pData,&Tag_Head,sizeof(KKMEDIA::FLV_TAG_HEADER));
	memcpy(pData+sizeof(KKMEDIA::FLV_TAG_HEADER),pMetaDatafo,OutLen);

	m_ReCalPreTagLen+=OutLen;

	MetaDataTagLen=m_ReCalPreTagLen;

	S_Data_Info *pAVInfo=(S_Data_Info *)::malloc(sizeof(S_Data_Info));
	pAVInfo->AVType=2;
	pAVInfo->pData=pData;
	pAVInfo->Len=llxx;
	m_AVQueueLock.Lock();
	m_VideoData_Queue.push(pAVInfo);
	m_AVQueueLock.Unlock();/**/

	m_PreTagLen=OutLen+11;
	free(pMetaDatafo);
}
	//�ص�ο�http://blog.csdn.net/yeyumin89/article/details/7932368
	//http://blog.csdn.net/zqf_office/article/details/50868231
	//http://blog.csdn.net/jwybobo2007/article/details/9221657
	//��ȡ��Ƶ����װ��flv��ʽ
	int CAV_Flv::LocalRecord(bool IsVideo,unsigned char* avbuffer,unsigned int avbufferlen,int avpts)
	{
		if(m_gnHear==0)
		{
			CreateFlvHeadrInfo();
			m_gnHear=1;
		}
	
		{
			
			if (IsVideo)
			{
				//¼h264��ʽ������
				if (NULL != m_pFlvFile)
				{
					//
					int TagDataLen=avbufferlen+1000;
					char *pTagData =(char *)::malloc(TagDataLen);
					memset(pTagData,0,TagDataLen);

					char* pdata=(char* )avbuffer;
					//��Ƶ������
					int AvDataLen=avbufferlen;
					int Index= flvEc.GetH264SeparatorLen(pdata,4);
					char H264Type=0x00;
					
					//ʱ���
					int Video_Ts=avpts;
					
					if(Index>0)
					{
						AvDataLen-=4;
						char AVCPacket[5]={0x00};
						memset(&AVCPacket,0,5);
						char *pDataNALU=(pdata+Index);
						char AVType=0xFF;
						char tpp=(*pDataNALU)&0x1f;
						//�����˶�� sps
						if(((*pDataNALU)&0x1f)==0x07)
						{
							//�����һ�γ���sps.
							if(SPSFist)
							{
								SPSFist=false;
								AVType=0x17;//67= 0110,0111 &0000,1111=7
								//AVC ������Ϣ
								KKMEDIA::AVC_DEC_CON_REC avc_dec_con_rec;
								avc_dec_con_rec.cfgVersion=0x01;

								avc_dec_con_rec.reserved6_lengthSizeMinusOne2=0xFF;//11111,111
								avc_dec_con_rec.reserved3_numOfSPS5=0xE1;//000,00001->111,0,0001=
								avc_dec_con_rec.numOfPPS=1;

								char *temp=pDataNALU;
								avc_dec_con_rec.spsLength=flvEc.GetNALULen(temp,100);
								char *pSpsData=(char*)::malloc(avc_dec_con_rec.spsLength);
								memset(pSpsData,0,avc_dec_con_rec.spsLength);

								//sps����
								memcpy(pSpsData,temp,avc_dec_con_rec.spsLength);
								avc_dec_con_rec.sps=pSpsData;

								temp+=avc_dec_con_rec.spsLength;
								AvDataLen-=avc_dec_con_rec.spsLength;
								int TempIndex=flvEc.GetH264SeparatorLen(temp,4);
								if(TempIndex>0)
								{
									AvDataLen-=TempIndex;
									//TempIndex++;
									temp+=TempIndex;
									//pps
									if(0x08==((*temp) & 0x1f))
									{
										//temp+=1;
										avc_dec_con_rec.ppsLength=flvEc.GetNALULen(temp,100);
										char *pPPSData=(char*)::malloc(avc_dec_con_rec.ppsLength);
										memset(pPPSData,0,avc_dec_con_rec.ppsLength);
										memcpy(pPPSData,temp,avc_dec_con_rec.ppsLength);
										avc_dec_con_rec.pps=pPPSData;
										AvDataLen-=avc_dec_con_rec.ppsLength;
									}
								}


								KKMEDIA::FLV_TAG_HEADER Tag_Head;
								memset(&Tag_Head,0,sizeof(Tag_Head));
								flvEc.FlvMemcpy(&Tag_Head.PreTagLen,4,&m_PreTagLen,4);
								KKMEDIA::FLV_TAG_HEADER *ppppp=&Tag_Head;
								Tag_Head.TagType=0x09;
								Tag_Head.ExpandTimeStamp=0;

								int TempLen=0;
								//Tagͷ
								memcpy(pTagData+TempLen,&Tag_Head,sizeof(KKMEDIA::FLV_TAG_HEADER));
								TempLen+=sizeof(KKMEDIA::FLV_TAG_HEADER);


								memset(&AVCPacket,0,5);
								//��Ƶ����
								AVCPacket[0]=AVType;//0x17
								memcpy(pTagData+TempLen,&AVCPacket,5);
								TempLen+=5;

								//Version
								memcpy(pTagData+TempLen,&avc_dec_con_rec.cfgVersion,1);
								TempLen+=1;

								char *pSps=(char*)avc_dec_con_rec.sps;
								avc_dec_con_rec.avcProfile=*(pSps+1);
								//profile
								memcpy(pTagData+TempLen,&avc_dec_con_rec.avcProfile,1);
								TempLen+=1;
								//profile compat
								avc_dec_con_rec.profileCompatibility=*(pSps+2);
								memcpy(pTagData+TempLen,&avc_dec_con_rec.profileCompatibility,1);
								TempLen+=1;
								//Level
								avc_dec_con_rec.avcLevel=*(pSps+3);
								memcpy(pTagData+TempLen,&avc_dec_con_rec.avcLevel,1);
								TempLen+=1;
								///* 6 bits reserved (111111) + 2 bits nal size length - 1 (11) */
								avc_dec_con_rec.reserved6_lengthSizeMinusOne2=0xFF;
								memcpy(pTagData+TempLen,&avc_dec_con_rec.reserved6_lengthSizeMinusOne2,1);
								TempLen+=1;
								//* 3 bits reserved (111) + 5 bits number of sps (00001) */
								avc_dec_con_rec.reserved3_numOfSPS5=0xe1;
								memcpy(pTagData+TempLen,&avc_dec_con_rec.reserved3_numOfSPS5,1);
								TempLen+=1;


								//sps����
								//memcpy(pTagData+TempLen,&avc_dec_con_rec.spsLength,2);
								flvEc.FlvMemcpy(pTagData+TempLen,2,&avc_dec_con_rec.spsLength,2);
								TempLen+=2;
								//sps����
								memcpy(pTagData+TempLen,avc_dec_con_rec.sps,avc_dec_con_rec.spsLength);
								TempLen+=avc_dec_con_rec.spsLength;

								//pps
								//���� ��λ�ֽ�
								avc_dec_con_rec.numOfPPS=1;
								memcpy(pTagData+TempLen,&avc_dec_con_rec.numOfPPS,1);
								TempLen+=1;

								//pps ���� ��ˣ���λ���ֽ�
								//memcpy(pTagData+TempLen,&avc_dec_con_rec.ppsLength,2);
								flvEc.FlvMemcpy(pTagData+TempLen,2,&avc_dec_con_rec.ppsLength,2);
								TempLen+=2;
								//����
								memcpy(pTagData+TempLen,avc_dec_con_rec.pps,avc_dec_con_rec.ppsLength);
								TempLen+=avc_dec_con_rec.ppsLength;

								m_PreTagLen=TempLen-4;//(-4,)

								int TagDataLen=TempLen-15;//(11+4)

								//Tag ����������
								flvEc.FlvMemcpy(pTagData+5,3,&TagDataLen,3);

								//fwrite(pTagData,TempLen, 1, local_recod->fp_h264_420_major);
								{
									S_Data_Info* SD =(S_Data_Info*)::malloc(sizeof(S_Data_Info));
									SD->AVType=0;
									char *pData =(char *)::malloc(TempLen);
									memset(pData,0,TempLen);
									memcpy(pData,pTagData,TempLen);
									SD->pData=pData;
									SD->Len=TempLen;
									m_Data_Vect.push_back(SD);
									pVideoInfo=SD;
								}


								//JNVido
								pDataNALU=temp+avc_dec_con_rec.ppsLength;
							}else{//ȥ��sps,pps.
								AVType=0x17;
								//ȥ��sps;
								int Length=flvEc.GetNALULen(pDataNALU,100);
								if(Length>0)
								{
									AvDataLen-=Length;
									pDataNALU+=Length;
									Length=0;
								}
								//pps
								Length=flvEc.GetH264SeparatorLen(pDataNALU,4);
								if(Length>0)
								{
									AvDataLen-=Length;
									pDataNALU+=Length;
									Length=0;
									Length=flvEc.GetNALULen(pDataNALU,100);
									if(Length>0)
									{
										AvDataLen-=Length;
										pDataNALU+=Length;
										Length=0;
									}
								}

							}
							//��һ��֡���
							Index= flvEc.GetH264SeparatorLen(pDataNALU,4);
							if(Index>0)
							{
								AvDataLen-=Index;
								pDataNALU+=Index;
								if(*pDataNALU==0x06)//��ǿ��Ϣ֡ sei
								{
									H264Type=0x06;
								}else if(((*pDataNALU)& 0x1f)==0x05){
									H264Type=0x05;
								}
								if(AvDataLen<=0)
								{
									return -1;
								}
							}
							memset(pTagData,0,TagDataLen);
						}

						char *pSeiData=NULL;
						int SEILen=0;
						char seiLen[4]={0};
LOOP1: 					
						if(0x08==*pDataNALU & 0x1f)
						{
							H264Type=0x00;
							AVType=0x17;//PPS
							return -1;
						}else if(((*pDataNALU) & 0x1f)==0x05)
						{
							H264Type=0x05;
							AVType=0x17;//�ؼ�֡
							AVCPacket[0]=0x01;
						}else if(*pDataNALU==0x41)
						{
							H264Type=0x41;
							AVType=0x27;       //P֡ ��Ҫ         type = 1 
							AVCPacket[0]=0x01;
						}else if(*pDataNALU==0x01)
						{
							H264Type=0x00;
							AVType=0x27; 	// B֡     ����Ҫ        type = 1 
							AVCPacket[0]=0x01;
						}else if(*pDataNALU==0x06)//��ǿ��Ϣ֡ sei
						{
							//SEI��H.264����ĸ�����ǿ��ϢNALU�����Խ�������û�а��������ṩһЩ���������Ʋ�������Ϣ�� FLVû��һ��Tag��������SEI���ݣ�����SEI���ݺͽ�������Ǹ���ƵNALU���ݴ���ͬһ��Video Tag���档
							AVType=0x27; 
							H264Type=0x06;
							AVCPacket[0]=0x01;
						}else if(((*pDataNALU) & 0x1f)==0x01)
						{
							H264Type=0x00;
							AVType=0x27; 
						}else
						{
							H264Type=0x00;
							return -1;
						}

						//SEI ��ǿ֡���϶�������һ֡
						if (H264Type==0x06)
						{
							AVCPacket[0]=0x01;
							AVCPacket[1]=0x00;
							AVCPacket[2]=0x00;
							AVCPacket[3]=0x00;
							AVCPacket[4]=0x00;
							memset(seiLen,0,4);


							SEILen=flvEc.GetNALULen(pDataNALU,AvDataLen);
							if(SEILen>0)
							{
								flvEc.FlvMemcpy(seiLen,4,&SEILen,4);
								pSeiData=(char *)::malloc(SEILen);
								memcpy(pSeiData,pDataNALU,SEILen);
								pDataNALU+=SEILen;
								AvDataLen-=SEILen;
							}
							//�õ���һ֡
							Index= flvEc.GetH264SeparatorLen(pDataNALU,4);
							if(Index>0)
							{
								AvDataLen-=Index;
								pDataNALU+=Index;
							}
							goto LOOP1;
						}

						int lx=0;
						//
						int datalen=0;


						KKMEDIA::FLV_TAG_HEADER Tag_Head;
						memset(&Tag_Head,0,sizeof(Tag_Head));
						//ǰһ��Tag����
						flvEc.FlvMemcpy(&Tag_Head.PreTagLen,4,&m_PreTagLen,4);
						flvEc.FlvMemcpy(&Tag_Head.Timestamp,3,&Video_Ts,3);
						//flvEc.FlvMemcpy(&Tag_Head.streamID,3,&stream.wStreamId,3);

						KKMEDIA::FLV_TAG_HEADER *ppppp=&Tag_Head;
						Tag_Head.TagType=0x09;

						memcpy(pTagData+datalen,&Tag_Head,sizeof(KKMEDIA::FLV_TAG_HEADER));
						datalen+=sizeof(KKMEDIA::FLV_TAG_HEADER);


						//AVCPacket
						{//��Ƶ����
							AVCPacket[0]=AVType;
							AVCPacket[1]=0x01;
							memcpy(pTagData+datalen,AVCPacket,5);
							datalen+=5;
						}
						//AVC ��ʽ SEI ����
						if (H264Type==0x06)//SEI��Ϣ
						{
							memcpy(pTagData+datalen,seiLen,4);
							datalen+=4;
							memcpy(pTagData+datalen,pSeiData,SEILen);
							datalen+=datalen;/**/
						}

						//NALU��Ԫ����,���
						flvEc.FlvMemcpy(pTagData+datalen,4,&AvDataLen, 4);
						datalen+=4;

						//NALU����
						memcpy(pTagData+datalen,pDataNALU, AvDataLen);
						datalen+=AvDataLen;

						m_PreTagLen=datalen-4;
						int TagDataLen=datalen-15;//(11+4)
						//Tag��������С
						flvEc.FlvMemcpy(pTagData+5,3,&TagDataLen,3);
						{
							S_Data_Info* SD =(S_Data_Info*)::malloc(sizeof(S_Data_Info));
							SD->AVType=0;
							char *pData =(char *)::malloc(datalen);
							memset(pData,0,datalen);
							memcpy(pData,pTagData,datalen);
							SD->pData=pData;
							SD->Len=datalen;
							m_Data_Vect.push_back(SD);

						}
						free(pTagData);
					}
				}

			}
			else
			{
				//http://www.cnblogs.com/haibindev/archive/2011/12/29/2305712.html
				int pTagBufferLen=avbufferlen+1000;
				char *pTagBuffer=(char *)::malloc(pTagBufferLen);
				memset(pTagBuffer,0,pTagBufferLen);
				KKMEDIA::FLV_TAG_HEADER Tag_Head;
				memset(&Tag_Head,0,sizeof(Tag_Head));
				//ǰһ��Tag����
				//ǰ4bits��ʾ��Ƶ��ʽ��ȫ����ʽ�뿴�ٷ��ĵ�����
				int Audio_Ts=avpts;
				flvEc.FlvMemcpy(&Tag_Head.PreTagLen,4,&m_PreTagLen,4);
				flvEc.FlvMemcpy(&Tag_Head.Timestamp,3,&Audio_Ts,3);
				//flvEc.FlvMemcpy(&Tag_Head.streamID,3,&stream.wStreamId,3);

				Tag_Head.TagType=0x08;
				int datalen=0;
				memcpy(pTagBuffer+datalen,&Tag_Head,sizeof(KKMEDIA::FLV_TAG_HEADER));
				datalen+=sizeof(KKMEDIA::FLV_TAG_HEADER);
				//ǰ4bits��ʾ��Ƶ��ʽ��ȫ����ʽ�뿴�ٷ��ĵ�����
				//1 -- ADPCM
				//2 -- MP3
				//4 -- Nellymoser 16-kHz mono
				//5 -- Nellymoser 8-kHz mono
				//10 -- AAC
				//������bits��ʾsamplerate��
				//��0 -- 5.5KHz
				//��1 -- 11kHz
				//��2 -- 22kHz
				//��3 -- 44kHz 1100=0x0C
				//����1bit��ʾ�������ȣ�
				//��0 -- snd8Bit
				//��1 -- snd16Bit
				//����1bit��ʾ���ͣ�
				//��0 -- sndMomo
				//��1 -- sndStereo  
				char TagAudio=0xAF; //1010,11,1,1
				//TagAudio &=0x0C;//3
				//TagAudio &=0x02;//1
				//TagAudio &=0x01;//sndStereo
				memcpy(pTagBuffer+datalen,&TagAudio,1);
				datalen++;


				//��Ƶ������Ϣ
				if(m_LocalAVFirst)
				{
					int Tempidx=datalen;
					m_LocalAVFirst=false;
					char AACPacketType=0x00;//012->
					memcpy(pTagBuffer+datalen,&AACPacketType,1);
					datalen++;
					char AudioSpecificConfig[2]={0x12,0x10};


					memcpy(pTagBuffer+datalen,&AudioSpecificConfig,2);
					datalen+=2;

					memcpy(pTagBuffer+datalen,avbuffer,3);
					datalen+=3;

					m_PreTagLen=datalen-4;
					int TagDataLen=m_PreTagLen-11;//(11)

					//Tag ��������С
					flvEc.FlvMemcpy(pTagBuffer+5,3,&TagDataLen,3);

					if(m_pFlvFile!=NULL)
					{
						{
							S_Data_Info* SD =(S_Data_Info*)::malloc(sizeof(S_Data_Info));
							SD->AVType=1;
							char *pData =(char *)::malloc(datalen);
							memset(pData,0,datalen);
							memcpy(pData,pTagBuffer,datalen);
							SD->pData=pData;
							SD->Len=datalen;

							m_Data_Vect.push_back(SD);
							pAudioInfo=SD;
                        }

						flvEc.FlvMemcpy(pTagBuffer,4,&m_PreTagLen,4);
						AACPacketType=0x01;
						memcpy(pTagBuffer+Tempidx,&AACPacketType,1);
						Tempidx++;

						memcpy(pTagBuffer+Tempidx,avbuffer+7,avbufferlen-7);
						Tempidx+=avbufferlen-7;

						m_PreTagLen=Tempidx-4;
						TagDataLen=m_PreTagLen-11;//(11+4)
						flvEc.FlvMemcpy(pTagBuffer+5,3,&TagDataLen,3);
						{
							S_Data_Info* SD2 =(S_Data_Info*)::malloc(sizeof(S_Data_Info));
							SD2->AVType=1;
							char *pDataX =(char *)::malloc(Tempidx);
							memset(pDataX,0,datalen);
							memcpy(pDataX,pTagBuffer,Tempidx);
							SD2->pData=pDataX;
							SD2->Len=Tempidx;

							m_Data_Vect.push_back(SD2);
						}
					}

				}else{
					char AACPacketType=0x01;
					memcpy(pTagBuffer+datalen,&AACPacketType,1);
					datalen++;

					memcpy(pTagBuffer+datalen,avbuffer+7,avbufferlen-7);
					datalen+=avbufferlen-7;

					m_PreTagLen=datalen-4;
					int TagDataLen=m_PreTagLen-11;//(11+4)
					//Tag ��������С
					flvEc.FlvMemcpy(pTagBuffer+5,3,&TagDataLen,3);
					if(m_pFlvFile!=NULL)
					{
						S_Data_Info* SD =(S_Data_Info*)::malloc(sizeof(S_Data_Info));
						SD->AVType=1;
						char *pData =(char *)::malloc(datalen);
						memset(pData,0,datalen);
						memcpy(pData,pTagBuffer,datalen);
						SD->pData=pData;
						SD->Len=datalen;
						m_Data_Vect.push_back(SD);
					}
				}


				free(pTagBuffer);
			}
			if(pAudioInfo!=NULL&&pVideoInfo!=NULL&&m_pFlvFile!=NULL)//
			{
				if(m_LocalAVOk)
				{
					m_LocalAVOk=false;
					//m_ReCalPreTagLen=0;
					//��Ƶ������Ϣ
					flvEc.FlvMemcpy(pAudioInfo->pData,4,&m_ReCalPreTagLen,4);
					fwrite(pAudioInfo->pData,pAudioInfo->Len,1,m_pFlvFile);
					m_ReCalPreTagLen=pAudioInfo->Len-4;
					
					//	fclose(m_pFlvFile);
					//��Ƶ������Ϣ
					flvEc.FlvMemcpy(pVideoInfo->pData,4,&m_ReCalPreTagLen,4);
					fwrite(pVideoInfo->pData,pVideoInfo->Len,1,m_pFlvFile);
					m_ReCalPreTagLen=pVideoInfo->Len-4;
					
					
					//��Ƶ������Ϣ
					S_Data_Info* pAAVInfo=(S_Data_Info*)::malloc(sizeof(S_Data_Info));
					pAAVInfo->AVType=1;
					pAAVInfo->Len=pAudioInfo->Len;
					pAAVInfo->pData=pAudioInfo->pData;
					m_AVQueueLock.Lock();
					m_VideoData_Queue.push(pAAVInfo);
					m_AVQueueLock.Unlock();

					//��Ƶ������Ϣ
					S_Data_Info* pVAVInfo=(S_Data_Info*)::malloc(sizeof(S_Data_Info));
					pVAVInfo->AVType=1;
					pVAVInfo->Len=pVideoInfo->Len;
					pVAVInfo->pData=pVideoInfo->pData;
					m_AVQueueLock.Lock();
					m_VideoData_Queue.push(pVAVInfo);
					m_AVQueueLock.Unlock();
				

				}
				if(!m_LocalAVOk&&!m_LocalAVFirst)
				{
					std::vector<S_Data_Info*>::iterator It=m_Data_Vect.begin();
					for(;It!=m_Data_Vect.end();)
					{
						S_Data_Info* pInfo=*It;
						if(pInfo==pAudioInfo||pInfo==pVideoInfo)
						{

						}else
						{
							flvEc.FlvMemcpy(pInfo->pData,4,&m_ReCalPreTagLen,4);
							fwrite( pInfo->pData, pInfo->Len,1,m_pFlvFile);
							m_ReCalPreTagLen=pInfo->Len-4;
							m_AVQueueLock.Lock();
							m_VideoData_Queue.push(pInfo);
							m_AVQueueLock.Unlock();
							//free(pInfo->pData);
							//free(pInfo);
						}
						It=m_Data_Vect.erase(It);
					}
				}
			}
		}
		return 0;
	}