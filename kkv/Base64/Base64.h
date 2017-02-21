#pragma once
#include <string>

class CBase64
{
public:
CBase64();  
    ~CBase64();  
  
    /********************************************************* 
    * ����˵�������������ݽ���base64���� 
    * ����˵����[in]pIn      ��Ҫ���б�������� 
                [in]uInLen  ����������ֽ��� 
                [out]strOut ����Ľ���base64����֮����ַ��� 
    * ����ֵ  ��true����ɹ�,falseʧ�� 
    * ��  ��  ��ChenLi 
    * ��дʱ�䣺2009-02-17 
    **********************************************************/  
    bool static Encode(const unsigned char *pIn, unsigned long uInLen, std::string& strOut);  
  
   
      
    /********************************************************* 
    * ����˵�������������ݽ���base64���� 
    * ����˵����[in]strIn        ��Ҫ���н�������� 
                [out]pOut       �������֮��Ľ������� 
                [out]uOutLen    ����Ľ���֮����ֽ������� 
    * ����ֵ  ��true����ɹ�,falseʧ�� 
    * ��  ��  ��ChenLi 
    * ��дʱ�䣺2009-02-17 
    **********************************************************/  
    bool static Decode(const std::string& strIn, unsigned char *pOut, unsigned long *uOutLen) ;  
  
    /********************************************************* 
    * ����˵�������������ݽ���base64���� 
    * ����˵����[in]strIn        ��Ҫ���н�������� 
                [out]pOut       �������֮��Ľ������� 
                [out]uOutLen    ����Ľ���֮����ֽ������� 
    * ����ֵ  ��true����ɹ�,falseʧ�� 
    * ��  ��  ��ChenLi 
    * ��дʱ�䣺2009-02-17 
    **********************************************************/  
   // bool static Decode(const unsigned char *pIn, unsigned long uInLen, unsigned char *pOut, unsigned long *uOutLen) ;
private:
	 /********************************************************* 
    * ����˵�������������ݽ���base64���� 
    * ����˵����[in]pIn          ��Ҫ���б�������� 
                [in]uInLen      ����������ֽ��� 
                [out]pOut       ����Ľ���base64����֮����ַ��� 
                [out]uOutLen    ����Ľ���base64����֮����ַ������� 
    * ����ֵ  ��true����ɹ�,falseʧ�� 
    * ��  ��  ��ChenLi 
    * ��дʱ�䣺2009-02-17 
    **********************************************************/  
    bool static Encode(const unsigned char *pIn, unsigned long uInLen, unsigned char *pOut, unsigned long *uOutLen);  
};

//void CreateGuid(CString &strguid)
//{
//	char buf[64] = {0};  
//	GUID guid;  
//	if (S_OK == ::CoCreateGuid(&guid))  
//	{  
//		_snprintf(buf, sizeof(buf)  
//			, "%08X-%04X-%04x-%02X%02X-%02X%02X%02X%02X%02X%02X"  
//			, guid.Data1  
//			, guid.Data2  
//			, guid.Data3  
//			, guid.Data4[0], guid.Data4[1]  
//		, guid.Data4[2], guid.Data4[3]
//		, guid.Data4[4], guid.Data4[5]  
//		, guid.Data4[6], guid.Data4[7]  
//		);  
//	}
//	strguid.Format("%s",buf);
//}
