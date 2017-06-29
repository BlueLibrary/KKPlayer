// TestUI.cpp : ����Ӧ�ó������ڵ㡣
//

#include "stdafx.h"


int APIENTRY _tWinMain2(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{

	HRESULT hRes = CoInitialize(NULL);
	int nArgs = 0;   
	
	SComMgr * pComMgr = new SComMgr;

	SOUI::CAutoRefPtr<SOUI::IRenderFactory> pRenderFactory=NULL;
	///GDI��Ⱦ��
	pComMgr->CreateRender_GDI((IObjRef**)&pRenderFactory);
	
	///ͼƬ������
	SOUI::CAutoRefPtr<SOUI::IImgDecoderFactory> pImgDecoderFactory;
	pComMgr->CreateImgDecoder((IObjRef**)&pImgDecoderFactory);
	pRenderFactory->SetImgDecoderFactory(pImgDecoderFactory);
	
	SOUI::SApplication *theApp=NULL;
	theApp=new SApplication(pRenderFactory,hInstance);

  
	 ///��֪��ʲôԭ����win10�£��ᵼ�°ٶ����̵���˸
	HMODULE hui=LoadLibraryW(_T("kkui.dll"));
	if(hui){
		CAutoRefPtr<IResProvider>   pResProvider;
		CreateResProvider(RES_PE,(IObjRef**)&pResProvider);

		//CreateResProvider(RES_FILE,(IObjRef**)&pResProvider);
     

		BOOL ret=pResProvider->Init((WPARAM)hui,0);
		theApp->AddResProvider(pResProvider);
	}
	return 0;
}


