#ifndef GlEs2Render_H_
#define GlEs2Render_H_
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <android/log.h>
#include "../KKPlayer.h"
class GlEs2Render: public CRender
{
	public:
	        GlEs2Render(KKPlayer* pPlayer);
			~GlEs2Render();
	public:	
			void  GlViewRender();
			/***
			*�Ƿ񱣳ֳ������
			*/
			void  SetKeepRatio(bool keep);
	private:		
			GLuint buildProgram(const char* vertexShaderSource, const char* fragmentShaderSource);
			
			void   GLES2_Renderer_reset();
            void   AVTexCoords_reset();
            void   AVTexCoords_cropRight(GLfloat cropRight);
			int  IniGl();
	public:
			virtual bool init(HWND hView);
			virtual void destroy();
			virtual void resize(unsigned int w, unsigned int h);
			
			virtual void render(char* buf,int width,int height,int Imgwidth);
			//���ֱ���ͼƬ
			virtual void renderBk(unsigned char* buf,int len);
			virtual void SetWaitPic(unsigned char* buf,int len);

			virtual void LoadCenterLogo(unsigned char* buf,int len);

			virtual void SetErrPic(unsigned char* buf,int len);
			virtual void ShowErrPic(bool show);
	private:
        KKPlayer* m_pPlayer;
        GLuint g_texYId;
        GLuint g_texUId;
        GLuint g_texVId;
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
        bool          m_bKeepRatio;
        

};
#endif