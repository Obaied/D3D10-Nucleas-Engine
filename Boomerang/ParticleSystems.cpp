#include "ParticleSystems.h"
#include "FX.h"
#include "Timer.h"

//Main Particle System & Instance
//============================================================
pSystem::pSystem()
	:mStreamOutVB(0)
	,mFxFilename(L"")
{
}

pSystem::~pSystem()
{	
	ReleaseCOM(mStreamOutVB);
}

UINT pSystem::GetMaxParticles()
{	return mMaxParticles;	}

UINT pSystem::GetMaxParticlesPerInst()
{	return (UINT)(mMaxParticles*0.3f);	}

const std::wstring& pSystem::GetFxFilename()
{	return mFxFilename;	}

//-------------------------------------------------------

PsInst::PsInst()
	:mVB(0),
	mFirstRun(true), mEnabled(true), 
	mDeathTimer(0.0f), mStartTime(0.0f), mSystemAge(0.0f),
	bDestroyed(false)
{
	mInitPos = static_cast<D3DXVECTOR3>(gZeroVec);
	mInitVec = static_cast<D3DXVECTOR3>(gZeroVec);
	D3DXMatrixIdentity(&mWorld);
}

PsInst::~PsInst()
{
	ReleaseCOM(mVB);
}

void PsInst::Enable()
{	mEnabled = true;	}

void PsInst::Disable()
{	mEnabled = false;	}

void PsInst::Destroy()
{
	bDestroyed = true;
}

void PsInst::IncrementDeathTimer(float dt)
{	mDeathTimer += dt;	}

ID3D10Buffer** PsInst::GetBuffer()
{	return &mVB;	}

D3DXVECTOR3& PsInst::GetInitPos()
{	return mInitPos;	}

D3DXVECTOR3& PsInst::GetInitVec()
{	return mInitVec;	}

D3DXMATRIX& PsInst::GetWorldMatrix()
{	return mWorld;	}

void PsInst::Drawn()
{	mFirstRun = false;	}

bool PsInst::IsEnabled()
{	return mEnabled;	}

bool PsInst::IsFirstRun()
{	return mFirstRun;	}

bool PsInst::IsDestroyed()
{    return bDestroyed;	}

float PsInst::GetDeathTimer()
{	return mDeathTimer;	}

float PsInst::GetStartTime()
{	return mStartTime;	}

float PsInst::GetSystemAge()
{	return mSystemAge;	}

//Pillar Particle System & Instance
PsPillarInst::PsPillarInst()
	:PsInst()
	,mIsInversed(false)
{
}

PsPillarInst::~PsPillarInst()
{
}

void PsPillarInst::Init(D3DXMATRIX World, D3DXVECTOR3 InitVec,
		bool IsInversed, float ThetaStep, float VelocityStep, float Radius,
		D3DXVECTOR2 Size)
{
	mWorld			= World;
	mInitPos		= D3DXVECTOR3(0.0f,0.0f,0.0f);
	mInitVec		= InitVec;
	mIsInversed		= IsInversed;
	mThetaStep		= ThetaStep;
	mVelocityStep	= VelocityStep;
	mRadius			= Radius;
	mSize			= Size;

	mStartTime = GetTimer().GetGameTime();

	BuildVB();
}

void PsPillarInst::Render()
{
	GetPsPillar().Render(*this);
	mSystemAge += GetTimer().GetDT();
}

void PsPillarInst::BuildVB()
{
	UINT MaxParticles = GetPsPillar().GetMaxParticles();
	D3D10_BUFFER_DESC desc;
	desc.ByteWidth		= sizeof(RotatingParticle) * MaxParticles;
	desc.BindFlags		= D3D10_BIND_VERTEX_BUFFER|D3D10_BIND_STREAM_OUTPUT;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags		= 0;
	desc.Usage			= D3D10_USAGE_DEFAULT;

	RotatingParticle* p  = new RotatingParticle[MaxParticles];
	D3DXVECTOR3 Velocity = mInitVec;
	float Theta			 = 0.0f;
	
	UINT Indicator = MaxParticles/10;
	Indicator /= 3;
	UINT iter = Indicator;

	for(UINT i = 0; i < MaxParticles; i++)
	{
		D3DXVECTOR3 Pos(0.0f,0.0f,0.0f);
		Pos.x = mRadius * cosf(Theta);
		Pos.y = 0.0f;
		Pos.z = mRadius * sinf(Theta);
		if(mIsInversed)
			Pos.x = -Pos.x;

		Pos = Pos + Velocity;
		p[i].Pos			= Pos;
		p[i].Age			= 0.0f;
		p[i].InitVelocity	= Velocity;
		p[i].Radius			= mRadius;
		p[i].Size			= mSize;
		p[i].Theta			= Theta;

		if(i == iter)
		{
			p[i].Type = 1;
			iter += Indicator;
		}
		else
			p[i].Type = 0;
		
		Velocity.y	+= mVelocityStep;
		Theta		+= mThetaStep;
	}

	D3D10_SUBRESOURCE_DATA InitData;
	InitData.pSysMem = p;

	HR(gd3dDev->CreateBuffer(&desc, &InitData, &mVB));
	delete[] p;
}

