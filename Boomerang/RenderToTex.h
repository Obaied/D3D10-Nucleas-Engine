#ifndef RENDERTOTEX_H
#define RENDERTOTEX_H

#include "D3DUtil.h"

class RenderToTex
{
public:
	RenderToTex();
	~RenderToTex();

	void Init(UINT Width, UINT Height, bool ColorMapEnabled, 
		DXGI_FORMAT ColorFormat);
	void BuildDepthMap();
	void BuildColorMap();

	void Begin();
	void End();

	ID3D10ShaderResourceView*	GetColorMap();
	ID3D10ShaderResourceView*	GetDepthMap();
	UINT						GetWidth();
	UINT						GetHeight();

private:
	RenderToTex(const RenderToTex& rhs);
	RenderToTex& operator=(const RenderToTex& rhs);

private:
	ID3D10ShaderResourceView* mColorMapSRV;
	ID3D10RenderTargetView*   mColorMapRTV;

	ID3D10ShaderResourceView* mDepthMapSRV;
	ID3D10DepthStencilView*	  mDepthMapDSV;

	UINT mWidth;
	UINT mHeight;
	bool  mColorMapEnabled;
	D3D10_VIEWPORT mViewPort;
	DXGI_FORMAT mColorFormat;

};

#endif