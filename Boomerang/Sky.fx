cbuffer cbPerFrame
{
	float4x4 gSkyWVP;
};

TextureCube gCubeTex;

SamplerState s_TriLinearWrap
{
	Filter	 = MIN_MAG_MIP_LINEAR;
	AddressU = WRAP;
	AddressV = WRAP;
};

struct VS_IN
{
	float3 PosL : POSITION;
};

struct VS_OUT
{
	float4 PosH		 : SV_POSITION;
	float3 LookUpVec : TEXCOORD;
};

VS_OUT Sky_VShader(VS_IN vIn)
{
	VS_OUT vOut;

	vOut.PosH		= mul(float4(vIn.PosL,1.0f), gSkyWVP).xyww;
	vOut.LookUpVec	= vIn.PosL;

	return vOut;
}

float4 Sky_PShader(VS_OUT vOut) : SV_TARGET
{
	return gCubeTex.Sample(s_TriLinearWrap, vOut.LookUpVec);
}

RasterizerState NoCullRS
{
	CullMode = None;
};

DepthStencilState Less_EqualDSS
{
	DepthFunc = LESS_EQUAL;
};

technique10 SkyTech
{
	pass P0
	{
		SetVertexShader( CompileShader( vs_4_0, Sky_VShader() ));
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_4_0, Sky_PShader() ));

		SetRasterizerState(NoCullRS);
		SetDepthStencilState(Less_EqualDSS, 0);
	}
}
