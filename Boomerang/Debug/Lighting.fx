
struct Light
{
	float3 Pos;
	float  Pad0;
	float3 Dir;
	float  Pad1;

	float4 Diffuse;
	float4 Ambient;
	float4 Specular;
	float3 Atten;
	float  Pad2;

	float SpotPower;
	float Range;
};

struct Material
{
	float4 Diffuse;
	float4 Ambient;
	float4 Specular;
	float  SpecPower;
};

struct SurfaceInfo
{
	float4 Diffuse;
	float4 Specular;
	float3 NormalW;
	float3 PosW;
};

SamplerState s_TriPointClamp
{
	Filter = MIN_MAG_MIP_POINT;
	AddressU = CLAMP;
	AddressV = CLAMP;
};

float CalcShadowFactor(Texture2D ShadowMap, float MapSize, float MapDX, float Epsilon
, float4 ProjTexC)
{
	ProjTexC.xyz /= ProjTexC.w;

	if(ProjTexC.x > 1.0f || ProjTexC.x < -1.0f
	|| ProjTexC.y > 1.0f || ProjTexC.y < -1.0f
	|| ProjTexC.z > 1.0f || ProjTexC.z < 0.0f)
		return 0.0f;

	ProjTexC.x = 0.5f*ProjTexC.x + 0.5f;
	ProjTexC.y = -0.5f*ProjTexC.y + 0.5f;

	float Depth = ProjTexC.z;

	float d0 = ShadowMap.Sample(s_TriPointClamp, ProjTexC.xy).r + Epsilon >= Depth ? 1 : 0;
	float d1 = ShadowMap.Sample(s_TriPointClamp, ProjTexC.xy + float2(MapDX,0.0f) ).r + Epsilon >= Depth ? 1 : 0;
	float d2 = ShadowMap.Sample(s_TriPointClamp, ProjTexC.xy + float2(0.0f,MapDX) ).r + Epsilon >= Depth ? 1 : 0;
	float d3 = ShadowMap.Sample(s_TriPointClamp, ProjTexC.xy + float2(MapDX,MapDX) ).r+ Epsilon >= Depth ? 1 : 0;

	float2 TexelPos = ProjTexC.xy*MapSize;
	float2 FracTexelPos = frac(TexelPos);

	return	lerp( 
				lerp(d0,d1, FracTexelPos.x),
				lerp(d2,d3, FracTexelPos.x),
				FracTexelPos.y);
}

float4 CalcPointLight(SurfaceInfo s, Light l, float3 EyePosW, float ShadowFactor)
{
	float3 EyeVecW = normalize(EyePosW - s.PosW);

	float3 LightVecW = l.Pos - s.PosW;
	float R = length(LightVecW);
	//Normalize the lightvec
	LightVecW /= R;
	
	if(R > l.Range)
		return float4(0.0f,0.0f,0.0f,0.0f);

	float DiffuseFactor = dot(s.NormalW, LightVecW);
	float3 AmbTerm = (l.Ambient * s.Diffuse).rgb;

	[branch]
	if(DiffuseFactor > 0.0f)
	{
		float3 Diff = l.Diffuse.xyz * s.Diffuse.xyz * DiffuseFactor;

		float3 Spec = float3(0.0f,0.0f,0.0f);
		if(s.Specular.r != 0.0f 
		&& s.Specular.g != 0.0f
		&& s.Specular.b != 0.0f
		&& s.Specular.a != 0.0f )
		{
			float3 RefLightVecW = reflect(-LightVecW, s.NormalW);
			float SpecPower = max(l.Specular.a,1.0f);
			Spec = l.Specular.xyz * s.Specular.xyz * pow( max( dot(EyeVecW.xyz, RefLightVecW.xyz), 0.0f), SpecPower);
		}

		float3 LitColor = Diff + Spec;
		[branch]
		if(ShadowFactor != -1.0f)
			LitColor *= ShadowFactor;

		LitColor += AmbTerm;

		LitColor/ dot(l.Atten, float3(1.0f,R,R*R) );

		return float4(LitColor, s.Diffuse.a);
	}
	return float4(AmbTerm, s.Diffuse.a);
}

float4 CalcDirLight(SurfaceInfo s, Light l, float3 EyePosW, float ShadowFactor)
{
	float3 LightVecW = -l.Dir;

	float DiffuseFactor = dot(s.NormalW, LightVecW);
	float3 LitColor = float3(0.0f, 0.0f, 0.0f);
	float3 AmbTerm  = (l.Ambient*s.Diffuse).rgb;

	[branch]
	if(DiffuseFactor > 0.0f)
	{
		float3 EyeVecW = normalize(EyePosW - s.PosW);
		float3 RefLightVecW = reflect(l.Dir, s.NormalW);

		float3 SpecTerm = float3(0.0f,0.0f,0.0f);
		if(s.Specular.r != 0.0f 
		&& s.Specular.g != 0.0f
		&& s.Specular.b != 0.0f
		&& s.Specular.a != 0.0f )
		{
			float SpecPower = max(l.Specular.a,1.0f);
			SpecTerm = pow( max( dot(EyeVecW, RefLightVecW), 0.0f), SpecPower) * (l.Specular*s.Specular).rgb;
		}

		float3 DiffTerm = DiffuseFactor*(l.Diffuse*s.Diffuse).rgb;
		LitColor += DiffTerm + SpecTerm;
		
		if(ShadowFactor != -1.0f)
			LitColor *= ShadowFactor;

		LitColor += AmbTerm;
		return float4(LitColor,s.Diffuse.a);

	}
	return float4(AmbTerm,s.Diffuse.a);
}

float4 CalcSpotLight(SurfaceInfo s, Light l, float3 EyePosW, float ShadowFactor)
{
	float4 LitColor = CalcPointLight(s, l, EyePosW, ShadowFactor);

	float3 LightVecW = normalize(l.Pos- s.PosW);
	float  SpotPower = max(l.SpotPower,1.0f);

	float  SpotFactor = pow( max( dot(-LightVecW,l.Dir), 0.0f), SpotPower);

	return LitColor * SpotFactor;
}