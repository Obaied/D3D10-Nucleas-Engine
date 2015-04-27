#include "MeshManager.h"

struct SphereVertex
{
	D3DXVECTOR3 Pos;
};

MeshManager& GetMeshManager()
{
	static MeshManager Mm;
	return Mm;
}

MeshManager::MeshManager()
{
}

MeshManager::~MeshManager()
{
	ReleaseCOM(mSphereData.Data);
}

const Mesh& MeshManager::CreateSphere()
{
	if(mSphereData.Data != 0)
	{
		//there is an already created sphere mesh, just return it
		return mSphereData;
	}

	std::vector<D3DXVECTOR3> Vertices;
	std::vector<UINT>		 Indices;
	BuildGeoSphere(5, 1.0f, Vertices, Indices);

	UINT NumVertices = Vertices.size();
	UINT NumFaces    = Indices.size()/3;
	UINT NumIndices  = Indices.size();

	D3D10_INPUT_ELEMENT_DESC IEPos[] = 
	{	
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
		D3D10_INPUT_PER_VERTEX_DATA, 0}
	};

	HR(D3DX10CreateMesh(gd3dDev, IEPos, 1, "POSITION", NumVertices, NumFaces,
		D3DX10_MESH_32_BIT, &mSphereData.Data) );

	SphereVertex* SphereVertices = new SphereVertex[NumVertices];
	DWORD* SphereIndices		 = new DWORD[NumIndices];
	UINT*  SphereAdj			 = new UINT[NumFaces];

	for(UINT i = 0; i < NumVertices; i++)
		SphereVertices[i].Pos = Vertices[i];

	for(UINT i = 0; i < NumIndices; i++)
		SphereIndices[i] = (DWORD)Indices[i];

	for(UINT i = 0; i < NumFaces; i++)
		SphereAdj[i] = 0;

	HR(mSphereData.Data->SetVertexData(0, SphereVertices) );
	HR(mSphereData.Data->SetIndexData(SphereIndices, NumIndices) );
	HR(mSphereData.Data->SetAttributeData(SphereAdj) );

	HR(mSphereData.Data->GenerateAdjacencyAndPointReps(0.001f) );
	HR(mSphereData.Data->Optimize(D3DX10_MESHOPT_ATTR_SORT | D3DX10_MESHOPT_VERTEX_CACHE,
		0, 0) );

	HR(mSphereData.Data->CommitToDevice() );
	delete[] SphereVertices;
	delete[] SphereIndices;
	delete[] SphereAdj;

	D3DXMatrixIdentity(&mSphereData.ScalingMatrix);
	D3DXMatrixIdentity(&mSphereData.RotateMatrix);

	return mSphereData;
}