ID3D10Buffer** PsPillarInst::GetInitBuffer()
{	return &mInitVB;	}

D3DXMATRIX& PsPillarInst::GetWorld()
{	return mWorld;	}

bool PsPillarInst::GetIsInversed()
{	return mIsInversed;	}

float PsPillarInst::GetThetaStep()
{	return mThetaStep;	}

float PsPillarInst::GetVelocityStep()
{	return mVelocityStep;	}

//----------------------------------------------------------

PsPillar& GetPsPillar()
{
	static PsPillar ps;
	return ps;
}

PsPillar::PsPillar()
	:pSystem()
{
}

PsPillar::~PsPillar()
{
}

void PsPillar::Init(UINT MaxParticles, std::wstring FxFilename, 
	ID3D10ShaderResourceView* TexArraySRV)
{
	mMaxParticles = MaxParticles;
	mFxFilename   = FxFilename;
	ID3D10Effect* fx = FX::CreateFX(mFxFilename);

	mfxUpdateTech			= fx->GetTechniqueByName("UpdateTech");
	mfxDrawTech				= fx->GetTechniqueByName("DrawTech");

	mfxTexArray				= fx->GetVariableByName("gTexArray")->AsShaderResource();
	mfxRandomTex			= fx->GetVariableByName("gRandomTex")->AsShaderResource();
	mfxMatW					= fx->GetVariableByName("gMatW")->AsMatrix();
	mfxMatVP				= fx->GetVariableByName("gMatVP")->AsMatrix();
	mfxGameTime				= fx->GetVariableByName("gGameTime")->AsScalar();
	mfxTimeStep				= fx->GetVariableByName("gTimeStep")->AsScalar();
	mfxThetaStep			= fx->GetVariableByName("gThetaStep")->AsScalar();
	mfxVelocityStep			= fx->GetVariableByName("gVelocityStep")->AsScalar();
	mfxIsInversed			= fx->GetVariableByName("gIsInversed")->AsScalar();
	mfxInitVec				= fx->GetVariableByName("gInitVec")->AsVector();
	mfxEyePosW				= fx->GetVariableByName("gEyePosW")->AsVector();

	HR(mfxTexArray->SetResource(TexArraySRV));
	HR(mfxRandomTex->SetResource(GetTexManager().GetRandomTexture()));

	BuildVB();
}

void PsPillar::BuildVB()
{
	D3D10_BUFFER_DESC desc;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags		= 0;
	desc.Usage			= D3D10_USAGE_DEFAULT;
	desc.ByteWidth	= sizeof(RotatingParticle) * mMaxParticles;
	desc.BindFlags	= D3D10_BIND_VERTEX_BUFFER|D3D10_BIND_STREAM_OUTPUT;
	
	HR(gd3dDev->CreateBuffer(&desc, 0, &mStreamOutVB));
}

