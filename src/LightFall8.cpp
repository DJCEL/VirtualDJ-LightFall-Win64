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
	m_alpha = 255;
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
	SAFE_RELEASE(pD3DRenderTargetView);
	SAFE_RELEASE(pD3DDeviceContext);
	m_Direct3D_On = false;
	return S_OK;
}
//---------------------------------------------------------------------------------------------
HRESULT VDJ_API CLightFall8::OnDraw(float crossfader)
{
	HRESULT hr = S_FALSE;
	TVertex8* vertices1 = nullptr;
	TVertex8* vertices2 = nullptr;
	//ID3D11ShaderResourceView *pTextureView1 = nullptr;
	//ID3D11ShaderResourceView *pTextureView2 = nullptr;

	if (width != m_Width || height != m_Height)
	{
		OnResizeVideo();
	}

	vertices1 = GetVertices(1);
	vertices2 = GetVertices(2);
	if (!vertices1 || !vertices2) return S_FALSE;
	
	memcpy(m_DefaultVertices[0], vertices1, 4 * sizeof(TVertex8));
	memcpy(m_DefaultVertices[1], vertices2, 4 * sizeof(TVertex8));
	
	memcpy(m_Vertices[0], vertices1, 4 * sizeof(TVertex8));
	memcpy(m_Vertices[1], vertices2, 4 * sizeof(TVertex8));
	TVertex8* pDoubleVertices[2] = { m_Vertices[0], m_Vertices[1] };

	// GetTexture() doesn't AddRef(), so we don't need to release later
	//hr = GetTexture(VdjVideoEngineDirectX11, 1, (void**) &pTextureView1);
	//hr = GetTexture(VdjVideoEngineDirectX11, 2, (void**) &pTextureView2);
	//ID3D11ShaderResourceView* pDoubleTextureView[2] = { pTextureView1, pTextureView2 };

	if (!pD3DDevice) return S_FALSE;

	pD3DDevice->GetImmediateContext(&pD3DDeviceContext);
	if (!pD3DDeviceContext) return S_FALSE;
	
	pD3DDeviceContext->OMGetRenderTargets(1, &pD3DRenderTargetView, nullptr);
	if (!pD3DRenderTargetView) return S_FALSE;
	
	hr = Rendering_D3D11(pD3DDevice, pD3DDeviceContext, pD3DRenderTargetView, pDoubleVertices, crossfader);
	if (hr != S_OK) return S_FALSE;

	return S_OK;
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
//-----------------------------------------------------------------------
void CLightFall8::Release_D3D11()
{

}
//---------------------------------------------------------------------------------------------
HRESULT CLightFall8::Rendering_D3D11(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, ID3D11RenderTargetView* pRenderTargetView, TVertex8* vertices[2], float crossfader)
{
	HRESULT hr = S_FALSE;
	//InfoTexture2D InfoRTV = {};
	//InfoTexture2D InfoSRV1 = {};
	//InfoTexture2D InfoSRV2 = {};
	//hr = GetInfoFromRenderTargetView(pRenderTargetView, &InfoRTV);
	//hr = GetInfoFromShaderResourceView(pTextureView[0], &InfoSRV1);
	//hr = GetInfoFromShaderResourceView(pTextureView[1], &InfoSRV2);

	int deck = 0;
	float fValue = 0.0f;

	if (crossfader <= 0.5f) 
	{
		deck = 1;
		fValue = 1.0f - crossfader / 0.5f;
	}
	else 
	{
		deck = 2;
		fValue = (crossfader - 0.5f) / 0.5f;
	}

	m_alpha = (int)(fValue * 255.0f);

	vertices[deck - 1][0].color = D3DCOLOR_RGBA(255, 255, 255, m_alpha);
	vertices[deck - 1][1].color = D3DCOLOR_RGBA(255, 255, 255, m_alpha);
	vertices[deck - 1][2].color = D3DCOLOR_RGBA(255, 255, 255, m_alpha);
	vertices[deck - 1][3].color = D3DCOLOR_RGBA(255, 255, 255, m_alpha);

	hr = RenderSurface(deck, false);
	if (hr != S_OK) return S_FALSE;

	return S_OK;
}
//---------------------------------------------------------------------------------------------
HRESULT  CLightFall8::RenderSurface(int deck, bool bDefaultVertices)
{
	HRESULT hr = S_OK;

	if (bDefaultVertices == true)
	{
		hr = DrawDeck(deck, NULL); // (pass NULL to DrawDeck() to use the default vertices)
	}
	else
	{
		VideoScaling(deck);
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
	bool b_CropVideoW,b_CropVideoH;
	float dx,dy;

	WidthOriginalVideo = m_DefaultVertices[1].position.x - m_DefaultVertices[0].position.x;
	HeightOriginalVideo = m_DefaultVertices[3].position.y - m_DefaultVertices[0].position.y;

   	b_CropVideoW = (WidthOriginalVideo !=  (float) m_Width);
	b_CropVideoH  = (HeightOriginalVideo != (float) m_Height);

	WidthVideo = m_Vertices[deck - 1][1].position.x - m_Vertices[deck - 1][0].position.x;
	HeightVideo = m_Vertices[deck - 1][3].position.y - m_Vertices[deck - 1][0].position.y;

	if (b_CropVideoW)
	{
		NewWidthVideo = HeightVideo / HeightOriginalVideo * WidthOriginalVideo;
		dx = (WidthVideo - NewWidthVideo) * 0.5f;

		m_Vertices[deck - 1][0].position.x += dx;
		m_Vertices[deck - 1][1].position.x -= dx;
		m_Vertices[deck - 1][2].position.x -= dx;
		m_Vertices[deck - 1][3].position.x += dx;
	}
	else if (b_CropVideoH)
	{
		NewHeightVideo = WidthVideo / WidthOriginalVideo * HeightOriginalVideo;
		dy = (HeightVideo - NewHeightVideo) * 0.5f;
	
		m_Vertices[deck - 1][0].position.y += dy;
		m_Vertices[deck - 1][1].position.y += dy;
		m_Vertices[deck - 1][2].position.y -= dy;
		m_Vertices[deck - 1][3].position.y -= dy;
	}
}
//-----------------------------------------------------------------------
HRESULT CLightFall8::GetInfoFromShaderResourceView(ID3D11ShaderResourceView* pShaderResourceView, InfoTexture2D* info)
{
	HRESULT hr = S_FALSE;
	
	D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
	ZeroMemory(&viewDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));

	pShaderResourceView->GetDesc(&viewDesc);

	DXGI_FORMAT ViewFormat = viewDesc.Format;
	D3D11_SRV_DIMENSION ViewDimension = viewDesc.ViewDimension;

	ID3D11Resource* pResource = nullptr;
	pShaderResourceView->GetResource(&pResource);
	if (!pResource) return S_FALSE;

	if (ViewDimension == D3D11_SRV_DIMENSION_TEXTURE2D)
	{
		ID3D11Texture2D* pTexture = nullptr;
		hr = pResource->QueryInterface(__uuidof(ID3D11Texture2D), (void**)&pTexture);
		if (hr != S_OK || !pTexture) return S_FALSE;

		D3D11_TEXTURE2D_DESC textureDesc;
		ZeroMemory(&textureDesc, sizeof(D3D11_TEXTURE2D_DESC));

		pTexture->GetDesc(&textureDesc);

		info->Format = textureDesc.Format;
		info->Width = textureDesc.Width;
		info->Height = textureDesc.Height;

		SAFE_RELEASE(pTexture);
	}

	SAFE_RELEASE(pResource);

	return S_OK;
}
//-----------------------------------------------------------------------
HRESULT CLightFall8::GetInfoFromRenderTargetView(ID3D11RenderTargetView* pRenderTargetView,InfoTexture2D* info)
{
	HRESULT hr = S_FALSE;
	
	D3D11_RENDER_TARGET_VIEW_DESC viewDesc;
	ZeroMemory(&viewDesc, sizeof(D3D11_RENDER_TARGET_VIEW_DESC));

	pRenderTargetView->GetDesc(&viewDesc);

	DXGI_FORMAT ViewFormat = viewDesc.Format;
	D3D11_RTV_DIMENSION ViewDimension = viewDesc.ViewDimension;

	ID3D11Resource* pResource = nullptr;
	pRenderTargetView->GetResource(&pResource);
	if (!pResource) return S_FALSE;

	if (ViewDimension == D3D11_RTV_DIMENSION_TEXTURE2D)
	{
		ID3D11Texture2D* pTexture = nullptr;
		hr = pResource->QueryInterface(__uuidof(ID3D11Texture2D), (void**)&pTexture);
		if (hr != S_OK || !pTexture) return S_FALSE;

		D3D11_TEXTURE2D_DESC textureDesc;
		ZeroMemory(&textureDesc, sizeof(D3D11_TEXTURE2D_DESC));

		pTexture->GetDesc(&textureDesc);

		info->Format = textureDesc.Format;
		info->Width = textureDesc.Width;
		info->Height = textureDesc.Height;

		SAFE_RELEASE(pTexture);
	}

	SAFE_RELEASE(pResource);

	return S_OK;
}
