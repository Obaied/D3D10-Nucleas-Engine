//Laser Particle System using StreamOut techniques
cbuffer cbPerFrame
{	
	float4x4 gMatW;
	float4x4 gMatVP;
	float	 gGameTime;
	float	 gTimeStep;
	float	 gDelay;
	float	 gLimit;
	float3	 gEyePosW;
}

cbuffer cbConstants
{
	float3 gAccel  = float3(0.0f, 0.0f, 0.0f);
	float  gSpeed  = 10.0f;
}

Texture2D		gTex0;
Texture2D		gTex1;
Texture1D		gRandomTex;

const static float PI = 3.14159265358979323f;

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
	Particle p;

	//Age
	//==============================================
	p.Age = gIn[0].Age + gTimeStep;

	//Theta
	//==============================================
	if(p.Age > gDelay)
		p.Theta = gIn[0].Theta + gTimeStep*gSpeed*5.0f;
	else
		p.Theta = gIn[0].Theta;

	if(p.Theta >= 2.0f*PI)
		p.Theta = 0.0f;

	//Pos
	//==============================================
	//I want the particle to move on the z direction when it is activated and to 
	// move freely when it is being loaded
	float3 Pos	= float3(0.0f,0.0f,0.0f);
	float3 v	= float3(0.0f,0.0f,0.0f);
	if(p.Age > gDelay)
	{
		Pos.x = gIn[0].Radius*cos(p.Theta);
		Pos.y = gIn[0].Radius*sin(p.Theta);
		Pos.z = gIn[0].Pos.z;

		v = float3(0.0f,0.0f,1.0f) * 0.01f;
		Pos = Pos + v;
	}
	else
	{
		v = gIn[0].InitVelocity * 0.00001f;
		Pos = gIn[0].Pos + v;
	}

	if(Pos.z > gLimit)
		Pos.z = 0.0f;

	//Pos = mul(float4(Pos,1.0f), gMatW);
	p.Pos = Pos;

	//Radius & Size & Type & InitVelocity
	//==============================================
	p.InitVelocity	= gIn[0].InitVelocity;
	p.Radius		= gIn[0].Radius;
	p.Size			= gIn[0].Size;
	p.Type			= gIn[0].Type;

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
	if(vIn.Age < gDelay)
		Opacity = 0.1f;
	else
	{
		float t = vIn.Age - (gDelay-0.5f);
		Opacity = smoothstep(0.6f,1.0f, t);
	}

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
	uint   ID	 : ID;
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
		gOut.PosH	= mul(QuadVertices[i], WVP);
		gOut.TexC	= QuadTexC[i];
		gOut.Color	= gIn[0].Color;
		gOut.Age	= gIn[0].Age;
		gOut.ID		= PrimID;

		tStream.Append(gOut);
	}
}

float4 Draw_PS(GS_OUT pIn) : SV_TARGET
{
	float4 FinalColor = float4(0.0f,0.0f,0.0f,0.0f);
	float Opacity = 1.0f;

	if(pIn.ID%2 == 0)
		FinalColor = gTex0.Sample(sTriLinearWrap, pIn.TexC ); 
	else
		FinalColor = gTex1.Sample(sTriLinearWrap, pIn.TexC ); 

	FinalColor.rgb *= pIn.Color.rgb;
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