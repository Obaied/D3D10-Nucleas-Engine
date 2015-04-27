#include "Lighting.fx"

cbuffer cbPerFrame
{	
	float4x4 gMatWVP;
	float4x4 gMatW;
	float3	 gEyePos;
}

cbuffer cbPerWorld
{
	Light	 gDirLight;
}

struct PNTvertex
{
	float3 Pos;
	float3 Normal;
	float2 TexC;
};

cbuffer cbConst
{
	float4 gZeroVec = float4(0.0f,0.0f,0.0f,0.0f);
	//PNTvertex CubeVertices[36];
}

Texture1D	gRandomTex;
Texture2D	gGridTex;

SamplerState sTriLinearWrap
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Clamp;
	AddressV = Clamp;
};

struct VS_IN
{
	float3 PosL			: POSITION;
	float2 Size			: SIZE;
	int   Orientation  : ORIENTATION;
};

struct VS_OUT
{
	float3 PosL			: POSITION;
	float2 Size			: SIZE;
	int   Orientation  : ORIENTATION;
};

VS_OUT Grid_VS(VS_IN vIn)
{
	VS_OUT v;
	v.PosL = vIn.PosL;
	v.Size = vIn.Size;
	v.Orientation = vIn.Orientation;
	return v;
}

struct GS_OUT
{
	float4 PosH  : SV_POSITION;
	float3 PosW	 : POSITION;
	float3 Normal : NORMAL;
	float2 TexC	 : TEXCOORD;
	int   Orientation : ORIENTATION;
};

