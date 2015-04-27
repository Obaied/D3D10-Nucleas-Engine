#include "D3DUtil.h"
#include <fstream>

typedef std::vector<D3DXVECTOR3> VertexList;
typedef std::vector<UINT>		 IndexList;

void GeoSphere_Subdivide(VertexList& Vertices, IndexList& Indices)
{
	/*	   <1>
		   / \
		  /   \
		 /     \
	   <3>-----<4>
	   /\	   / \
	  /	 \	  /	  \
     /    \  /     \
    /      \/       \
  <0>------<5>------<2>	   */

	typedef D3DXVECTOR3 point;

	std::vector<D3DXVECTOR3> Pos;
	std::vector<UINT>	     Ind;

	Pos = Vertices;
	Ind = Indices;
	//The number of vertices is being subdivided and doesn't hold true
	// anymore. Just look at the figure above. It has 6 vertices and 4
	// triangles, but the indices are always equal to the number of 
	// triangles * 3
	int NumTris = Indices.size()/3;

	Vertices.resize(0);
	Indices.resize(0);

	for(int i = 0; i < NumTris; i++)
	{
		point v0 = Pos[ Ind[i*3 + 0] ];
		point v1 = Pos[ Ind[i*3 + 1] ];
		point v2 = Pos[ Ind[i*3 + 2] ];
		point v3 = 0.5f * (v1+v0);
		point v4 = 0.5f * (v1+v2);
		point v5 = 0.5f * (v0+v2);

		Vertices.push_back(v0);
		Vertices.push_back(v1);
		Vertices.push_back(v2);
		Vertices.push_back(v3);
		Vertices.push_back(v4);
		Vertices.push_back(v5);


		Indices.push_back(i*6 + 0);
		Indices.push_back(i*6 + 3);
		Indices.push_back(i*6 + 5);

		Indices.push_back(i*6 + 3);
		Indices.push_back(i*6 + 4);
		Indices.push_back(i*6 + 5);

		Indices.push_back(i*6 + 5);
		Indices.push_back(i*6 + 4);
		Indices.push_back(i*6 + 2);

		Indices.push_back(i*6 + 3);
		Indices.push_back(i*6 + 1);
		Indices.push_back(i*6 + 4);

	}

}

void BuildGeoSphere(UINT NumSubdivisions, float Radius,
	VertexList& Vertices, IndexList& Indices)
{
	//Build a Sphere by tesselating an tetradecahedron
	//==============================================

	Vertices.resize(0);
	Indices.resize(0);

	NumSubdivisions = Min(NumSubdivisions, UINT(5) );

	const float X = 0.525731f; 
	const float Z = 0.850651f;

	D3DXVECTOR3 Pos[12] = 
	{
		D3DXVECTOR3(-X, 0.0f, Z),  D3DXVECTOR3(X, 0.0f, Z),  
		D3DXVECTOR3(-X, 0.0f, -Z), D3DXVECTOR3(X, 0.0f, -Z),    
		D3DXVECTOR3(0.0f, Z, X),   D3DXVECTOR3(0.0f, Z, -X), 
		D3DXVECTOR3(0.0f, -Z, X),  D3DXVECTOR3(0.0f, -Z, -X),    
		D3DXVECTOR3(Z, X, 0.0f),   D3DXVECTOR3(-Z, X, 0.0f), 
		D3DXVECTOR3(Z, -X, 0.0f),  D3DXVECTOR3(-Z, -X, 0.0f)
	};

	UINT Ind[60] = 
	{
		1,4,0,  4,9,0,  4,5,9,  8,5,4,  1,8,4,    
		1,10,8, 10,3,8, 8,3,5,  3,2,5,  3,7,2,    
		3,10,7, 10,6,7, 6,11,7, 6,0,11, 6,1,0, 
		10,1,6, 11,0,9, 2,11,9, 5,2,9,  11,2,7 
	};

	Vertices.resize(12);
	Indices.resize(60);

	for(int i = 0; i < 12; i++)
		Vertices[i] = Pos[i];
	for(int i = 0; i < 60; i++)
		Indices[i] = Ind[i];

	for(UINT i = 0; i < NumSubdivisions; i++)
		GeoSphere_Subdivide(Vertices, Indices);


	for(size_t i = 0; i < Vertices.size(); i++)
	{
		D3DXVec3Normalize(&Vertices[i], &Vertices[i]);
		Vertices[i] *= Radius;
	}
}

float GaussianDistribution(float x, float y, float Sigma)
{
	float v = 1.0f / sqrtf(2.0f*PI*Sigma*Sigma);

	float Value = v * exp( -(x*x + y*y) / ( 2.0f*Sigma*Sigma ) );

	return Value;
}

