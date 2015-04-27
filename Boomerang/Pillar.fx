//Pillar particle system using OS Effect File to render a Helix pillar effect 
cbuffer cbPerFrame
{	
	float4x4 gMatW;
	float4x4 gMatVP;
	float	 gGameTime;
	float	 gTimeStep;
	float	 gThetaStep;
	float	 gVelocityStep;
	
	float4	 gInitVec;
	float4	 gEyePosW;
	bool	 gIsInversed;
}

cbuffer cbConstants
{
	float3 gAccel  = float3(0.0f, -0.0001f, 0.0f);
}

Texture2DArray	gTexArray;
Texture1D		gRandomTex;

static const float PI = 3.14159265358979323f;
#define VT_EMITTER 1
#define VT_FLARE   0

SamplerState sTriLinearWrap
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Wrap;
	AddressV = Wrap;
};

//Helper Functions
//=================================================================
float RandomFloat(float Offset)
{
	float t = gGameTime + Offset;

	return gRandomTex.SampleLevel(sTriLinearWrap, t, 0).r;
}

float RandomFloat(float A, float B, float Offset)
{
	float t = gGameTime + Offset;

	float C = gRandomTex.SampleLevel(sTriLinearWrap, t, 0).r;

	C = (C + 1.0f)/2.0f;
	C = C * (B-A) + A;

	return C;
}

float3 RandomVec3(float Offset)
{
	float t = gGameTime + Offset;

	float3 C = gRandomTex.SampleLevel(sTriLinearWrap, t, 0).rgb;

	return normalize(C);
}

//================================================================
struct Particle
{
	float3 Pos			: POSITION;
	float3 InitVelocity	: VELOCITY;
	float2 Size			: SIZE;
	float  Age			: AGE;
	float  Theta		: THETA;
	float  Radius		: RADIUS;
	int	   Type			: TYPE;
};

Particle Update_VS(Particle vIn)
{
	return vIn;
}

[maxvertexcount(1)]
void Update_GS(point Particle gIn[1],
		uint PrimID : SV_PRIMITIVEID,
		inout PointStream<Particle> pStream)
{
	//Update the emitter particle
	//-------------------------------------------------------------
	Particle p;

	//Theta
	//==========================
	p.Theta = gIn[0].Theta + 0.5f;

	//Radius
	//==========================
	p.Radius = gIn[0].Radius;

	//Age
	//==========================
	p.Age = gIn[0].Age + gTimeStep;

	//Velocity
	//==========================
	p.InitVelocity = gIn[0].InitVelocity + float3(0.0f,0.0007f,0.0f);

	//Pos
	//==========================
	float3 Pos  = float3(0.0f,0.0f,0.0f);
	float3 v0	= p.InitVelocity;
	Pos.x = p.Radius*cos(p.Theta);
	Pos.y = 0.0f;
	Pos.z = p.Radius*sin(p.Theta);
	//if(gIsInversed)
	//	Pos.x = -Pos.x;

	Pos = Pos + v0;
	//Pos = mul(float4(Pos,1.0f), gMatW);
	p.Pos = Pos;
		
	if(p.Pos.y >= 100.0f)
		p.InitVelocity = gInitVec.xyz;

	//Size
	//==========================
	p.Size = gIn[0].Size;

	//Type
	//==========================
	p.Type = gIn[0].Type;

	pStream.Append(p);
}

GeometryShader gsStreamOut =  ConstructGSWithSO(
	CompileShader(gs_4_0, Update_GS() ),
	"POSITION.xyz; VELOCITY.xyz; SIZE.xy; AGE.x; THETA.x; RADIUS.x; TYPE.x");

DepthStencilState DisableDepthDSS
{
	DepthEnable		= false;
	DepthWriteMask	= ZERO;
};
					
technique10 UpdateTech
{
	pass p0
	{
		SetVertexShader(CompileShader(vs_4_0, Update_VS() ) );
		SetGeometryShader(gsStreamOut);
		SetPixelShader(NULL);
		
		SetDepthStencilState(DisableDepthDSS, 0);
	}
}

//Draw Tech
//========================================================

struct VS_OUT
{
	float3 PosL  : POSITION;
	float2 Size	 : SIZE;
	float4 Color : COLOR;
	float  Age	 : AGE;
	uint   Type  : TYPE;
};

