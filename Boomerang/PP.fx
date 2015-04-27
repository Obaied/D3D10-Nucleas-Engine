cbuffer cbPerFrame
{
	float gOpacity;
}

Texture2D gQuadTex;
Texture2D gOriginalTex;
Texture2D gScreenTex;

SamplerState sTriLinearWrap
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = WRAP;
	AddressV = WRAP;
};

struct VS_IN
{
	float3 PosL : POSITION;
	float2 TexC : TEXCOORD;
};

struct VS_OUT
{
	float4 PosH : SV_POSITION;
	float2 TexC : TEXCOORD;
};

VS_OUT Quad_VS(VS_IN vIn)
{
	VS_OUT v;
	v.PosH = float4(vIn.PosL,1.0f);
	v.TexC = vIn.TexC;

	return v;
}

#define PS_IN VS_OUT

struct PS_OUT
{
	float4 Color : SV_TARGET;
};

float4 Opacity_PS(PS_IN pIn) : SV_TARGET
{
	//ALWAYS ALWAYS ALWAYS NEVER TRUST THE ALPHA VALUE FROM TEXTURES
	// IF IT IS NOT USED IN ALPHA BLENDING OR CULLING
	//I HAD A BLACK TEXTURE ON THE PARTICLE SYSTEMS BECAUSE I 
	//USED A FLOAT4 AND USED THE ALPHA VALUE FROM THE TEXTURES

	float3 Tex1 = gOriginalTex.Sample(sTriLinearWrap, pIn.TexC);
	float3 Tex0 = gScreenTex.Sample(sTriLinearWrap, pIn.TexC);

	float3 FinalColor = (gOpacity)*Tex0 + (1.0f-gOpacity)*Tex1;

	return float4(FinalColor,1.0f);
}

technique10 OpacityTech
{
	pass p0
	{
		SetVertexShader(	CompileShader(vs_4_0, Quad_VS() )	);
		SetGeometryShader(	NULL	);
		SetPixelShader(		CompileShader(ps_4_0, Opacity_PS() )	);
	}
}

//Quad Tech
//=================================================================
float4 Quad_PS(PS_IN pIn) : SV_TARGET
{
	float3 FinalColor = gQuadTex.Sample(sTriLinearWrap, pIn.TexC).rgb;

	return float4(FinalColor,1.0f);
}

technique10 QuadTech
{
	pass p0
	{
		SetVertexShader(	CompileShader(vs_4_0, Quad_VS() )	);
		SetGeometryShader(	NULL	);
		SetPixelShader(		CompileShader(ps_4_0, Quad_PS() )	);
	}
}