void PsPillar::Render(PsPillarInst& Inst)
{
	float dt = GetTimer().GetDT();
	float GameTime = GetTimer().GetGameTime();
	bool  Enabled = Inst.IsEnabled();
	bool  IsFirstRun = Inst.IsFirstRun();
	bool  IsInversed = Inst.GetIsInversed();
	float StartTime = Inst.GetStartTime();
	float ThetaStep = Inst.GetThetaStep();
	float VelocityStep = Inst.GetVelocityStep();
	D3DXVECTOR3 InitVec = Inst.GetInitVec();
	D3DXMATRIX  World	= Inst.GetWorld();
	ID3D10Buffer** DrawVB = Inst.GetBuffer();

	if(Enabled)
	{
		gd3dDev->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_POINTLIST);
		gd3dDev->IASetInputLayout(InputLayout::ILRotatingParticle);

		HR(mfxMatW->SetMatrix((float*)&World) );
		HR(mfxMatVP->SetMatrix((float*)&(GetCamera().ViewMatrix()*GetCamera().ProjMatrix() ) ) );
		HR(mfxTimeStep->SetFloat(dt));
		HR(mfxGameTime->SetFloat(GameTime));
		HR(mfxInitVec->SetFloatVector((float*)&InitVec));
		D3DXVECTOR4 EyePos = D3DXVECTOR4(GetCamera().Pos().x, GetCamera().Pos().y,
			GetCamera().Pos().z, 0.0f);
		HR(mfxEyePosW->SetFloatVector((float*)&EyePos));
		HR(mfxIsInversed->SetBool(IsInversed));
		HR(mfxThetaStep->SetFloat(ThetaStep) );
		HR(mfxVelocityStep->SetFloat(VelocityStep) );

		UINT Stride = sizeof(RotatingParticle);
		UINT Offset = 0;
		gd3dDev->IASetVertexBuffers(0, 1, DrawVB, &Stride, &Offset);

		gd3dDev->SOSetTargets(1, &mStreamOutVB, &Offset);

		D3D10_TECHNIQUE_DESC TechDesc;
		mfxUpdateTech->GetDesc(&TechDesc);

		for(UINT i = 0; i < TechDesc.Passes; i++)
		{
			ID3D10EffectPass* Pass = mfxUpdateTech->GetPassByIndex(i);
		
			HR(Pass->Apply(0));
			gd3dDev->Draw(mMaxParticles, 0);
		}

		//Unbind the vertex buffer from the SO stage
		ID3D10Buffer* BufferArray[1] = {0};
		gd3dDev->SOSetTargets(1, BufferArray, &Offset);

		//Swap the buffers and draw the updated geometry
		std::swap(mStreamOutVB, *DrawVB);
		gd3dDev->IASetVertexBuffers(0, 1, DrawVB, &Stride, &Offset);

		mfxDrawTech->GetDesc(&TechDesc);
		for(UINT i = 0; i < TechDesc.Passes; i++)
		{
			ID3D10EffectPass* Pass = mfxDrawTech->GetPassByIndex(i);

			HR(Pass->Apply(0));
			gd3dDev->DrawAuto();
		}
	}
}

//Laser Particle System & Instance
//===========================================

PsLaserInst::PsLaserInst()
	:PsInst()
{
	bIsAccelerating = false;
	Disable();
}

PsLaserInst::~PsLaserInst()
{
}

void PsLaserInst::Init(const D3DXVECTOR3& InitPos, const D3DXMATRIX& TransMatrix, 
					   const D3DXVECTOR3& RayDir, float Limit, float Delay)
{
	mInitPos = D3DXVECTOR3(0.0f,0.0f,0.0f);
	mInitVec = D3DXVECTOR3(0.0f,0.0f,0.0f);
	mStartTime = GetTimer().GetGameTime();
	mDelay	 = Delay;
	mLimit	 = Limit;
	mRayDir	 = RayDir;
	mInitPos = InitPos;

	D3DXVECTOR3 Source(0.0f,0.0f, 1.0f);
	D3DXVECTOR3 Target = RayDir;
	float dot = D3DXVec3Dot(&Source, &Target);
	float Angle = acosf(dot);
	D3DXMATRIX RotMatrix;
	D3DXVECTOR3 Axis;
	D3DXMatrixIdentity(&RotMatrix);
	D3DXVec3Cross(&Axis, &Source, &Target);
	D3DXVec3Normalize(&Axis, &Axis);
	D3DXMatrixRotationAxis(&RotMatrix, &Axis, Angle);
	mWorld = RotMatrix * TransMatrix;

	BuildVB();
}

void PsLaserInst::Render()
{
	if(mSystemAge > mDelay)
		bIsAccelerating = true;

	GetPsLaser().Render(*this);
	mSystemAge += GetTimer().GetDT();

}

