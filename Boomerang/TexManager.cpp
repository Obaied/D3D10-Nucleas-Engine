#include "TexManager.h"
#include <fstream>
#include <stdint.h>

TexManager& GetTexManager()
{
	static TexManager tm;

	return tm;
}

TexManager::TexManager()
{
	CreateRandomTexture();
	CreateNoiseTexture();
}

TexManager::~TexManager()
{
	for(size_t i = 0; i < Textures.size(); i++)
		ReleaseCOM(Textures[i].RV);
	
	ReleaseCOM(mRandomTexture.RV);
}

void TexManager::Init()
{
	
}

void TexManager::DumpInfo()
{
	std::wofstream fout("Textures.txt");

	fout << "Num of textures: " << Textures.size() << std::endl;

	
	for(UINT i = 0; i < Textures.size(); i++)
		fout << "Texture " << i << ":" << Textures[i].SrcName << std::endl;

	fout << std::endl;

}

ID3D10ShaderResourceView* TexManager::CreateTexture2D(std::wstring SrcName)
{
	for(size_t i = 0; i < Textures.size(); i++)
		if(SrcName == Textures[i].SrcName)
			return Textures[i].RV;

	ID3D10ShaderResourceView* rv = 0;

	HR(D3DX10CreateShaderResourceViewFromFile(gd3dDev, SrcName.c_str(), 0,0,
		&rv, 0));

	Texture t(rv, SrcName, TEXTURE_TYPE_2D);
	Textures.push_back(t);

	return rv;
}

ID3D10ShaderResourceView* TexManager::CreateTextureArray(std::wstring ArrayName,
	StringVector& SrcNames)
{
	for(size_t i = 0; i < Textures.size(); i++)
		if(ArrayName == Textures[i].SrcName)
			return Textures[i].RV;

	std::vector<ID3D10Texture2D*> SrcTex(SrcNames.size(),0);
	for(UINT i = 0; i < SrcNames.size(); i++)
	{
		D3DX10_IMAGE_LOAD_INFO LoadInfo;

		LoadInfo.Width     = D3DX10_FROM_FILE;
		LoadInfo.Height    = D3DX10_FROM_FILE;
		LoadInfo.Depth     = D3DX10_FROM_FILE;
		LoadInfo.MipLevels = D3DX10_FROM_FILE; 
		LoadInfo.Format    = DXGI_FORMAT_R8G8B8A8_UNORM;
		LoadInfo.CpuAccessFlags = D3D10_CPU_ACCESS_READ|
								  D3D10_CPU_ACCESS_WRITE;
		LoadInfo.Filter    = D3DX10_FILTER_NONE;
		LoadInfo.MipFilter = D3DX10_FILTER_NONE;
		LoadInfo.Usage     = D3D10_USAGE_STAGING;
		LoadInfo.FirstMipLevel = 0;
		LoadInfo.BindFlags	   = 0;
		LoadInfo.MiscFlags     = 0;
		LoadInfo.pSrcInfo      = 0;

		HR(D3DX10CreateTextureFromFile(gd3dDev, SrcNames[i].c_str(), 
			&LoadInfo, 0, (ID3D10Resource**)&SrcTex[i], 0));
	}

	D3D10_TEXTURE2D_DESC TexDesc;
	SrcTex[0]->GetDesc(&TexDesc);

	D3D10_TEXTURE2D_DESC ad;
	ad.Width			 = TexDesc.Width;
	ad.Height			 = TexDesc.Height;
	ad.MipLevels		 = TexDesc.MipLevels;
	ad.Format			 = TexDesc.Format;
	ad.ArraySize		 = SrcNames.size();
	ad.BindFlags		 = D3D10_BIND_SHADER_RESOURCE;
	ad.Usage			 = D3D10_USAGE_DEFAULT;
	ad.CPUAccessFlags	 = 0;
	ad.MiscFlags		 = 0;
	ad.SampleDesc.Count	  = 1;
	ad.SampleDesc.Quality = 0;

	ID3D10Texture2D* TexArray = 0;
	HR(gd3dDev->CreateTexture2D(&ad, 0, &TexArray));

	for(UINT i = 0; i < SrcNames.size(); i++)
	{
		for(UINT j = 0; j < ad.MipLevels; j++)
		{
			D3D10_MAPPED_TEXTURE2D MappedTex2D;
			SrcTex[i]->Map(j, D3D10_MAP_READ, 0, &MappedTex2D);

			gd3dDev->UpdateSubresource(TexArray, 
				D3D10CalcSubresource(j,i, ad.MipLevels),
				0, MappedTex2D.pData, MappedTex2D.RowPitch, 0);

			SrcTex[i]->Unmap(j);
		}
	}

	D3D10_SHADER_RESOURCE_VIEW_DESC vd;
	vd.Format = ad.Format;
	vd.Texture2DArray.MostDetailedMip = 0;
	vd.Texture2DArray.MipLevels = ad.MipLevels;
	vd.Texture2DArray.FirstArraySlice = 0;
	vd.Texture2DArray.ArraySize = ad.ArraySize;
	vd.ViewDimension = D3D10_SRV_DIMENSION_TEXTURE2DARRAY;

	ID3D10ShaderResourceView* TexArrayRV = 0;
	HR(gd3dDev->CreateShaderResourceView(TexArray, &vd, &TexArrayRV));

	ReleaseCOM(TexArray);

	for(UINT i = 0; i < ad.ArraySize; i++)
		ReleaseCOM(SrcTex[i]);

	Texture t(TexArrayRV, ArrayName, TEXTURE_TYPE_2D_ARRAY);
	Textures.push_back(t);

	return TexArrayRV;
}

