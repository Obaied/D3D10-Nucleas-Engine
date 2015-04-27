#ifndef _FX_H
#define _FX_H

#include "D3DUtil.h"

namespace FX
{
	extern ID3D10Effect* SkyFX;
	extern ID3D10Effect* LaserFX;
	extern ID3D10Effect* GridPtFX;
	extern ID3D10Effect* PP_FX;
	extern ID3D10Effect* BallFX;

	ID3D10Effect* CreateFX(std::wstring SrcName);
	void InitAll();
	void DestroyAll();
}

#endif