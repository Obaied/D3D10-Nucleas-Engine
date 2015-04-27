#include "Sky.h"
#include "InputLayout.h"

Sky::Sky()
	:mRadius(10000.0f),
	mSphere(0)
{
}

Sky::~Sky()
{
	ReleaseCOM(mSphere);
}

void Sky::Init(std::wstring SkySrcName, float Radius)
{
	mSrcName = SkySrcName;
	mRadius = Radius;

	//Mesh Creation
	std::vector<D3DXVECTOR3> Vertices;
	std::vector<UINT>		 Indices;

	BuildGeoSphere(5, mRadius, Vertices, Indices);

	UINT NumVertices = Vertices.size();
	UINT NumIndices  = Indices.size();
	UINT NumFaces	 = NumIndices / 3;

	SkyVertex* MeshVertices = new SkyVertex[NumVertices];
	DWORD*	   MeshIndices  = new DWORD[NumIndices];
	UINT*	   MeshAtr		= new UINT[NumFaces];

	for(size_t i = 0; i < NumVertices; i++)
		MeshVertices[i].Pos = Vertices[i];

	for(size_t i = 0; i < NumIndices; i++)
		MeshIndices[i] = (DWORD)Indices[i];

	for(size_t i = 0; i < NumFaces; i++)
		MeshAtr[i] = 0;

	D3D10_INPUT_ELEMENT_DESC VertexDesc[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, 
		D3D10_INPUT_PER_VERTEX_DATA, 0}
	};

	HR(D3DX10CreateMesh(gd3dDev, VertexDesc, 1, "POSITION", NumVertices, 
		NumFaces, D3DX10_MESH_32_BIT, &mSphere));

	HR(mSphere->SetVertexData(0, MeshVertices));
	HR(mSphere->SetIndexData(MeshIndices, NumIndices));
	HR(mSphere->SetAttributeData(MeshAtr));

	HR(mSphere->GenerateAdjacencyAndPointReps(0.001f));
	HR(mSphere->Optimize(D3DX10_MESHOPT_ATTR_SORT | D3DX10_MESHOPT_VERTEX_CACHE, 
		0, 0));
	HR(mSphere->CommitToDevice());

	delete[] MeshVertices;
	delete[] MeshIndices;
	delete[] MeshAtr;

	//FX Handles
	mfxSkyTech = FX::SkyFX->GetTechniqueByName("SkyTech");
	mfxSkyWVP  = FX::SkyFX->GetVariableByName("gSkyWVP")->AsMatrix();
	mfxCubeTex = FX::SkyFX->GetVariableByName("gCubeTex")->AsShaderResource();

	//Cube Texture
	mCubeTexSRV = GetTexManager().CreateCubeTexure(mSrcName);
}

void Sky::Render(D3DXMATRIX VP)
{
	gd3dDev->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	gd3dDev->IASetInputLayout(InputLayout::ILPos);

	D3DXMATRIX World;
	//D3DXMatrixTranslation(&World, GetCamera().Pos().x, GetCamera().Pos().y, GetCamera().Pos().z);
	D3DXMatrixIdentity(&World);

	D3DXMATRIX SkyWVP;
	//SkyWVP = World * GetCamera().ViewMatrix()*GetCamera().ProjMatrix();
	SkyWVP = World * VP;

	HR(mfxSkyWVP->SetMatrix((float*)&SkyWVP));
	HR(mfxCubeTex->SetResource(mCubeTexSRV));
	
	D3D10_TECHNIQUE_DESC TechDesc;
	HR(mfxSkyTech->GetDesc(&TechDesc));

	for(UINT i = 0; i < TechDesc.Passes; i++)
	{
		ID3D10EffectPass* Pass = mfxSkyTech->GetPassByIndex(i);

		HR(Pass->Apply(0));
		HR(mSphere->DrawSubset(0));
	}
}

ID3D10ShaderResourceView* Sky::GetCubeTex()
{	return mCubeTexSRV;		}