ID3D10ShaderResourceView* TexManager::CreateCubeTexure(std::wstring SrcName)
{
	for(size_t i = 0; i < Textures.size(); i++)
	{
		if(SrcName == Textures[i].SrcName)
			return Textures[i].RV;
	}

	D3DX10_IMAGE_LOAD_INFO ld;
	ld.MiscFlags = D3D10_RESOURCE_MISC_TEXTURECUBE;

	ID3D10Texture2D* Tex = 0;
	HR(D3DX10CreateTextureFromFile(gd3dDev, SrcName.c_str(), 
		&ld, 0, (ID3D10Resource**)&Tex,0));

	D3D10_TEXTURE2D_DESC td;
	Tex->GetDesc(&td);

	D3D10_SHADER_RESOURCE_VIEW_DESC vd;
	vd.Format = td.Format;
	vd.ViewDimension = D3D10_SRV_DIMENSION_TEXTURECUBE;
	vd.TextureCube.MipLevels = td.MipLevels;
	vd.TextureCube.MostDetailedMip = 0;

	ID3D10ShaderResourceView* RV;
	HR(gd3dDev->CreateShaderResourceView(Tex, &vd, &RV));

	Texture t(RV,SrcName,TEXTURE_TYPE_CUBE);
	Textures.push_back(t);

	ReleaseCOM(Tex);

	return RV;
}


void TexManager::CreateRandomTexture()
{
	ID3D10Texture1D* Tex = 0;
	D3D10_TEXTURE1D_DESC td;
	ID3D10ShaderResourceView* RV = 0;
	D3D10_SHADER_RESOURCE_VIEW_DESC rd;
	D3D10_SUBRESOURCE_DATA	TexData;

	D3DXVECTOR4 RandomNumbers[1024];
	for(int i = 0; i < 1024; i++)
	{
		RandomNumbers[i].x = RandF(-1.0f,1.0f);
		RandomNumbers[i].y = RandF(-1.0f,1.0f);
		RandomNumbers[i].z = RandF(-1.0f,1.0f);
		RandomNumbers[i].w = RandF(-1.0f,1.0f);
	}

	TexData.pSysMem = RandomNumbers;
	TexData.SysMemPitch = 1024 * sizeof(D3DXVECTOR4);
	TexData.SysMemSlicePitch = 1024 * sizeof(D3DXVECTOR4);

	td.Width			 = 1024;
	td.ArraySize		 = 1;
	td.BindFlags		 = D3D10_BIND_SHADER_RESOURCE;
	td.CPUAccessFlags	 = 0;
	td.Format			 = DXGI_FORMAT_R32G32B32A32_FLOAT;
	td.MipLevels		 = 1;
	td.MiscFlags		 = 0;
	td.Usage			 = D3D10_USAGE_IMMUTABLE;
	
	HR(gd3dDev->CreateTexture1D(&td, &TexData, &Tex));

	rd.Format					 = DXGI_FORMAT_R32G32B32A32_FLOAT;
	rd.Texture1D.MipLevels		 = 1;
	rd.Texture1D.MostDetailedMip = 0;
	rd.ViewDimension			 = D3D10_SRV_DIMENSION_TEXTURE1D;

	HR(gd3dDev->CreateShaderResourceView(Tex, &rd, &RV));

	mRandomTexture.RV	  = RV;
	mRandomTexture.SrcName = L"RandomTex0";
	mRandomTexture.Type	  = TEXTURE_TYPE_1D;

	ReleaseCOM(Tex);
}