void BuildGeoCube(PNTvertex** Vertices)
{
	PNTvertex v[] = {
		//Front Face
		//RFE: this is wrong, just check the first quad
		{	D3DXVECTOR3(-1.0f,-1.0f,-1.0f), D3DXVECTOR3(0.0f,0.0f,-1.0f), D3DXVECTOR2(0.0f,1.0f) },
		{	D3DXVECTOR3(-1.0f, 1.0f,-1.0f), D3DXVECTOR3(0.0f,0.0f,-1.0f), D3DXVECTOR2(0.0f,0.0f) },
		{	D3DXVECTOR3( 1.0f, 1.0f,-1.0f), D3DXVECTOR3(0.0f,0.0f,-1.0f), D3DXVECTOR2(1.0f,0.0f) },

		{	D3DXVECTOR3( 1.0f, 1.0f,-1.0f), D3DXVECTOR3(0.0f,0.0f,-1.0f), D3DXVECTOR2(0.0f,1.0f) },
		{	D3DXVECTOR3( 1.0f,-1.0f,-1.0f), D3DXVECTOR3(0.0f,0.0f,-1.0f), D3DXVECTOR2(1.0f,0.0f) },
		{	D3DXVECTOR3(-1.0f,-1.0f,-1.0f), D3DXVECTOR3(0.0f,0.0f,-1.0f), D3DXVECTOR2(1.0f,1.0f) },

		//Back Face
		{	D3DXVECTOR3( 1.0f,-1.0f, 1.0f), D3DXVECTOR3(0.0f,0.0f, 1.0f), D3DXVECTOR2(0.0f,1.0f) },
		{	D3DXVECTOR3( 1.0f, 1.0f, 1.0f), D3DXVECTOR3(0.0f,0.0f, 1.0f), D3DXVECTOR2(0.0f,0.0f) },
		{	D3DXVECTOR3(-1.0f, 1.0f, 1.0f), D3DXVECTOR3(0.0f,0.0f, 1.0f), D3DXVECTOR2(1.0f,0.0f) },

		{	D3DXVECTOR3(-1.0f, 1.0f, 1.0f), D3DXVECTOR3(0.0f,0.0f, 1.0f), D3DXVECTOR2(0.0f,1.0f) },
		{	D3DXVECTOR3(-1.0f,-1.0f, 1.0f), D3DXVECTOR3(0.0f,0.0f, 1.0f), D3DXVECTOR2(1.0f,0.0f) },
		{	D3DXVECTOR3( 1.0f,-1.0f, 1.0f), D3DXVECTOR3(0.0f,0.0f, 1.0f), D3DXVECTOR2(1.0f,1.0f) },

		//Top Face
		{	D3DXVECTOR3(-1.0f, 1.0f,-1.0f), D3DXVECTOR3(0.0f,1.0f, 0.0f), D3DXVECTOR2(0.0f,1.0f) },
		{	D3DXVECTOR3(-1.0f, 1.0f, 1.0f), D3DXVECTOR3(0.0f,1.0f, 0.0f), D3DXVECTOR2(0.0f,0.0f) },
		{	D3DXVECTOR3( 1.0f, 1.0f, 1.0f), D3DXVECTOR3(0.0f,1.0f, 0.0f), D3DXVECTOR2(1.0f,0.0f) },

		{	D3DXVECTOR3( 1.0f, 1.0f, 1.0f), D3DXVECTOR3(0.0f,1.0f, 0.0f), D3DXVECTOR2(0.0f,1.0f) },
		{	D3DXVECTOR3( 1.0f, 1.0f,-1.0f), D3DXVECTOR3(0.0f,1.0f, 0.0f), D3DXVECTOR2(1.0f,0.0f) },
		{	D3DXVECTOR3(-1.0f, 1.0f,-1.0f), D3DXVECTOR3(0.0f,1.0f, 0.0f), D3DXVECTOR2(1.0f,1.0f) },

		//Bot Face
		{	D3DXVECTOR3( 1.0f,-1.0f,-1.0f), D3DXVECTOR3(0.0f,-1.0f, 0.0f), D3DXVECTOR2(0.0f,1.0f) },
		{	D3DXVECTOR3( 1.0f,-1.0f, 1.0f), D3DXVECTOR3(0.0f,-1.0f, 0.0f), D3DXVECTOR2(0.0f,0.0f) },
		{	D3DXVECTOR3(-1.0f,-1.0f, 1.0f), D3DXVECTOR3(0.0f,-1.0f, 0.0f), D3DXVECTOR2(1.0f,0.0f) },

		{	D3DXVECTOR3(-1.0f,-1.0f, 1.0f), D3DXVECTOR3(0.0f,-1.0f, 0.0f), D3DXVECTOR2(0.0f,1.0f) },
		{	D3DXVECTOR3(-1.0f,-1.0f,-1.0f), D3DXVECTOR3(0.0f,-1.0f, 0.0f), D3DXVECTOR2(1.0f,0.0f) },
		{	D3DXVECTOR3( 1.0f,-1.0f,-1.0f), D3DXVECTOR3(0.0f,-1.0f, 0.0f), D3DXVECTOR2(1.0f,1.0f) },

		//Left Face
		{	D3DXVECTOR3(-1.0f,-1.0f, 1.0f), D3DXVECTOR3(-1.0f,0.0f, 0.0f), D3DXVECTOR2(0.0f,1.0f) },
		{	D3DXVECTOR3(-1.0f, 1.0f, 1.0f), D3DXVECTOR3(-1.0f,0.0f, 0.0f), D3DXVECTOR2(0.0f,0.0f) },
		{	D3DXVECTOR3(-1.0f, 1.0f,-1.0f), D3DXVECTOR3(-1.0f,0.0f, 0.0f), D3DXVECTOR2(1.0f,0.0f) },

		{	D3DXVECTOR3(-1.0f, 1.0f,-1.0f), D3DXVECTOR3(-1.0f,0.0f, 0.0f), D3DXVECTOR2(0.0f,1.0f) },
		{	D3DXVECTOR3(-1.0f,-1.0f,-1.0f), D3DXVECTOR3(-1.0f,0.0f, 0.0f), D3DXVECTOR2(1.0f,0.0f) },
		{	D3DXVECTOR3(-1.0f,-1.0f, 1.0f), D3DXVECTOR3(-1.0f,0.0f, 0.0f), D3DXVECTOR2(1.0f,1.0f) },

		//Right Face
		{	D3DXVECTOR3( 1.0f,-1.0f,-1.0f), D3DXVECTOR3(1.0f,0.0f, 0.0f), D3DXVECTOR2(0.0f,1.0f) },
		{	D3DXVECTOR3( 1.0f, 1.0f,-1.0f), D3DXVECTOR3(1.0f,0.0f, 0.0f), D3DXVECTOR2(0.0f,0.0f) },
		{	D3DXVECTOR3( 1.0f, 1.0f, 1.0f), D3DXVECTOR3(1.0f,0.0f, 0.0f), D3DXVECTOR2(1.0f,0.0f) },

		{	D3DXVECTOR3( 1.0f, 1.0f, 1.0f), D3DXVECTOR3(1.0f,0.0f, 0.0f), D3DXVECTOR2(0.0f,1.0f) },
		{	D3DXVECTOR3( 1.0f,-1.0f, 1.0f), D3DXVECTOR3(1.0f,0.0f, 0.0f), D3DXVECTOR2(1.0f,0.0f) },
		{	D3DXVECTOR3( 1.0f,-1.0f,-1.0f), D3DXVECTOR3(1.0f,0.0f, 0.0f), D3DXVECTOR2(1.0f,1.0f) },
	};

	*Vertices = v;

}

