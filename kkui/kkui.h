// ���� ifdef ���Ǵ���ʹ�� DLL �������򵥵�
// ��ı�׼�������� DLL �е������ļ��������������϶���� KKUI_EXPORTS
// ���ű���ġ���ʹ�ô� DLL ��
// �κ�������Ŀ�ϲ�Ӧ����˷��š�������Դ�ļ��а������ļ����κ�������Ŀ���Ὣ
// KKUI_API ������Ϊ�Ǵ� DLL ����ģ����� DLL ���ô˺궨���
// ������Ϊ�Ǳ������ġ�
#ifdef KKUI_EXPORTS
#define KKUI_API __declspec(dllexport)
#else
#define KKUI_API __declspec(dllimport)
#endif

// �����Ǵ� kkui.dll ������
class KKUI_API Ckkui {
public:
	Ckkui(void);
	// TODO: �ڴ�������ķ�����
};

extern KKUI_API int nkkui;

KKUI_API int fnkkui(void);
