#ifndef _PARTICLE_H
#define _PARTICLE_H

#include "D3DUtil.h"
#include "TexManager.h"
#include "InputLayout.h"
#include "Camera.h"

struct RotatingParticle
{
	D3DXVECTOR3 Pos;
	D3DXVECTOR3 InitVelocity;
	D3DXVECTOR2 Size;
	float		Age;
	float		Theta;
	float		Radius;
	int			Type;
};

//Main Particle System & Instance
//===========================================================
class pSystem
{
public:
	pSystem();
	virtual ~pSystem();
	UINT				GetMaxParticles();
	UINT				GetMaxParticlesPerInst();
	const std::wstring& GetFxFilename();

protected:
	virtual void BuildVB() = 0;
	pSystem(const pSystem& rhs);
	pSystem& operator=(const pSystem& rhs);

protected:
	std::wstring mFxFilename;
	UINT		 mMaxParticles;

	ID3D10Buffer* mStreamOutVB;
};

class PsInst
{
public:
	PsInst();
	virtual ~PsInst();
	virtual void Render() = 0;

	void Enable();
	void Disable();
	void Drawn();
	void Destroy();

	bool IsEnabled();
	bool IsFirstRun();
	bool IsDestroyed();

	float GetDeathTimer();
	float GetStartTime();
	float GetSystemAge();
	ID3D10Buffer** GetBuffer();

	D3DXMATRIX&  GetWorldMatrix();
	D3DXVECTOR3& GetInitPos();
	D3DXVECTOR3& GetInitVec();

	void IncrementDeathTimer(float dt);

protected:
	virtual void BuildVB() = 0;
	PsInst(const PsInst& rhs);
	PsInst& operator=(const PsInst& rhs);

protected:

	ID3D10Buffer* mVB;
	D3DXMATRIX	mWorld;
	D3DXVECTOR3 mInitPos;
	D3DXVECTOR3 mInitVec;

	float	mStartTime;
	float	mDeathTimer;
	float	mSystemAge;
	bool	mFirstRun;
	bool	mEnabled;
	bool	bDestroyed;
};

//Pillar Particle system & Instance
class PsPillarInst : public PsInst
{
public:
	PsPillarInst();
	virtual ~PsPillarInst();

	void Init(D3DXMATRIX World, D3DXVECTOR3 InitVec,
		bool IsInversed, float ThetaStep, float VelocityStep, float Radius,
		D3DXVECTOR2 Size);
	virtual void Render();

	ID3D10Buffer**	GetInitBuffer();
	D3DXMATRIX&		GetWorld();
	bool			GetIsInversed();
	float			GetDelay();
	void			ResetDelay();
	float			GetThetaStep();
	float			GetVelocityStep();

protected:
	virtual void BuildVB();
	PsPillarInst(const PsPillarInst& rhs);
	PsPillarInst& operator=(const PsPillarInst& rhs);

protected:
	ID3D10Buffer* mInitVB;
	bool  mIsInversed;
	float mThetaStep;
	float mVelocityStep;
	float mRadius;
	D3DXVECTOR2 mSize;
};

class PsPillar : public pSystem
{
	friend PsPillar& GetPsPillar();
public:
	PsPillar();
	virtual ~PsPillar();

	void Init(UINT MaxParticles, std::wstring FxFilename, 
		ID3D10ShaderResourceView* TexArraySRV);
	void Render(PsPillarInst& Inst);

protected:
	virtual void BuildVB();
	PsPillar(const PsPillar& rhs);
	PsPillar& operator=(const PsPillar& rhs);

protected:

	ID3D10EffectTechnique*				mfxUpdateTech;
	ID3D10EffectTechnique*				mfxDrawTech;

	ID3D10EffectShaderResourceVariable*	mfxTexArray;
	ID3D10EffectShaderResourceVariable* mfxRandomTex;
	ID3D10EffectMatrixVariable*			mfxMatW;
	ID3D10EffectMatrixVariable*			mfxMatVP;
	ID3D10EffectScalarVariable*			mfxGameTime;
	ID3D10EffectScalarVariable*			mfxTimeStep;
	ID3D10EffectScalarVariable*			mfxThetaStep;
	ID3D10EffectScalarVariable*			mfxVelocityStep;
	ID3D10EffectScalarVariable*			mfxDelay;
	ID3D10EffectScalarVariable*			mfxIsInversed;
	ID3D10EffectVectorVariable*			mfxInitVec;
	ID3D10EffectVectorVariable*			mfxEyePosW;
};

//Laser Particle system & Instance
class PsLaserInst : public PsInst
{
public:
	PsLaserInst();
	virtual ~PsLaserInst();

	void Init(const D3DXVECTOR3& InitPos, const D3DXMATRIX& TransMatrix, 
					   const D3DXVECTOR3& RayDir, float Limit, float Delay);
	virtual void Render();

	const AABB&			GetAABB();
	D3DXVECTOR3			GetRayDir();
	float				GetDelay();
	float				GetLimit();
	bool				IsAccelerating();


protected:
	virtual void BuildVB();
	PsLaserInst(const PsLaserInst& rhs);
	PsLaserInst& operator=(const PsLaserInst& rhs);

protected:
	D3DXVECTOR3 mRayDir;
	AABB		mBox;

	float		mDelay;
	float		mLimit;
	bool		bIsAccelerating;
};

class PsLaser : public pSystem
{
	friend PsLaser& GetPsLaser();
public:
	PsLaser();
	virtual ~PsLaser();

	UINT GetMaxParticlesPerInst();
	void Init(UINT MaxParticles, std::wstring FxFilename, float DeathTime,
		ID3D10ShaderResourceView* Tex0SRV, ID3D10ShaderResourceView* Tex1SRV);
	void Render(PsLaserInst& Inst);

	float GetDeathTime();

protected:
	virtual void BuildVB();
	PsLaser(const PsLaser& rhs);
	PsLaser& operator=(const PsLaser& rhs);

protected:

	ID3D10EffectTechnique*				mfxUpdateTech;
	ID3D10EffectTechnique*				mfxDrawTech;

	ID3D10EffectShaderResourceVariable*	mfxTex0;
	ID3D10EffectShaderResourceVariable*	mfxTex1;
	ID3D10EffectShaderResourceVariable* mfxRandomTex;
	ID3D10EffectMatrixVariable*			mfxMatW;
	ID3D10EffectMatrixVariable*			mfxMatVP;
	ID3D10EffectScalarVariable*			mfxGameTime;
	ID3D10EffectScalarVariable*			mfxTimeStep;
	ID3D10EffectScalarVariable*			mfxDelay;
	ID3D10EffectScalarVariable*			mfxLimit;
	ID3D10EffectVectorVariable*			mfxEyePosW;

	float	mDeathTime;
};

#endif