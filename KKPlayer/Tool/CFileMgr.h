#ifndef CFileMgr_H
#define CFileMgr_H
class CFileMgr
{
     public:
		    CFileMgr();
		    //�ж��ļ��Ƿ����
		    bool IsExist(LPCTSTR Path);
			//���Ŀ¼����������Զ�����
			bool CreateDirectory(LPCTSTR Path);
};
#endif