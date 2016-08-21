#include <vector>
#include <queue>
#include "FlvEncode.h"
#include "../KKLock.h"
#ifndef AV_FLV_H
#define AV_FLV_H
	typedef struct _S_Data_Info{
		void *pData;
		int Len;
		int AVType; //0 ��Ƶ��1��Ƶ��2����
	}S_Data_Info;
	class CAV_Flv
	{
		   public:
				CAV_Flv(void);
				~CAV_Flv(void);
				void CreateFlvHeadrInfo();
				int LocalRecord(bool IsVideo,unsigned char* avbuffer,unsigned int avbufferlen,int avpts);
			private:
				/******׼���õ�����*****/
				std::queue<S_Data_Info*> m_VideoData_Queue;
				S_Data_Info* pTempCache;
				
				CKKLock m_AVQueueLock;
			
		
				FILE* m_pFlvFile;
				bool m_LocalAVFirst;
				bool m_LocalAVOk;
			
				int MetaDataPos;
				int MetaDataTagLen;
				KKMEDIA::METADATA MetaData;
				int m_OpenState;

				//ǰһ��tag����
				int m_PreTagLen;
				//���¼���
				int m_ReCalPreTagLen;
				//��ʱ�洢������
				std::vector<S_Data_Info*> m_Data_Vect;
				S_Data_Info* pVideoInfo;
				S_Data_Info* pAudioInfo;
				bool SPSFist;
	};
#endif