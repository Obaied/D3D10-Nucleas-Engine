#ifndef AI_H
#define AI_H

#include "D3DUtil.h"
#include "MeshManager.h"

struct AI
{
	AI()
	{
		IsMoving = false;
		Pos = D3DXVECTOR3(0.0f,0.0f,0.0f);
		Vel = DIR_UP;
		ExplosionAmt = 0.0f;
		IsLaserTurret = false;
		D3DXMatrixIdentity(&World);
	}
	D3DXVECTOR3 Pos;
	D3DXVECTOR2 Vel;
	BS			Sphere;
	bool		IsMoving;
	D3DXMATRIX	World;
	float		ExplosionAmt;
	UINT		Index;
	bool		IsLaserTurret;
};

class AIManager
{
public:
	AIManager();
	~AIManager();

	void Init();
	const AI& CreateAI(const D3DXVECTOR3& Pos, const D3DXVECTOR2& Vel,
		float Radius, bool IsMoving, bool IsTurret);
	void UpdateAI(UINT Num, float Amt);
	void DestroyAIWithPowerDestroy();
	void ChangeDirection(UINT Num, const D3DXVECTOR2& Vel);
	void DrawSphere(UINT Index);

	UINT	  GetNumAI();
	const AI& GetConstAI(UINT i);
	AI&		  GetAI(UINT i);
	const D3DXMATRIX& GetScalingMatrix();

private:
	std::vector<AI> mAIs;
	Mesh	mSphereData;
	UINT	mIndex;
};

#endif