#include "Lighting.fx"

cbuffer cbPerFrame
{	
	float4x4 gMatWVP;
	float4x4 gMatW;
	float3	 gEyePos;
	float4   gDiffMtrl;

	float	 gExplosionAmt;
}

cbuffer cbPerWorld
{
	Light gLight;
}

cbuffer cbConst
{
	float4 gZeroVec = float4(0.0f,0.0f,0.0f,0.0f);
}

Texture2D	gTex;

SamplerState sTriLinearWrap
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Wrap;
	AddressV = Wrap;
};

struct VS_IN
{
	float3 PosL : POSITION;
};

struct VS_OUT
{
	float3 PosL : POSITION;
};

VS_OUT AI_VS(VS_IN vIn)
{
	VS_OUT v;
	v.PosL = vIn.PosL;
	return v;
}

struct GS_OUT
{
	float4 PosH   : SV_POSITION;
	float3 PosW	  : POSITION;
	float3 Normal : NORMAL;
	float2 TexC	  : TEXCOORD;
};

[maxvertexcount(3)]
void AI_GS(triangle VS_OUT gIn[3],
		uint PrimID : SV_PRIMITIVEID,			
		inout TriangleStream<GS_OUT> tStream)
{
//   	   <B>
//		   / \
//		  /   \
//		 /     \
//	    /       \
//	   /	     \
//	  /	 	   	  \
//   /             \
//  /               \
// <A>--------------<C>

float3 A = gIn[0].PosL;
float3 B = gIn[1].PosL;
float3 C = gIn[2].PosL;

float3 BA = B-A;
float3 CA = C-A;

float3 FaceNormal = normalize( cross(BA, CA) );

GS_OUT gOut;
[unroll]
for(int i = 0; i < 3; i++)
{
	float3 Pos = gIn[i].PosL + (FaceNormal*gExplosionAmt);
	gOut.PosH	= mul(float4(Pos,1.0f), gMatWVP);
	gOut.PosW	= mul(float4(Pos,1.0f), gMatW).xyz;
	gOut.TexC	= float2(0.0f,0.0f);
	gOut.Normal = FaceNormal;

	tStream.Append(gOut);
}

}

#define PS_IN GS_OUT

float4 AI_PS(PS_IN pIn) : SV_TARGET
{
	float4 DiffMtrl = gDiffMtrl;
	clip(DiffMtrl.a-0.15f);

	SurfaceInfo s = {DiffMtrl, gZeroVec, pIn.Normal, pIn.PosW};
	float4 LitColor = CalcDirLight(s, gLight, gEyePos, -1.0f);
	
	return LitColor;
}
	
technique10 AITech
{
	pass p0
	{
		SetVertexShader(	CompileShader(vs_4_0, AI_VS() ) );
		SetGeometryShader(	CompileShader(gs_4_0, AI_GS() ) );
		SetPixelShader(		CompileShader(ps_4_0, AI_PS() ) );
	}
}