void BuildGeoQuad(PNTvertex** Vertices)
{
	PNTvertex v[] = {
		//this is wrong, just check the first quad
		{	D3DXVECTOR3(-1.0f,-1.0f, 0.0f), D3DXVECTOR3(0.0f,0.0f,-1.0f), D3DXVECTOR2(0.0f,1.0f) },
		{	D3DXVECTOR3(-1.0f, 1.0f, 0.0f), D3DXVECTOR3(0.0f,0.0f,-1.0f), D3DXVECTOR2(0.0f,0.0f) },
		{	D3DXVECTOR3( 1.0f, 1.0f, 0.0f), D3DXVECTOR3(0.0f,0.0f,-1.0f), D3DXVECTOR2(1.0f,0.0f) },

		{	D3DXVECTOR3( 1.0f, 1.0f, 0.0f), D3DXVECTOR3(0.0f,0.0f,-1.0f), D3DXVECTOR2(0.0f,1.0f) },
		{	D3DXVECTOR3( 1.0f,-1.0f, 0.0f), D3DXVECTOR3(0.0f,0.0f,-1.0f), D3DXVECTOR2(1.0f,0.0f) },
		{	D3DXVECTOR3(-1.0f,-1.0f, 0.0f), D3DXVECTOR3(0.0f,0.0f,-1.0f), D3DXVECTOR2(1.0f,1.0f) },
	};

	*Vertices = v;
}