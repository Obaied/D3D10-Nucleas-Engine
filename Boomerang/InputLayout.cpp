#include "InputLayout.h"
#include "FX.h"

ID3D10InputLayout* InputLayout::ILPos					 = 0;
ID3D10InputLayout* InputLayout::ILRotatingParticle		 = 0;
ID3D10InputLayout* InputLayout::ILPosTex				 = 0;
ID3D10InputLayout* InputLayout::ILPosSizeVH				 = 0;

void InputLayout::InitAll()
{
	//ILPos
	//----------------------------------------------------------------------
	D3D10_INPUT_ELEMENT_DESC Pos[] = 
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, 
		D3D10_INPUT_PER_VERTEX_DATA, 0}
	};

	D3D10_PASS_DESC PassDesc;
	HR(FX::SkyFX->GetTechniqueByName("SkyTech")->GetPassByIndex(0)->GetDesc(&PassDesc));

	HR(gd3dDev->CreateInputLayout(Pos, 1, PassDesc.pIAInputSignature, 
		PassDesc.IAInputSignatureSize, &ILPos));

	//ILRotatingParticle
	//----------------------------------------------------------------------
	D3D10_INPUT_ELEMENT_DESC RotatingParticle[] = 
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, 
		D3D10_INPUT_PER_VERTEX_DATA, 0},
		{"VELOCITY", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12,
		D3D10_INPUT_PER_VERTEX_DATA, 0},
		{"SIZE", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, 
		D3D10_INPUT_PER_VERTEX_DATA, 0},
		{"AGE", 0, DXGI_FORMAT_R32_FLOAT, 0, 32, 
		D3D10_INPUT_PER_VERTEX_DATA, 0},
		{"THETA", 0, DXGI_FORMAT_R32_FLOAT, 0, 36,
		D3D10_INPUT_PER_VERTEX_DATA, 0},
		{"RADIUS", 0, DXGI_FORMAT_R32_FLOAT, 0, 40,
		D3D10_INPUT_PER_VERTEX_DATA, 0},
		{"TYPE", 0, DXGI_FORMAT_R16_SINT, 0, 44,
		D3D10_INPUT_PER_VERTEX_DATA, 0}
	};

	//RFE: Why this technique and not the other one???
	// both are used for vertex input
	//Answer: Both work, just tried it
	HR(FX::LaserFX->GetTechniqueByName("UpdateTech")->GetPassByIndex(0)->GetDesc(&PassDesc));

	HR(gd3dDev->CreateInputLayout(RotatingParticle, 7, PassDesc.pIAInputSignature, 
		PassDesc.IAInputSignatureSize, &ILRotatingParticle));

	//ILPosTex
	//----------------------------------------------------------------------
	D3D10_INPUT_ELEMENT_DESC PosTex[] = 
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, 
		D3D10_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, 
		D3D10_INPUT_PER_VERTEX_DATA, 0}
	};

	//RFE: Why this technique and not the other one???
	// both are used for vertex input
	//Answer: Both work, just tried it
	HR(FX::PP_FX->GetTechniqueByName("QuadTech")->GetPassByIndex(0)->GetDesc(&PassDesc));

	HR(gd3dDev->CreateInputLayout(PosTex, 2, PassDesc.pIAInputSignature, 
		PassDesc.IAInputSignatureSize, &ILPosTex));

	//ILPosSizeVH
	//----------------------------------------------------------------------
	D3D10_INPUT_ELEMENT_DESC PosSizeVH[] = 
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
		D3D10_INPUT_PER_VERTEX_DATA, 0},
		{"SIZE", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12,
		D3D10_INPUT_PER_VERTEX_DATA, 0},
		{"ORIENTATION", 0, DXGI_FORMAT_R16_UINT, 0, 20,
		D3D10_INPUT_PER_VERTEX_DATA, 0}
	};
	HR(FX::GridPtFX->GetTechniqueByName("GridTech")->GetPassByIndex(0)->GetDesc(&PassDesc));

	HR(gd3dDev->CreateInputLayout(PosSizeVH, 3, PassDesc.pIAInputSignature, 
		PassDesc.IAInputSignatureSize, &ILPosSizeVH) );
}

void InputLayout::DestroyAll()
{
	ReleaseCOM(ILPosSizeVH);
	ReleaseCOM(ILPosTex);
	ReleaseCOM(ILRotatingParticle);
	ReleaseCOM(ILPos);	
}