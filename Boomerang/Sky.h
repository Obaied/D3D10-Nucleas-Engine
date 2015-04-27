#ifndef _SKY_H
#define _SKY_H

#include "D3DUtil.h"
#include "TexManager.h"
#include "FX.h"
#include "Camera.h"

struct SkyVertex
{
	D3DXVECTOR3 Pos;
};

class Sky
{
public:
	Sky();
	~Sky();

	void Init(std::wstring SkySrcName, float Radius);
	void Render(D3DXMATRIX VP);

	ID3D10ShaderResourceView* GetCubeTex();

private:

	std::wstring						mSrcName;
	float								mRadius;
	ID3DX10Mesh*						mSphere;
	ID3D10ShaderResourceView*			mCubeTexSRV;

	ID3D10EffectTechnique*				mfxSkyTech;
	ID3D10EffectMatrixVariable*			mfxSkyWVP;
	ID3D10EffectShaderResourceVariable* mfxCubeTex;

};

#endif