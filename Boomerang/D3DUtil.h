#ifndef _D3DUTIL_
#define _D3DUTIL_

#ifndef WIN32_WINNT
#define WIN32_WINNT 0x0600
#endif

#if defined(DEBUG) || defined(_DEBUG)
	#ifndef D3D_DEBUG_INFO
	#define D3D_DEBUG_INFO
	#endif
#endif

#if defined(DEBUG) || defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#pragma warning(disable:4005)

#include <d3d10.h>
#include <d3d9.h>
#include <dxgi.h>
#include <d3dx10.h>
#include <DxErr.h>
#include <string>
#include <vector>
#include <cassert>
#include <sstream>

//Error Checking & Memory Management
//==================================================================
#if defined(DEBUG) | defined(_DEBUG)
	#ifndef HR
	#define HR(x)                                              \
	{                                                          \
		HRESULT hr = (x);                                      \
		if(FAILED(hr))                                         \
		{                                                      \
			DXTrace(__FILE__, (DWORD)__LINE__, hr, L#x, true); \
		}                                                      \
	}
	#endif

#else
	#ifndef HR
	#define HR(x) (x)
	#endif
#endif 

#define ReleaseCOM(x) {if(x){ x->Release(); x = NULL;} }

#define Error(x) {MessageBox(0, L"Error Handler", L#x, MB_ICONERROR|MB_OK);}

//Program Helper Structures
//==============================================================

template <typename t>
class Array1D
{
public:
	explicit Array1D(int Size);
	~Array1D();

	t& operator[](int rhs);
	t* GetDataPointer() const;
	int Size() const;

private:
	t* Data;
	int mSize;
};

template <typename t>
Array1D<t>::Array1D(int Size)
{
	Data = new t[Size];
	mSize = Size;
}

template <typename t>
Array1D<t>::~Array1D()
{
	delete[] Data;
}

template <typename t>
t& Array1D<t>::operator[](int rhs)
{
	return Data[rhs];
}

template <typename t>
t* Array1D<t>::GetDataPointer() const
{
	return Data;
}

template <typename t>
int Array1D<t>::Size() const 
{	return mSize;	}

//D3D Helper Structures
//==================================================================
struct Light
{
	Light()
	{
		ZeroMemory(this, sizeof(Light));
	}
	D3DXVECTOR3 Pos;
	float		pad0;
	D3DXVECTOR3 Dir;
	float		pad1;
	D3DXCOLOR	Diffuse;
	D3DXCOLOR	Ambient;
	D3DXCOLOR	Specular;
	D3DXVECTOR3 Atten;
	float		pad2;
	float		SpotPower;
	float		Range;
};

struct Material
{
	Material()
	{
		ZeroMemory(this, sizeof(Material));
	}
	D3DXCOLOR Diffuse;
	D3DXCOLOR Ambient;
	D3DXCOLOR Specular;
	float	  SpecPower;
};

struct Ray
{
	D3DXVECTOR3 Pos;
	D3DXVECTOR3 Dir;
};

//Bounding Spheres
struct AABB
{
	AABB()
	{
	}
	AABB(D3DXVECTOR3 min, D3DXVECTOR3 max):
	Min(min), Max(max)
	{
	}

	void ApplyMatrix(D3DXMATRIX& Mat)
	{
		D3DXVec3TransformCoord(&Min, &Min, &Mat);
		D3DXVec3TransformCoord(&Max, &Max, &Mat);
	}

	D3DXVECTOR3 Center()
	{
		return 0.5f*(Min+Max);
	}
	D3DXVECTOR3 Extent()
	{
		return 0.5f*(Max-Min);
	}

	D3DXVECTOR3 Min;
	D3DXVECTOR3 Max;
};

struct BS
{
	BS()
	{
	}
	BS(D3DXVECTOR3 center, float radius)
		:Center(center), Radius(radius)
	{
	}

	void ApplyMatrix(D3DXMATRIX& Mat)
	{
		D3DXVec3TransformCoord(&Center, &Center, &Mat);
	}

	D3DXVECTOR3 Center;
	float		Radius;
};


//Global Pointers
//==================================================================
extern ID3D10Device* gd3dDev;

//Constants
//==================================================================
const float INFINITY = FLT_MAX;
const float PI       = 3.14159265358979323f;
const float MATH_EPS = 0.0001f;
const D3DXVECTOR4 gZeroVec(0.0f,0.0f,0.0f,0.0f);

const D3DXCOLOR D3DCOLOR_WHITE(1.0f, 1.0f, 1.0f, 1.0f);
const D3DXCOLOR D3DCOLOR_BLACK(0.0f, 0.0f, 0.0f, 1.0f);
const D3DXCOLOR D3DCOLOR_RED(1.0f, 0.0f, 0.0f, 1.0f);
const D3DXCOLOR D3DCOLOR_GREEN(0.0f, 1.0f, 0.0f, 1.0f);
const D3DXCOLOR D3DCOLOR_BLUE(0.0f, 0.0f, 1.0f, 1.0f);
const D3DXCOLOR D3DCOLOR_YELLOW(1.0f, 1.0f, 0.0f, 1.0f);
const D3DXCOLOR D3DCOLOR_CYAN(0.0f, 1.0f, 1.0f, 1.0f);
const D3DXCOLOR D3DCOLOR_MAGNETA(1.0f, 0.0f, 1.0f, 1.0f);

const D3DXCOLOR D3DCOLOR_BEACH_SAND(1.0f, 0.96f, 0.62f, 1.0f);
const D3DXCOLOR D3DCOLOR_LIGHT_YELLOW_GREEN(0.48f, 0.77f, 0.46f, 1.0f);
const D3DXCOLOR D3DCOLOR_DARK_YELLOW_GREEN(0.1f, 0.48f, 0.19f, 1.0f);
const D3DXCOLOR D3DCOLOR_DARKBROWN(0.45f, 0.39f, 0.34f, 1.0f);

const D3DXVECTOR2 DIR_UP(0.0f,1.0f);
const D3DXVECTOR2 DIR_DOWN(0.0f,-1.0f);
const D3DXVECTOR2 DIR_RIGHT(1.0f,0.0f);
const D3DXVECTOR2 DIR_LEFT(-1.0f,0.0f);

//Vertex Structures
//==================================================================
struct Pvertex
{
	D3DXVECTOR3 Pos;
};

struct PNTvertex
{
	D3DXVECTOR3 Pos;
	D3DXVECTOR3 Normal;
	D3DXVECTOR2 TexC;
};

struct PTvertex
{
	D3DXVECTOR3 Pos;
	D3DXVECTOR2 TexC;
};

struct PTNTvertex
{
	D3DXVECTOR3 Pos;
	D3DXVECTOR3 Tangent;
	D3DXVECTOR3 Normal;
	D3DXVECTOR2 TexC;
};

//Helper Functions
//==================================================================
D3DX10INLINE float RandF(float a, float b)
{
	float r = (float)(rand()) / (float)(RAND_MAX);
	r = a + r*(b-a);
	return r;
}

template<typename T>
D3DX10INLINE T Min(const T& a, const T& b)
{
	if(a == b)
		return a;

	return a < b ? a : b;
}

template<typename T>
D3DX10INLINE T Max(const T& a, const T& b)
{
	if(a == b)
		return a;

	return a > b ? a : b;
}

template<typename T>
D3DX10INLINE T SmoothStep(const T& x, const T& y, const T& t)
{
	if(t < x)		return x;
	else if(t > y)	return y;
	else			return t;
}
//Geometric Model Creation Functions
//==================================================================

void BuildGeoSphere(UINT NumSubdivisions, float Radius,
	std::vector<D3DXVECTOR3>& Vertices, std::vector<UINT>& Indices);

void BuildGeoCube(PNTvertex** Vertices);

void BuildGeoQuad(PNTvertex** Vertices);

//Helper functions
//==================================================================
float GaussianDistribution(float x, float y, float Sigma);

void CalcAABB(AABB& Box, std::vector<D3DXVECTOR3>& Vertices);

#endif