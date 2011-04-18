#include "GwenDX10Renderer.h"
#include "Gwen/Texture.h"
#include "Gwen/Font.h"

#include "Util.h"
#include "ShaderBuilder.h"
#include "LeakDetector.h"

namespace Gwen
{
	namespace Renderer
	{
		DirectX10::DirectX10(ID3D10Device* pDevice) : 
			m_pDevice(pDevice),
			m_iVertNum(0),
			m_pCurrentTexture(NULL),
			m_pVertBuffer(NULL),
			m_pRender2DEffect(NULL),
			m_pRender2DTechnique(NULL),
			m_pTextureShaderVar(NULL)
		{    
			D3D10_BUFFER_DESC bufferd;
			bufferd.Usage = D3D10_USAGE_DYNAMIC;
			bufferd.ByteWidth = sizeof(m_pVerts);
			bufferd.BindFlags = D3D10_BIND_VERTEX_BUFFER;
			bufferd.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
			bufferd.MiscFlags = 0;
			m_pDevice->CreateBuffer(&bufferd, NULL, &m_pVertBuffer);

			/*
			ID3D10Blob* errorBlob;
			HRESULT hr;
			D3DX10CreateEffectFromFile("Assets/render2D.fx",
				NULL,
				NULL,
				"fx_4_0",
				D3D10_SHADER_ENABLE_STRICTNESS,
				0,
				m_pDevice,
				NULL,
				NULL,
				&m_pRender2DEffect,
				&errorBlob,
				&hr);

			if (FAILED(hr))
			{
				MessageBox(NULL, (char*)errorBlob->GetBufferPointer(), "Error", MB_OK);
			}*/

			m_pRender2DEffect = ShaderBuilder::RequestEffect("Assets/render2D", "fx_4_0", m_pDevice);


			m_pRender2DTechnique = m_pRender2DEffect->GetTechniqueByName("Render");
			m_pRender2DTexturedTechnique = m_pRender2DEffect->GetTechniqueByName("RenderTextured");
			m_pTextureShaderVar = m_pRender2DEffect->GetVariableByName("tex")->AsShaderResource();

			D3D10_PASS_DESC PassDesc;
			m_pRender2DTechnique->GetPassByIndex( 0 )->GetDesc(&PassDesc);

			D3D10_INPUT_ELEMENT_DESC layoutScene[] =
			{
				{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D10_INPUT_PER_VERTEX_DATA, 0 },
				{ "COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 8, D3D10_INPUT_PER_VERTEX_DATA, 0 },
				{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D10_INPUT_PER_VERTEX_DATA, 0 },
			};
			UINT numElementsScene = sizeof( layoutScene ) / sizeof( layoutScene[0] );
			m_pDevice->CreateInputLayout( layoutScene, numElementsScene, PassDesc.pIAInputSignature,
				PassDesc.IAInputSignatureSize, &m_pInputLayout );
		}

		DirectX10::~DirectX10()
		{
			m_pVertBuffer->Release();
		}

		void DirectX10::Begin()
		{
		}

		void DirectX10::End()
		{
			Flush();
		}

		void DirectX10::SetDrawColor(Color color)
		{
			m_Color = COLOR_ARGB(color.a, color.r, color.g, color.b);
		}

		void DirectX10::DrawLine(int x, int y, int a, int b)
		{
			Translate( x, y );
			Translate( a, b );
		}

		void DirectX10::DrawFilledRect(Rect rect)
		{
			if ( m_pCurrentTexture != NULL )
			{
				Flush();
				m_pTextureShaderVar->SetResource(NULL);
				m_pCurrentTexture = NULL;
			}	

			Translate( rect );

			AddVert(rect.x, rect.y);
			AddVert(rect.x+rect.w, rect.y);
			AddVert(rect.x, rect.y + rect.h);

			AddVert(rect.x+rect.w, rect.y);
			AddVert(rect.x+rect.w, rect.y+rect.h);
			AddVert(rect.x, rect.y + rect.h);
		}

		void DirectX10::StartClip()
		{
			Flush();
			const Gwen::Rect& rect = ClipRegion();

			D3D10_RECT r;
			r.left = ceil(((float)rect.x) * Scale());
			r.right = ceil(((float)(rect.x + rect.w)) * Scale()) + 1;
			r.top = ceil((float)rect.y * Scale());
			r.bottom = ceil(((float)(rect.y + rect.h)) * Scale()) + 1;
			m_pDevice->RSSetScissorRects(1, &r);
		}

		void DirectX10::EndClip()
		{
			Flush();
		}

		void DirectX10::LoadTexture(Texture* pTexture)
		{
			ID3D10ShaderResourceView* ptr;
			HRESULT hr;

			D3DX10_IMAGE_INFO imageinfo;
			D3DX10_IMAGE_LOAD_INFO loadinfo;
			ZeroMemory(&loadinfo, sizeof(loadinfo));
			loadinfo.Width = D3DX10_DEFAULT;
			loadinfo.Height = D3DX10_DEFAULT;
			loadinfo.BindFlags = D3DX10_DEFAULT;
			loadinfo.Filter = D3DX10_DEFAULT;
			loadinfo.Format = (DXGI_FORMAT)D3DX10_DEFAULT;
			loadinfo.Usage = (D3D10_USAGE)D3DX10_DEFAULT;
			loadinfo.pSrcInfo = &imageinfo;
			loadinfo.MipLevels = 1;
			D3DX10CreateShaderResourceViewFromFileW(m_pDevice, 
												pTexture->name.GetUnicode().c_str(),
												&loadinfo,
												NULL,
												&ptr,
												&hr);

			if (hr != S_OK)
				return;

			pTexture->data = ptr;
			pTexture->width = imageinfo.Width;
			pTexture->height = imageinfo.Height;

		}

		void DirectX10::FreeTexture(Texture* pTexture)
		{
			ID3D10ShaderResourceView* pImage = (ID3D10ShaderResourceView*)pTexture->data;
			if ( !pImage ) return;

			pImage->Release();
			pTexture->data = NULL;
		}

		void DirectX10::DrawTexturedRect(Gwen::Texture* pTexture, Rect rect, float u1, float v1, float u2, float v2)
		{
			ID3D10ShaderResourceView* pImage = (ID3D10ShaderResourceView*)pTexture->data;

			// Missing image, not loaded properly?
			if ( !pImage )
			{
				return DrawMissingImage(rect);
			}

			Translate(rect);

			if (m_pCurrentTexture != pImage)
			{
				Flush();
				m_pTextureShaderVar->SetResource(pImage);
				m_pCurrentTexture = pImage;
			}		

			AddVert(rect.x, rect.y, u1, v1);
			AddVert(rect.x+rect.w, rect.y, u2, v1);
			AddVert(rect.x, rect.y + rect.h, u1, v2);

			AddVert(rect.x+rect.w, rect.y, u2, v1);
			AddVert(rect.x+rect.w, rect.y+rect.h, u2, v2);
			AddVert(rect.x, rect.y + rect.h, u1, v2);
		}

		void DirectX10::LoadFont(Font* font)
		{
			m_FontList.push_back(font);
			// Scale the font according to canvas
			font->realsize = font->size * Scale();

			D3DX10_FONT_DESCW fd;

			memset(&fd, 0, sizeof(fd));
			wcscpy_s(fd.FaceName, LF_FACESIZE, font->facename.c_str());

			fd.Width = 0;
			fd.MipLevels = 1;
			fd.CharSet = DEFAULT_CHARSET;
			fd.Height = font->realsize * -1.0f;
			fd.OutputPrecision = OUT_DEFAULT_PRECIS;
			fd.Italic = 0;
			fd.Weight = FW_NORMAL;
			fd.Quality = font->realsize < 14 ? DEFAULT_QUALITY : CLEARTYPE_QUALITY;
			fd.PitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;

			ID3DX10Font* pD3DXFont;
			HRESULT hr = D3DX10CreateFontIndirectW(m_pDevice, &fd, &pD3DXFont);

			FontData* pFontData = new FontData;
			pFontData->pFont = pD3DXFont;

			// ID3DXFont doesn't measure trailing spaces, so we measure the width of a space here and store it
			// in the font data - then we can add it to the width when we measure text with trailing spaces.
			{
				RECT rctA = {0,0,0,0};
				pD3DXFont->DrawTextW(NULL, L"A", -1, &rctA, DT_CALCRECT | DT_LEFT | DT_TOP | DT_SINGLELINE, D3DXCOLOR(0.0f) );

				RECT rctSpc = {0,0,0,0};
				pD3DXFont->DrawTextW(NULL, L"A A", -1, &rctSpc, DT_CALCRECT | DT_LEFT | DT_TOP | DT_SINGLELINE, D3DXCOLOR(0.0f) );

				pFontData->iSpaceWidth = rctSpc.right - rctA.right * 2;
			}

			font->data = pFontData;
		}

		void DirectX10::FreeFont(Font* font)
		{
			m_FontList.remove(font);
			if (!font->data) return;

			FontData* pFontData = (FontData*)font->data;

			if (pFontData->pFont)
				pFontData->pFont->Release();

			delete pFontData;
			font->data = NULL;
		}

		void DirectX10::RenderText(Font* pFont, Point pos, const UnicodeString& text)
		{
			Flush();

			// If the font doesn't exist, or the font size should be changed
			if (!pFont->data || fabs( pFont->realsize - pFont->size * Scale() ) > 2)
			{
				FreeFont(pFont);
				LoadFont(pFont);
			}

			FontData* pFontData = (FontData*) pFont->data;
			
			Translate(pos.x, pos.y);

			RECT ClipRect = { pos.x, pos.y, 0, 0 };
			pFontData->pFont->DrawTextW(NULL, text.c_str(), -1, &ClipRect, DT_LEFT | DT_TOP | DT_NOCLIP | DT_SINGLELINE, m_Color);
		}

		Gwen::Point DirectX10::MeasureText(Font* pFont, const UnicodeString& text)
		{
			// If the font doesn't exist, or the font size should be changed
			if (!pFont->data || fabs( pFont->realsize - pFont->size * Scale() ) > 2)
			{
				FreeFont( pFont );
				LoadFont( pFont );
			}

			FontData* pFontData = (FontData*) pFont->data;

			Point size;

			if (text.empty())
			{
				RECT rct = {0,0,0,0};
				pFontData->pFont->DrawTextW(NULL, L"W", -1, &rct, DT_CALCRECT, D3DXCOLOR(0.0f));

				return Gwen::Point(0, rct.bottom);
			}

			RECT rct = {0,0,0,0};
			pFontData->pFont->DrawTextW(NULL, text.c_str(), -1, &rct, DT_CALCRECT | DT_LEFT | DT_TOP | DT_SINGLELINE, D3DXCOLOR(0.0f));

			for (int i=text.length()-1; i>=0 && text[i] == L' '; i--)
			{
				rct.right += pFontData->iSpaceWidth;
			}

			return Point(rct.right / Scale(), rct.bottom / Scale());
		}

		void DirectX10::Flush()
		{
			if (m_iVertNum > 0)
			{
				VERTEXFORMAT2D* mappedBuffer = NULL;
				m_pVertBuffer->Map(D3D10_MAP_WRITE_DISCARD, 0, (void**)&mappedBuffer);
				memcpy_s(mappedBuffer, m_iVertNum*sizeof(VERTEXFORMAT2D), m_pVerts, m_iVertNum*sizeof(VERTEXFORMAT2D));
				m_pVertBuffer->Unmap();

				if (m_pCurrentTexture != NULL)
				{
					m_pRender2DTexturedTechnique->GetPassByIndex(0)->Apply(0);
				}
				else
				{
					m_pRender2DTechnique->GetPassByIndex(0)->Apply(0);
				}
				
				UINT stride = sizeof(VERTEXFORMAT2D);
				UINT offset = 0;
				m_pDevice->IASetInputLayout(m_pInputLayout);
				m_pDevice->IASetVertexBuffers(0, 1, &m_pVertBuffer, &stride, &offset);
				m_pDevice->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

				m_pDevice->Draw(m_iVertNum, 0);

				m_iVertNum = 0;
			}
		}

		void DirectX10::AddVert(int x, int y)
		{
			if (m_iVertNum >= MaxVerts-1)
			{
				Flush();
			}

			m_pVerts[m_iVertNum].x = (float)x/640.0f - 1.0f;
			m_pVerts[m_iVertNum].y = (float)y/-360.0f + 1.0f;
			m_pVerts[m_iVertNum].color = m_Color;

			m_iVertNum++;
		}

		void DirectX10::AddVert(int x, int y, float u, float v)
		{
			if (m_iVertNum >= MaxVerts-1)
			{
				Flush();
			}

			m_pVerts[m_iVertNum].x = (float)x/640.0f - 1.0f;
			m_pVerts[m_iVertNum].y = (float)y/-360.0f + 1.0f;
			m_pVerts[m_iVertNum].u = u;
			m_pVerts[m_iVertNum].v = v;

			m_pVerts[m_iVertNum].color = m_Color;

			m_iVertNum++;
		}

	}
}