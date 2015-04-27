#include "RenderToTex.h"

RenderToTex::RenderToTex()
	:mWidth(0),mHeight(0),mColorMapEnabled(true),
	mColorFormat(DXGI_FORMAT_R8G8B8A8_UNORM),
	mColorMapSRV(0), mColorMapRTV(0), mDepthMapSRV(0), mDepthMapDSV(0)
{
}

RenderToTex::~RenderToTex()
{
	ReleaseCOM(mColorMapSRV);
	ReleaseCOM(mColorMapRTV);

	ReleaseCOM(mDepthMapSRV);
	ReleaseCOM(mDepthMapDSV);
}

void RenderToTex::Init(UINT Width, UINT Height, bool ColorMapEnabled, 
	DXGI_FORMAT ColorFormat)
{
	mWidth				= Width;
	mHeight				= Height;
	mColorMapEnabled	= ColorMapEnabled;
	mColorFormat		= ColorFormat;

	BuildDepthMap();

	if(mColorMapEnabled)
		BuildColorMap();

	mViewPort.Width		= mWidth;
	mViewPort.Height	= mHeight;
	mViewPort.MinDepth	= 0.0f;
	mViewPort.MaxDepth	= 1.0f;
	mViewPort.TopLeftX	= 0;
	mViewPort.TopLeftY	= 0;
}

void RenderToTex::BuildDepthMap()
{
	ID3D10Texture2D* Tex = 0;

	D3D10_TEXTURE2D_DESC TexDesc;
	TexDesc.ArraySize			 = 1;
	TexDesc.BindFlags			 = D3D10_BIND_SHADER_RESOURCE |
									D3D10_BIND_DEPTH_STENCIL;
	TexDesc.CPUAccessFlags		 = 0;
	TexDesc.Format				 = DXGI_FORMAT_R32_TYPELESS;
	TexDesc.Width				 = mWidth;
	TexDesc.Height				 = mHeight;
	TexDesc.MipLevels			 = 1;
	TexDesc.MiscFlags			 = 0;
	TexDesc.Usage				 = D3D10_USAGE_DEFAULT;
	TexDesc.SampleDesc.Count	 = 1;
	TexDesc.SampleDesc.Quality   = 0;

	HR(gd3dDev->CreateTexture2D(&TexDesc, 0, &Tex));

	//you wanna use the same object two times, once as a shader resource and once as a depth/stencil view. THE SAME OBJECT
	//Shader Resource View
	D3D10_SHADER_RESOURCE_VIEW_DESC SRVDesc;
	SRVDesc.Format						= DXGI_FORMAT_R32_FLOAT;
	SRVDesc.Texture2D.MipLevels			= 1;
	SRVDesc.Texture2D.MostDetailedMip	= 0;
	SRVDesc.ViewDimension				= D3D10_SRV_DIMENSION_TEXTURE2D;

	HR(gd3dDev->CreateShaderResourceView(Tex, &SRVDesc, &mDepthMapSRV));

	//Depth/Stencil View
	D3D10_DEPTH_STENCIL_VIEW_DESC DSVDesc;
	DSVDesc.Format						= DXGI_FORMAT_D32_FLOAT;
	DSVDesc.Texture2D.MipSlice			= 0;
	DSVDesc.ViewDimension				= D3D10_DSV_DIMENSION_TEXTURE2D;

	HR(gd3dDev->CreateDepthStencilView(Tex, &DSVDesc, &mDepthMapDSV));

	ReleaseCOM(Tex);
}

void RenderToTex::BuildColorMap()
{
	ID3D10Texture2D* Tex = 0;

	D3D10_TEXTURE2D_DESC TexDesc;
	TexDesc.ArraySize			 = 1;
	TexDesc.BindFlags			 = D3D10_BIND_SHADER_RESOURCE |
									D3D10_BIND_RENDER_TARGET;
	TexDesc.CPUAccessFlags		 = 0;
	TexDesc.Format				 = mColorFormat;
	TexDesc.Width				 = mWidth;
	TexDesc.Height				 = mHeight;
	TexDesc.MipLevels			 = 1;
	TexDesc.MiscFlags			 = D3D10_RESOURCE_MISC_GENERATE_MIPS;
	TexDesc.Usage				 = D3D10_USAGE_DEFAULT;
	TexDesc.SampleDesc.Count	 = 1;
	TexDesc.SampleDesc.Quality   = 0;

	HR(gd3dDev->CreateTexture2D(&TexDesc, 0, &Tex));

	//Shader Resource View
	D3D10_SHADER_RESOURCE_VIEW_DESC SRVDesc;
	SRVDesc.Format						= TexDesc.Format;
	SRVDesc.Texture2D.MipLevels			= 1;
	SRVDesc.Texture2D.MostDetailedMip	= 0;
	SRVDesc.ViewDimension				= D3D10_SRV_DIMENSION_TEXTURE2D;

	HR(gd3dDev->CreateShaderResourceView(Tex, &SRVDesc, &mColorMapSRV));

	//Depth/Stencil View
	D3D10_RENDER_TARGET_VIEW_DESC RTVDesc;
	RTVDesc.Format						= TexDesc.Format;
	RTVDesc.Texture2D.MipSlice			= 0;
	RTVDesc.ViewDimension				= D3D10_RTV_DIMENSION_TEXTURE2D;

	HR(gd3dDev->CreateRenderTargetView(Tex, &RTVDesc, &mColorMapRTV));

	ReleaseCOM(Tex);
}

void RenderToTex::Begin()
{
	ID3D10RenderTargetView* RenderTargets[] = {mColorMapRTV};
	gd3dDev->OMSetRenderTargets(1, RenderTargets, mDepthMapDSV);

	if(mColorMapRTV)
		gd3dDev->ClearRenderTargetView(mColorMapRTV, D3DCOLOR_BLACK);
	
	gd3dDev->ClearDepthStencilView(mDepthMapDSV, D3D10_CLEAR_DEPTH, 1.0f, 0);

	gd3dDev->RSSetViewports(1, &mViewPort);
};

void RenderToTex::End()
{
	if(mColorMapSRV)
		gd3dDev->GenerateMips(mColorMapSRV);
}

ID3D10ShaderResourceView* RenderToTex::GetColorMap()
{	return mColorMapSRV;	}

ID3D10ShaderResourceView* RenderToTex::GetDepthMap()
{	return mDepthMapSRV;	}

UINT RenderToTex::GetWidth()
{	return mWidth;	}
UINT RenderToTex::GetHeight()
{	return mHeight;	}