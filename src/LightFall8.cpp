#include "LightFall8.h"


//---------------------------------------------------------------------------------------------
CLightFall8::CLightFall8()
{
	pD3DDevice = nullptr;
	pD3DDeviceContext = nullptr;
	pD3DRenderTargetView = nullptr;
	memset(m_DefaultVertices, 0, 4 * sizeof(TVertex8));
	memset(m_Vertices, 0, 8 * sizeof(TVertex8));
	m_Direct3D_On = false;
	m_Width = 0;
	m_Height = 0;
	alpha = 255;
}
//---------------------------------------------------------------------------------------------
CLightFall8::~CLightFall8()
{

}
//---------------------------------------------------------------------------------------------
HRESULT VDJ_API CLightFall8::OnLoad()
{
	return S_OK;
}
//---------------------------------------------------------------------------------------------
HRESULT VDJ_API CLightFall8::OnGetPluginInfo(TVdjPluginInfo8 *infos)
{
	infos->Author = "DJ CEL";
	infos->PluginName = "LightFall";
	infos->Description = "Modify the intensity of the light during the transition";
	infos->Flags = VDJFLAG_VIDEO_MASTERONLY;
	infos->Version = "3.0 (64-bit)";
	
	return S_OK;
}
//---------------------------------------------------------------------------------------------
ULONG VDJ_API CLightFall8::Release()
{
	delete this;
	return 0;
}
//---------------------------------------------------------------------------------------------
HRESULT VDJ_API CLightFall8::OnDeviceInit()
{
	HRESULT hr = S_FALSE;

	m_Direct3D_On = true;
	m_Width = width;
	m_Height = height;

	hr = GetDevice(VdjVideoEngineDirectX11,(void**) &pD3DDevice);
	if(hr!=S_OK || pD3DDevice==nullptr) return S_FALSE;

	hr = Initialize_D3D11(pD3DDevice);
	if (hr != S_OK) return S_FALSE;
	
	return S_OK;
}
//---------------------------------------------------------------------------------------------
HRESULT VDJ_API CLightFall8::OnDeviceClose()
{
	m_Direct3D_On = false;
	return S_OK;
}
//---------------------------------------------------------------------------------------------
HRESULT VDJ_API CLightFall8::OnDraw(float crossfader)
{
	HRESULT hr = S_FALSE;
	//ID3D11ShaderResourceView *pTextureView1 = nullptr;
	//ID3D11ShaderResourceView *pTextureView2 = nullptr;

	if (width != m_Width || height != m_Height)
	{
		OnResizeVideo();
	}

	memcpy(m_Vertices[0], GetVertices(1), 4 * sizeof(TVertex8));
	memcpy(m_Vertices[1], GetVertices(2), 4 * sizeof(TVertex8));

	// GetTexture() doesn't AddRef(), so we don't need to release later
	//hr = GetTexture(VdjVideoEngineDirectX11, 1, (void**) &pTextureView1);
	//hr = GetTexture(VdjVideoEngineDirectX11, 2, (void**) &pTextureView2);

	if (!pD3DDevice) return S_FALSE;

	pD3DDevice->GetImmediateContext(&pD3DDeviceContext);
	if (!pD3DDeviceContext) return S_FALSE;
	
	pD3DDeviceContext->OMGetRenderTargets(1, &pD3DRenderTargetView, nullptr);
	if (!pD3DRenderTargetView) return S_FALSE;
	
	hr = Rendering_D3D11(pD3DDevice, pD3DDeviceContext, pD3DRenderTargetView, crossfader);

	return hr;
}
//-----------------------------------------------------------------------
void CLightFall8::OnResizeVideo()
{
	m_Width = width;
	m_Height = height;
}
//-----------------------------------------------------------------------
HRESULT CLightFall8::Initialize_D3D11(ID3D11Device* pDevice)
{
	return S_OK;
}
//---------------------------------------------------------------------------------------------
HRESULT CLightFall8::Rendering_D3D11(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, ID3D11RenderTargetView* pRenderTargetView, float crossfader)
{
	HRESULT hr = S_FALSE;
	int deck = 0;
	float compressor_rate = 0.0f;

	if (crossfader <= 0.5f) deck = 1;
	else deck = 2;

	if (crossfader <= 0.5f) compressor_rate = 1.0f - crossfader / 0.5f;
	else compressor_rate = (crossfader - 0.5f) / 0.5f;

	alpha = (int)(compressor_rate * 255.0f);

	m_Vertices[deck - 1][0].color = D3DCOLOR_RGBA(255, 255, 255, alpha);
	m_Vertices[deck - 1][1].color = D3DCOLOR_RGBA(255, 255, 255, alpha);
	m_Vertices[deck - 1][2].color = D3DCOLOR_RGBA(255, 255, 255, alpha);
	m_Vertices[deck - 1][3].color = D3DCOLOR_RGBA(255, 255, 255, alpha);

	hr = RenderSurface(deck, false);

	if (hr != S_OK) return S_FALSE;

	return S_OK;
}
//---------------------------------------------------------------------------------------------
HRESULT  CLightFall8::RenderSurface(int deck, bool bDefault)
{
	HRESULT hr = S_OK;

	VideoScaling(deck);

	if (bDefault == true)
	{
		hr = DrawDeck(deck, NULL); // (pass NULL to DrawDeck() to use the default vertices)
	}
	else
	{
		hr = DrawDeck(deck, m_Vertices[deck - 1]);
	}

	return hr;
}
//---------------------------------------------------------------------------------------------
void CLightFall8::VideoScaling(int deck)
{
	float WidthOriginalVideo, HeightOriginalVideo;
	float WidthVideo, HeightVideo;
	float NewWidthVideo, NewHeightVideo;
	float RatioOriginalVideo;
	bool b_CropVideoW,b_CropVideoH;
	float dx,dy;

	memcpy(m_DefaultVertices, GetVertices(deck), 4 * sizeof(TVertex8));

	WidthOriginalVideo = m_DefaultVertices[1].position.x - m_DefaultVertices[0].position.x;
	HeightOriginalVideo = m_DefaultVertices[3].position.y - m_DefaultVertices[0].position.y;

    b_CropVideoW = (WidthOriginalVideo !=  (float) m_Width);
	b_CropVideoH  = (HeightOriginalVideo != (float) m_Height);

	RatioOriginalVideo = HeightOriginalVideo / WidthOriginalVideo;

	WidthVideo = m_Vertices[deck - 1][1].position.x - m_Vertices[deck - 1][0].position.x;
	HeightVideo = m_Vertices[deck - 1][3].position.y - m_Vertices[deck - 1][0].position.y;

	if (b_CropVideoW)
	{
		NewWidthVideo = HeightVideo / RatioOriginalVideo;
		dx = (WidthVideo - NewWidthVideo) * 0.5f;

		m_Vertices[deck - 1][0].position.x += dx;
		m_Vertices[deck - 1][1].position.x -= dx;
		m_Vertices[deck - 1][2].position.x -= dx;
		m_Vertices[deck - 1][3].position.x += dx;
	}
	else if (b_CropVideoH)
	{
		NewHeightVideo = WidthVideo * RatioOriginalVideo;
		dy = (HeightVideo - NewHeightVideo) * 0.5f;
	
		m_Vertices[deck - 1][0].position.y += dy;
		m_Vertices[deck - 1][1].position.y += dy;
		m_Vertices[deck - 1][2].position.y -= dy;
		m_Vertices[deck - 1][3].position.y -= dy;
	}
}
