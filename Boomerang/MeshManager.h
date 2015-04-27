#ifndef MeshManager_h
#define MeshManager_h

#include "D3DUtil.h"

struct Mesh
{
	Mesh()
	{
		Data = 0;
	}
	~Mesh()
	{
	}

	ID3DX10Mesh* Data;
	D3DXMATRIX ScalingMatrix;
	D3DXMATRIX RotateMatrix;
};

class MeshManager
{
public:

	friend MeshManager& GetMeshManager();
	MeshManager();
	~MeshManager();

	const Mesh& CreateSphere();

private:

	std::vector<Mesh> mMeshes;
	Mesh mSphereData;
};

#endif