[maxvertexcount(36)]
void Grid_GS(point VS_OUT gIn[1],
		uint PrimID : SV_PRIMITIVEID,			
		inout TriangleStream<GS_OUT> tStream)
{
	float HalfSize = 0.5f*gIn[0].Size.x;
	
	PNTvertex v[36] = {
	//Front Face
		{	float3(-1.0f,-1.0f,-1.0f), float3(0.0f,0.0f,-1.0f), float2(0.0f,1.0f) },
		{	float3(-1.0f, 1.0f,-1.0f), float3(0.0f,0.0f,-1.0f), float2(0.0f,0.0f) },
		{	float3( 1.0f, 1.0f,-1.0f), float3(0.0f,0.0f,-1.0f), float2(1.0f,0.0f) },

		{	float3( 1.0f, 1.0f,-1.0f), float3(0.0f,0.0f,-1.0f), float2(0.0f,1.0f) },
		{	float3( 1.0f,-1.0f,-1.0f), float3(0.0f,0.0f,-1.0f), float2(1.0f,0.0f) },
		{	float3(-1.0f,-1.0f,-1.0f), float3(0.0f,0.0f,-1.0f), float2(1.0f,1.0f) },

		//Back Face
		{	float3( 1.0f,-1.0f, 1.0f), float3(0.0f,0.0f, 1.0f), float2(0.0f,1.0f) },
		{	float3( 1.0f, 1.0f, 1.0f), float3(0.0f,0.0f, 1.0f), float2(0.0f,0.0f) },
		{	float3(-1.0f, 1.0f, 1.0f), float3(0.0f,0.0f, 1.0f), float2(1.0f,0.0f) },

		{	float3(-1.0f, 1.0f, 1.0f), float3(0.0f,0.0f, 1.0f), float2(0.0f,1.0f) },
		{	float3(-1.0f,-1.0f, 1.0f), float3(0.0f,0.0f, 1.0f), float2(1.0f,0.0f) },
		{	float3( 1.0f,-1.0f, 1.0f), float3(0.0f,0.0f, 1.0f), float2(1.0f,1.0f) },

		//Top Face
		{	float3(-1.0f, 1.0f,-1.0f), float3(0.0f,1.0f, 0.0f), float2(0.0f,1.0f) },
		{	float3(-1.0f, 1.0f, 1.0f), float3(0.0f,1.0f, 0.0f), float2(0.0f,0.0f) },
		{	float3( 1.0f, 1.0f, 1.0f), float3(0.0f,1.0f, 0.0f), float2(1.0f,0.0f) },

		{	float3( 1.0f, 1.0f, 1.0f), float3(0.0f,1.0f, 0.0f), float2(0.0f,1.0f) },
		{	float3( 1.0f, 1.0f,-1.0f), float3(0.0f,1.0f, 0.0f), float2(1.0f,0.0f) },
		{	float3(-1.0f, 1.0f,-1.0f), float3(0.0f,1.0f, 0.0f), float2(1.0f,1.0f) },

		//Bot Face
		{	float3( 1.0f,-1.0f,-1.0f), float3(0.0f,-1.0f, 0.0f), float2(0.0f,1.0f) },
		{	float3( 1.0f,-1.0f, 1.0f), float3(0.0f,-1.0f, 0.0f), float2(0.0f,0.0f) },
		{	float3(-1.0f,-1.0f, 1.0f), float3(0.0f,-1.0f, 0.0f), float2(1.0f,0.0f) },

		{	float3(-1.0f,-1.0f, 1.0f), float3(0.0f,-1.0f, 0.0f), float2(0.0f,1.0f) },
		{	float3(-1.0f,-1.0f,-1.0f), float3(0.0f,-1.0f, 0.0f), float2(1.0f,0.0f) },
		{	float3( 1.0f,-1.0f,-1.0f), float3(0.0f,-1.0f, 0.0f), float2(1.0f,1.0f) },

		//Left Face
		{	float3(-1.0f,-1.0f, 1.0f), float3(-1.0f,0.0f, 0.0f), float2(0.0f,1.0f) },
		{	float3(-1.0f, 1.0f, 1.0f), float3(-1.0f,0.0f, 0.0f), float2(0.0f,0.0f) },
		{	float3(-1.0f, 1.0f,-1.0f), float3(-1.0f,0.0f, 0.0f), float2(1.0f,0.0f) },

		{	float3(-1.0f, 1.0f,-1.0f), float3(-1.0f,0.0f, 0.0f), float2(0.0f,1.0f) },
		{	float3(-1.0f,-1.0f,-1.0f), float3(-1.0f,0.0f, 0.0f), float2(1.0f,0.0f) },
		{	float3(-1.0f,-1.0f, 1.0f), float3(-1.0f,0.0f, 0.0f), float2(1.0f,1.0f) },

		//Right Face
		{	float3( 1.0f,-1.0f,-1.0f), float3(1.0f,0.0f, 0.0f), float2(0.0f,1.0f) },
		{	float3( 1.0f, 1.0f,-1.0f), float3(1.0f,0.0f, 0.0f), float2(0.0f,0.0f) },
		{	float3( 1.0f, 1.0f, 1.0f), float3(1.0f,0.0f, 0.0f), float2(1.0f,0.0f) },

		{	float3( 1.0f, 1.0f, 1.0f), float3(1.0f,0.0f, 0.0f), float2(0.0f,1.0f) },
		{	float3( 1.0f,-1.0f, 1.0f), float3(1.0f,0.0f, 0.0f), float2(1.0f,0.0f) },
		{	float3( 1.0f,-1.0f,-1.0f), float3(1.0f,0.0f, 0.0f), float2(1.0f,1.0f) },
	};
	
	for(int i = 0; i < 36; i++)
	{
		v[i].Pos.xz *= HalfSize;
	}

	for(int j = 0; j < 36; j++)
	{
		//Vertical Wall & Horizontal Wall
		if(gIn[0].Orientation == 1 || gIn[0].Orientation == 2)
			v[j].Pos.y *= HalfSize;
	}

	float x,y,z;
	x = gIn[0].PosL.x;
	y = gIn[0].PosL.y;
	z = gIn[0].PosL.z;

	float4x4 TransMat0;
	TransMat0[0] = float4(1.0f,0.0f,0.0f,0.0f);
	TransMat0[1] = float4(0.0f,1.0f,0.0f,0.0f);
	TransMat0[2] = float4(0.0f,0.0f,1.0f,0.0f);
	TransMat0[3] = float4(gIn[0].PosL,1.0f);

	float4x4 TransMat1;
	TransMat1[0] = float4(1.0f,0.0f,0.0f,0.0f);
	TransMat1[1] = float4(0.0f,1.0f,0.0f,0.0f);
	TransMat1[2] = float4(0.0f,0.0f,1.0f,0.0f);
	TransMat1[3] = float4(gIn[0].PosL.x,
			gIn[0].PosL.y+HalfSize+1.0f, gIn[0].PosL.z, 1.0f);

	//default case is it being horizontal
	float4x4 T;

	//Tile
	if(gIn[0].Orientation == 0)
		T = TransMat0;
	//Vertical Wall & Horizontal Wall
	else if(gIn[0].Orientation == 1 || gIn[0].Orientation == 2)
		T = TransMat1;

	float4x4 WVP = mul(T,gMatWVP);
	GS_OUT gOut;
	[unroll]
	for(int k = 0; k < 36; k++)
	{
		gOut.PosH = mul(float4(v[k].Pos,1.0f), WVP);
		gOut.PosW = mul(float4(v[k].Pos,1.0f), T).xyz;
		gOut.Normal = v[k].Normal;
		gOut.TexC = v[k].TexC;
		gOut.Orientation = gIn[0].Orientation;

		tStream.Append(gOut);

		if(k==2||k==5||k==8||k==11||k==14
			||k==17||k==20||k==23||k==26||k==29
			||k==32||k==35)
			tStream.RestartStrip();
	}
}

#define PS_IN GS_OUT

float4 Grid_PS(PS_IN pIn) : SV_TARGET
{
	float4 DiffMtrl = gGridTex.Sample(sTriLinearWrap, pIn.TexC);
	clip(DiffMtrl.a-0.15f);

	SurfaceInfo s = {DiffMtrl, gZeroVec, pIn.Normal, pIn.PosW};
	float4 LitColor = CalcDirLight(s, gDirLight, gEyePos, -1.0f);
	
	return LitColor;
}
	
technique10 GridTech
{
	pass p0
	{
		SetVertexShader(	CompileShader(vs_4_0, Grid_VS() ) );
		SetGeometryShader(	CompileShader(gs_4_0, Grid_GS() ) );
		SetPixelShader(		CompileShader(ps_4_0, Grid_PS() ) );
	}
}