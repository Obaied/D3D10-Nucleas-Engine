#include "FX.h"

//Focus:
// Observe that you will get a Unresolved External Symbol error if you left the members of the namespace unintialized.
// that is because you are using it in the InputLayout member functions, which is why you have to initilize them to a null at first.
ID3D10Effect* FX::SkyFX  = 0;
ID3D10Effect* FX::LaserFX = 0;
ID3D10Effect* FX::GridPtFX = 0;
ID3D10Effect* FX::PP_FX  = 0;
ID3D10Effect* FX::BallFX = 0;

ID3D10Effect* FX::CreateFX(std::wstring SrcName)
{
	DWORD ShaderFlags = D3D10_SHADER_ENABLE_STRICTNESS;
#if defined(DEBUG) | defined(_DEBUG)
	ShaderFlags |= D3D10_SHADER_DEBUG;
	ShaderFlags |= D3D10_SHADER_SKIP_OPTIMIZATION;

#endif
	ID3D10Blob* CompilationErrors = 0;
	ID3D10Effect* fx = 0;

	HRESULT hr = D3DX10CreateEffectFromFile(SrcName.c_str(), 0,0,"fx_4_0",
		ShaderFlags, 0, gd3dDev, 0,0, &fx, &CompilationErrors, 0);

	if(FAILED(hr))
	{
		if( CompilationErrors )
		{
			MessageBoxA(0, (char*)CompilationErrors->GetBufferPointer(), 0, MB_OK);
			ReleaseCOM(CompilationErrors);
		}
		DXTrace(__FILE__, (DWORD)__LINE__, hr, SrcName.c_str(), true);
		return 0;
	}

	return fx;
}

void FX::InitAll()
{
	SkyFX			 = CreateFX(L"Sky.fx");
	LaserFX			 = CreateFX(L"Laser.fx");
	GridPtFX		 = CreateFX(L"GridPt.fx");
	PP_FX			 = CreateFX(L"PP.fx");
	BallFX			 = CreateFX(L"Ball.fx");
}

void FX::DestroyAll()
{
	ReleaseCOM(PP_FX);
	ReleaseCOM(BallFX);
	ReleaseCOM(GridPtFX);
	ReleaseCOM(LaserFX);
	ReleaseCOM(SkyFX);
}