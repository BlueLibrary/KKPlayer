#ifndef GlEs2Render_H_
#define GlEs2Render_H_
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <android/log.h>
#include "../KKPlayer.h"
#include <jni.h>
class GlEs2Render: public IkkRender
{
	public:
	        GlEs2Render(KKPlayer* pPlayer);
			~GlEs2Render();
	public:	
			void  GlViewRender(bool ReLoad);
			/***
			*�Ƿ񱣳ֳ������
			*/
			void     SetKeepRatio(int KeepRatio);
			
			jobject  SetSurfaceTexture(JNIEnv *env);
			jobject  GetSurfaceTexture();
			jobject  GetViewSurface();
			void     setFrameAvailable(bool const available);
	private:
	        ///����һ��Surface��SurfaceTexture��
	        jobject  GenerateSurface();
            ///������õ�	
			GLuint buildProgram(const char* vertexShaderSource, const char* fragmentShaderSource);
			GLuint buildProgramSurfaceTexture(const char* vertexShaderSource, const char* fragmentShaderSource);
			
			void   GLES2_Renderer_reset();
            void   AVTexCoords_reset();
            void   AVTexCoords_cropRight(GLfloat cropRight);
			int    IniGl();
	public:
			virtual bool init(HWND hView);
			virtual void destroy();
			virtual void resize(unsigned int w, unsigned int h);
			
			virtual void render(kkAVPicInfo *Picinfo,bool wait);
			//���ֱ���ͼƬ
			virtual void renderBk(unsigned char* buf,int len);
			virtual void SetWaitPic(unsigned char* buf,int len);

			virtual void LoadCenterLogo(unsigned char* buf,int len);

			virtual void SetErrPic(unsigned char* buf,int len);
			virtual void ShowErrPic(bool show);
			virtual void SetLeftPicStr(const char *str);
			virtual void FillRect(kkBitmap img,kkRect rt,unsigned int color);
			
			
			void SetRenderImgCall(fpRenderImgCall fp,void* UserData);
	        bool GetHardInfo(void** pd3d,void** pd3ddev,int *ver);
	        void SetResetHardInfoCall(fpResetDevCall call,void* UserData);
			long long GetOnSizeTick();
	        void RetSetSizeTick();
	        void renderLock();
	        void renderUnLock();
	private:
        KKPlayer* m_pPlayer;
        GLuint g_texYId;
        GLuint g_texUId;
        GLuint g_texVId;
		GLuint g_SurfaceTextVId;
        GLuint g_glProgram;
        GLuint g_av2_texcoord;
        GLuint g_av4_position;

        GLuint m_vertexShader;
        GLuint m_fragmentShader;
		
	
		
        GLuint m_plane_textures[3];
        GLint m_us2_sampler[3];

        GLfloat m_AVVertices[8];
        GLfloat m_AVTexcoords[8];
        int m_RenderWidth;         ///����������
        int m_RenderHeight;        ///��������߶�
        int m_Picwidth;
        int m_Picheight;
        bool m_bAdJust;
       
        GLuint m_pGLHandle;
        GLuint gvPositionHandle;
        int m_Screen_Width;
        int m_Screen_Height;
        unsigned int  m_nTextureID;
		///�����������
        int           m_nKeepRatio;
        int           m_nLastKeepRatio;
		
		
		GLuint g_glSurfaceProgram;
		GLuint m_vertexShaderSurfaceTexture;
        GLuint m_fragmentShaderSurfaceTexture;
		GLuint m_textureParamHandle;
        GLuint m_texturepositionHandle;
        GLuint m_textureCoordHandle;
        GLuint m_textureTranformHandle;
	
		jmethodID updateTexImageMethodId;
		jmethodID getTimestampMethodId;
		jmethodID getTransformMtxId ;
		jmethodID setDefaultBufferSizeMethodId;
		jobject   javaSurfaceTextureObj;
		jobject   javaViewSurfaceObj;
        JNIEnv   *m_penv;
		bool     m_bfameAvailable;
		int      m_bAvPicLoaded;
		int      m_UsedViewSurfaceed;
};
#endif