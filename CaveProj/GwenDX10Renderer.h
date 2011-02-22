#include "Gwen/Gwen.h"
#include "Gwen/BaseRender.h"

#include <D3D10.h>
#include <D3DX10.h>

namespace Gwen 
{
	namespace Renderer 
	{
		class GWEN_EXPORT DirectX10 : public Gwen::Renderer::Base
		{
		public:
				DirectX10(ID3D10Device* pDevice);
				~DirectX10();

				void Begin();
				void End();

				void SetDrawColor(Color color);

				void DrawLine(int x, int y, int a, int b);
				void DrawFilledRect(Rect rect);

				void StartClip();
				void EndClip();

				void LoadTexture(Texture* pTexture);
				void FreeTexture(Texture* pTexture);
				void DrawTexturedRect(Texture* pTexture, Rect rect, float u1=0.0f, float v1=0.0f, float u2=1.0f, float v2=1.0f);

				void LoadFont(Font* pFont);
				void FreeFont(Font* pFont);
				void RenderText(Font* pFont, Point pos, const UnicodeString& text);
				Gwen::Point MeasureText(Font* pFont, const UnicodeString& text);
				
		protected:
				struct VERTEXFORMAT2D
				{
					FLOAT x, y;
					DWORD color;
					float u, v;
				};

				struct FontData
				{
					ID3DX10Font* pFont;
					int	iSpaceWidth;
				};

				void* m_pCurrentTexture;
				ID3D10Device* m_pDevice;
				DWORD m_Color;

				void Flush();
				void AddVert(int x, int y);
				void AddVert(int x, int y, float u, float v);

				static const int MaxVerts = 1024;
				VERTEXFORMAT2D m_pVerts[MaxVerts];
				int m_iVertNum;

				Gwen::Font::List m_FontList;

				ID3D10Buffer* m_pVertBuffer;
				ID3D10Effect* m_pRender2DEffect;
				ID3D10EffectTechnique* m_pRender2DTechnique;
				ID3D10EffectTechnique* m_pRender2DTexturedTechnique;
				ID3D10EffectShaderResourceVariable* m_pTextureShaderVar;
				ID3D10InputLayout* m_pInputLayout;
		};
	}
}