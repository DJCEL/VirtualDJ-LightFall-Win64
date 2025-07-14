#ifndef LIGHTFALL8_H
#define LIGHTFALL8_H

#include "VdjVideo8.h"
#include <D3D11_4.h>
#pragma comment (lib, "D3D11.lib")


//-------------------------------------------------------------------------------------------
class CLightFall8 : public IVdjPluginVideoTransition8
{
public:
	CLightFall8();
	~CLightFall8();
	HRESULT VDJ_API OnLoad();
	HRESULT VDJ_API OnGetPluginInfo(TVdjPluginInfo8 *infos);
	ULONG   VDJ_API Release();
	HRESULT VDJ_API OnDeviceInit();
	HRESULT VDJ_API OnDeviceClose();
	HRESULT VDJ_API OnDraw(float crossfader);

private:
	HRESULT Initialize_D3D11(ID3D11Device* pDevice);
	HRESULT Compose(float crossfader);
	HRESULT RenderSurface(int deck, bool bDefault);
	void OnResizeVideo();
	void VideoScaling(int deck);

	ID3D11Device* pD3DDevice;
	TVertex8 m_Vertices[2][4];
	TVertex8 m_DefaultVertices[4];
	bool m_Direct3D_On;
	int m_Width;
	int m_Height;
	int alpha;

	typedef DWORD D3DCOLOR;
	#ifndef D3DCOLOR_RGBA
		#define D3DCOLOR_RGBA(r,g,b,a) ((D3DCOLOR)((((a)&0xff)<<24)|(((r)&0xff)<<16)|(((g)&0xff)<<8)|((b)&0xff)))
	#endif
	
};

#endif