VS_OUT Draw_VS(Particle vIn)
{
	VS_OUT vOut = (VS_OUT)0;

	vOut.PosL  = vIn.Pos;
	vOut.Size  = vIn.Size;
	vOut.Age   = vIn.Age;

	float Opacity = 0.0f;
	if(vIn.Type == VT_EMITTER)
		Opacity = 1.0f;
	else if(vIn.Type == VT_FLARE)
		Opacity = 0.3f;

	vOut.Color = float4(1.0f, 1.0f, 1.0f, Opacity);
	vOut.Type = vIn.Type;
	
	return vOut;
}

struct GS_OUT
{
	float4 PosH  : SV_POSITION;
	float2 TexC	 : TEXCOORD;
	float4 Color : COLOR;
	float  Age   : AGE;
	uint   Type	 : TYPE;
};

[maxvertexcount(4)]
void Draw_GS(point VS_OUT gIn[1],
		uint PrimID : SV_PRIMITIVEID,
		inout TriangleStream<GS_OUT> tStream)
{
	float3 Look  = normalize(gEyePosW.xyz - gIn[0].PosL);
	float3 Right = normalize(cross(float3(0.0f,1.0f,0.0f), Look ) );
	float3 Up	 = normalize(cross(Look, Right) );

	float4x4 BBWorld;
	BBWorld[0] = float4(Right,0.0f);
	BBWorld[1] = float4(Up,0.0f);
	BBWorld[2] = float4(Look,0.0f);
	BBWorld[3] = float4(gIn[0].PosL,1.0f);

	float4x4 WVP = mul(BBWorld, gMatW);
	WVP = mul(WVP, gMatVP);

	float HalfWidth  = 0.5f*gIn[0].Size.x;
	float HalfHeight = 0.5f*gIn[0].Size.y;

	float4 QuadVertices[4];
	QuadVertices[0] = float4(-HalfWidth,-HalfHeight, 0.0f, 1.0f);
	QuadVertices[1] = float4( HalfWidth,-HalfHeight, 0.0f, 1.0f);
	QuadVertices[2] = float4(-HalfWidth, HalfHeight, 0.0f, 1.0f);
	QuadVertices[3] = float4( HalfWidth, HalfHeight, 0.0f, 1.0f);

	float2 QuadTexC[4];
	QuadTexC[0] = float2(0.0f,1.0f);
	QuadTexC[1] = float2(1.0f,1.0f);
	QuadTexC[2] = float2(0.0f,0.0f);
	QuadTexC[3] = float2(1.0f,0.0f);

	GS_OUT gOut;
	[unroll]
	for(int i = 0; i < 4; i++)
	{
		gOut.PosH = mul(QuadVertices[i], WVP);
		gOut.TexC = QuadTexC[i];
		gOut.Color = gIn[0].Color;
		gOut.Age   = gIn[0].Age;
		gOut.Type  = gIn[0].Type;

		tStream.Append(gOut);
	}
}

float4 Draw_PS(GS_OUT pIn) : SV_TARGET
{
	float4 FinalColor = float4(0.0f,0.0f,0.0f,0.0f);
		FinalColor = gTexArray.Sample(sTriLinearWrap, float3(pIn.TexC,0) ); 
		FinalColor *= pIn.Color;
		FinalColor.a = pIn.Color.a;

	return FinalColor;
}
	
DepthStencilState NoDepthWriteDDS
{
	DepthEnable = true;
	DepthWriteMask = ZERO;
};

BlendState AdditiveBlending
{
	AlphaToCoverageEnable = false;
	BlendEnable[0] = true;
	SrcBlend  = SRC_ALPHA;
	DestBlend = ONE;
	BlendOP   = ADD;
	SrcBlendAlpha  = ZERO;
	DestBlendAlpha = ZERO;
	BlendOpAlpha   = ADD;
	RenderTargetWriteMask[0] = 0x0f;
};


technique10 DrawTech
{
	pass p0
	{
		SetVertexShader(	CompileShader(vs_4_0, Draw_VS() ) );
		SetGeometryShader(	CompileShader(gs_4_0, Draw_GS() ) );
		SetPixelShader(		CompileShader(ps_4_0, Draw_PS() ) );

		SetDepthStencilState(NoDepthWriteDDS,0);
		SetBlendState(AdditiveBlending, float4(0.0f,0.0f,0.0f,0.0f), 0xffffffff);
	}
}