void PsLaserInst::BuildVB()
{
	UINT MaxParticles = GetPsLaser().GetMaxParticlesPerInst();
	D3D10_BUFFER_DESC desc;
	desc.ByteWidth		= sizeof(RotatingParticle) * MaxParticles;
	desc.BindFlags		= D3D10_BIND_VERTEX_BUFFER|D3D10_BIND_STREAM_OUTPUT;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags		= 0;
	desc.Usage			= D3D10_USAGE_DEFAULT;

	D3D10_SUBRESOURCE_DATA InitData;
	RotatingParticle* p = new RotatingParticle[MaxParticles];
	float Step = 2.0f*PI;

	D3DXVECTOR3 Max(-FLT_MAX,-FLT_MAX,-FLT_MAX);
	D3DXVECTOR3 Min(FLT_MAX,FLT_MAX,FLT_MAX);

	for(UINT i = 0; i < MaxParticles; i++)
	{
		p[i].InitVelocity.x = RandF(-1.0f,1.0f);
		p[i].InitVelocity.y = RandF(-1.0f,1.0f);
		p[i].InitVelocity.z = RandF(-1.0f,1.0f);
		D3DXVec3Normalize(&p[i].InitVelocity, &p[i].InitVelocity);
		p[i].Theta  = RandF(0.0f, 2.0f*PI);
		p[i].Radius = RandF(0.5f, 1.0f);
		p[i].Size = D3DXVECTOR2(0.80f,1.00f);
		p[i].Age = 0.0f;
		p[i].Type = 0;

		D3DXVECTOR3 Pos;
		Pos.x = p[i].Radius*cosf(p[i].Theta);
		Pos.y = p[i].Radius*sinf(p[i].Theta);
		Pos.z = RandF(0.0f, mLimit);
		p[i].Pos = Pos;
		
		if(Pos.x > Max.x)
			Max.x = Pos.x;
		if(Pos.y > Max.y)
			Max.y = Pos.y;
		if(Pos.z > Max.z)
			Max.z = Pos.z;

		if(Pos.x < Min.x)
			Min.x = Pos.x;
		if(Pos.y < Min.y)
			Min.y = Pos.y;
		if(Pos.z < Min.z)
			Min.z = Pos.z;
	}

	mBox.Min = Min;
	mBox.Max = Max;
	mBox.Min.x *= 0.1f;
	mBox.Min.y *= 0.1f;
	mBox.Max.x *= 0.1f;
	mBox.Max.y *= 0.1f;
	mBox.ApplyMatrix(mWorld);

	InitData.pSysMem = p;
	HR(gd3dDev->CreateBuffer(&desc, &InitData, &mVB));
	delete[] p;
}

const AABB& PsLaserInst::GetAABB()
{	return mBox;	}

float PsLaserInst::GetLimit()
{	return mLimit;	}

float PsLaserInst::GetDelay()
{	return mDelay;	}

D3DXVECTOR3 PsLaserInst::GetRayDir()
{	return mRayDir;	}

bool PsLaserInst::IsAccelerating()
{   return bIsAccelerating;		}

//----------------------------------------------------------

PsLaser& GetPsLaser()
{
	static PsLaser ps;
	return ps;
}

PsLaser::PsLaser()
	:pSystem()
{
}

PsLaser::~PsLaser()
{
}

UINT PsLaser::GetMaxParticlesPerInst()
{
	return 5000;
}

void PsLaser::Init(UINT MaxParticles, std::wstring FxFilename, float DeathTime,
	ID3D10ShaderResourceView* Tex0SRV, ID3D10ShaderResourceView* Tex1SRV)
{
	mMaxParticles = MaxParticles;
	mFxFilename   = FxFilename;
	mDeathTime    = DeathTime;
	ID3D10Effect* fx = FX::CreateFX(mFxFilename);

	mfxUpdateTech			= fx->GetTechniqueByName("UpdateTech");
	mfxDrawTech				= fx->GetTechniqueByName("DrawTech");

	mfxTex0					= fx->GetVariableByName("gTex0")->AsShaderResource();
	mfxTex1					= fx->GetVariableByName("gTex1")->AsShaderResource();
	mfxRandomTex			= fx->GetVariableByName("gRandomTex")->AsShaderResource();
	mfxMatW					= fx->GetVariableByName("gMatW")->AsMatrix();
	mfxMatVP				= fx->GetVariableByName("gMatVP")->AsMatrix();
	mfxGameTime				= fx->GetVariableByName("gGameTime")->AsScalar();
	mfxTimeStep				= fx->GetVariableByName("gTimeStep")->AsScalar();
	mfxLimit				= fx->GetVariableByName("gLimit")->AsScalar();
	mfxDelay				= fx->GetVariableByName("gDelay")->AsScalar();
	mfxEyePosW				= fx->GetVariableByName("gEyePosW")->AsVector();

	HR(mfxTex0->SetResource(Tex0SRV));
	HR(mfxTex1->SetResource(Tex1SRV));
	HR(mfxRandomTex->SetResource(GetTexManager().GetRandomTexture()));

	BuildVB();
}

