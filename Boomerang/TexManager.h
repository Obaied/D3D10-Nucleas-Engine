#ifndef TEXMANAGER_H
#define TEXMANAGER_H

#include "D3DUtil.h"
#include <vector>

enum TYPE_TEXTURE
{
	TEXTURE_TYPE_1D = 0,
	TEXTURE_TYPE_1D_ARRAY,
	TEXTURE_TYPE_2D,
	TEXTURE_TYPE_2D_ARRAY,
	TEXTURE_TYPE_3D,
	TEXTURE_TYPE_3D_ARRAY,
	TEXTURE_TYPE_CUBE
};

struct Texture
{
	Texture(ID3D10ShaderResourceView* rv, std::wstring srcname, int type)
	{
		RV = rv;
		SrcName = srcname;
		Type = type;
	}

	Texture()
	{
		RV   = 0;
		Type = 2;
	}

	ID3D10ShaderResourceView* RV;
	std::wstring SrcName;
	int Type;
};

class TexManager
{
public:
	friend TexManager& GetTexManager();
	typedef std::vector<std::wstring> StringVector;

	void Init();

	ID3D10ShaderResourceView* CreateTexture2D(std::wstring SrcName);
	ID3D10ShaderResourceView* CreateTextureArray(
		std::wstring ArrayName, StringVector& SrcNames);
	ID3D10ShaderResourceView* CreateCubeTexure(std::wstring SrcName);

	ID3D10ShaderResourceView* GetRandomTexture();
	ID3D10ShaderResourceView* GetNoiseTexture();

private:
	TexManager();
	~TexManager();
	
	void CreateRandomTexture();
	void CreateNoiseTexture();

	void DumpInfo();

private:
	std::vector<Texture> Textures;
	Texture mRandomTexture;
	Texture mNoiseTexture;

};

#endif