ID3D10ShaderResourceView* TexManager::GetRandomTexture()
{	return mRandomTexture.RV;  }


void TexManager::CreateNoiseTexture()
{
	ID3D10Texture2D* Tex = 0;
	
	D3D10_TEXTURE2D_DESC TexDesc;
	TexDesc.ArraySize		= 1;
	TexDesc.BindFlags		= D3D10_BIND_SHADER_RESOURCE;
	TexDesc.CPUAccessFlags	= D3D10_CPU_ACCESS_WRITE;
	/*yes, i know. it should be DXGI_FORMAT_R8G8B8A8_UINT but the 
	sample in MSDN:
	http://msdn.microsoft.com/en-us/library/windows/desktop/bb205131(v=vs.85).aspx#Filling_Textures_Manually
	does it in UNORM */
	TexDesc.Format			= DXGI_FORMAT_R8G8B8A8_UNORM;
	TexDesc.Height			= 512;
	TexDesc.Width			= 512;
	TexDesc.MipLevels		= 1;
	TexDesc.MiscFlags		= 0;
	TexDesc.SampleDesc.Count	= 1;
	TexDesc.SampleDesc.Quality	= 0;
	TexDesc.Usage				= D3D10_USAGE_DYNAMIC;

	HR(gd3dDev->CreateTexture2D(&TexDesc, 0, &Tex) );

	D3D10_MAPPED_TEXTURE2D MappedTex;
	HR(Tex->Map( D3D10CalcSubresource(0, 0, 1), D3D10_MAP_WRITE_DISCARD, 0, 
		&MappedTex) );
	
	uint8_t* Texels = (uint8_t*)MappedTex.pData;
	for(UINT i = 0; i < TexDesc.Height; i++)
	{
		UINT RowStart = i * MappedTex.RowPitch;
		for(UINT j = 0; j < TexDesc.Width; j++)
		{
			UINT Random = (UINT)RandF(0.0f,256.0f);
			Texels[RowStart + 4*j + 0] = 0;
			Texels[RowStart + 4*j + 1] = Random;
			Texels[RowStart + 4*j + 2] = 0;
			Texels[RowStart + 4*j + 3] = 255;
		}
	}

	Tex->Unmap( D3D10CalcSubresource(0, 0, 1) );

	D3D10_SHADER_RESOURCE_VIEW_DESC SRVDesc;
	SRVDesc.Texture2D.MipLevels			= 1;
	SRVDesc.Texture2D.MostDetailedMip	= 0;
	SRVDesc.Format						= TexDesc.Format;
	SRVDesc.ViewDimension				= D3D10_SRV_DIMENSION_TEXTURE2D;

	HR(gd3dDev->CreateShaderResourceView(Tex, &SRVDesc, &mNoiseTexture.RV) );
	
	mNoiseTexture.SrcName = L"NoiseTexture0";
	mNoiseTexture.Type	  = TEXTURE_TYPE_2D;
}

ID3D10ShaderResourceView* TexManager::GetNoiseTexture()
{
	return mNoiseTexture.RV;
}