void PsLaser::BuildVB()
{
	D3D10_BUFFER_DESC desc;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags		= 0;
	desc.Usage			= D3D10_USAGE_DEFAULT;
	desc.ByteWidth	= sizeof(RotatingParticle) * mMaxParticles;
	desc.BindFlags	= D3D10_BIND_VERTEX_BUFFER|D3D10_BIND_STREAM_OUTPUT;
	
	HR(gd3dDev->CreateBuffer(&desc, 0, &mStreamOutVB));
}

void PsLaser::Render(PsLaserInst& Inst)
{
	float dt = GetTimer().GetDT();
	float GameTime = GetTimer().GetGameTime();
	float StartTime = Inst.GetStartTime();
	float Limit		= Inst.GetLimit();
	float Delay		= Inst.GetDelay();
	bool  Enabled = Inst.IsEnabled();
	bool  IsFirstRun = Inst.IsFirstRun();
	D3DXVECTOR3 InitPos = Inst.GetInitPos();
	D3DXVECTOR3 InitVec = Inst.GetInitVec();
	ID3D10Buffer** DrawVB = Inst.GetBuffer();

	D3DXMATRIX  World	 = Inst.GetWorldMatrix();

	if(Enabled)
	{
		gd3dDev->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_POINTLIST);
		gd3dDev->IASetInputLayout(InputLayout::ILRotatingParticle);

		HR(mfxMatW->SetMatrix((float*)&World) );
		HR(mfxMatVP->SetMatrix((float*)&(GetCamera().ViewMatrix()*GetCamera().ProjMatrix() ) ) );
		HR(mfxTimeStep->SetFloat(dt));
		HR(mfxGameTime->SetFloat(GameTime));
		HR(mfxEyePosW->SetFloatVector((float*)&GetCamera().Pos()));
		HR(mfxLimit->SetFloat(Limit) );
		HR(mfxDelay->SetFloat(Delay) );

		UINT Stride = sizeof(RotatingParticle);
		UINT Offset = 0;
		gd3dDev->IASetVertexBuffers(0, 1, DrawVB, &Stride, &Offset);

		gd3dDev->SOSetTargets(1, &mStreamOutVB, &Offset);

		D3D10_TECHNIQUE_DESC TechDesc;
		mfxUpdateTech->GetDesc(&TechDesc);

		for(UINT i = 0; i < TechDesc.Passes; i++)
		{
			ID3D10EffectPass* Pass = mfxUpdateTech->GetPassByIndex(i);
		
			HR(Pass->Apply(0));
			if(IsFirstRun)
			{
				gd3dDev->Draw(mMaxParticles, 0);
				Inst.Drawn();
			}
			else
				gd3dDev->DrawAuto();
		}

		//Unbind the vertex buffer from the SO stage
		ID3D10Buffer* BufferArray[1] = {0};
		gd3dDev->SOSetTargets(1, BufferArray, &Offset);

		//Swap the buffers and draw the updated geometry
		std::swap(mStreamOutVB, *DrawVB);
		gd3dDev->IASetVertexBuffers(0, 1, DrawVB, &Stride, &Offset);

		mfxDrawTech->GetDesc(&TechDesc);
		for(UINT i = 0; i < TechDesc.Passes; i++)
		{
			ID3D10EffectPass* Pass = mfxDrawTech->GetPassByIndex(i);

			HR(Pass->Apply(0));
			gd3dDev->DrawAuto();
		}
	}

}

float PsLaser::GetDeathTime()
{	return mDeathTime;	}