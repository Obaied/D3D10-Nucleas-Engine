#ifndef _INPUTLAYOUT_H
#define _INPUTLAYOUT_H

#include "D3DUtil.h"

namespace InputLayout
{
	extern ID3D10InputLayout* ILPos;
	extern ID3D10InputLayout* ILRotatingParticle;
	extern ID3D10InputLayout* ILPosTex;
	extern ID3D10InputLayout* ILPosSizeVH;

	void InitAll();
	void DestroyAll();

};

#endif