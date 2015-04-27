#include "AIManager.h"

struct SphereVertex
{
	D3DXVECTOR3 Pos;
};

AIManager::AIManager()
{
	mIndex = 0;
}

AIManager::~AIManager()
{
	mAIs.clear();
}

void AIManager::Init()
{
	mSphereData = GetMeshManager().CreateSphere();
}

const AI& AIManager::CreateAI(const D3DXVECTOR3& Pos, const D3DXVECTOR2& Vel,
		float Radius, bool IsMoving = false, bool IsTurret = false)
{
	AI a;

	a.Pos			= Pos;
	a.Vel			= Vel;
	a.Sphere.Center = Pos;
	a.Sphere.Radius = Radius;
	a.IsMoving		= IsMoving;
	a.Index			= mIndex;
	a.IsLaserTurret	= IsTurret;
	mIndex++;

	D3DXMATRIX S, T;
	D3DXMatrixTranslation(&T, a.Pos.x, a.Pos.y, a.Pos.z);
	D3DXMatrixScaling(&S, Radius, Radius, Radius);
	a.World = S*T;

	UINT size = mAIs.size();
	mAIs.push_back(a);
	return mAIs[size];
}

void AIManager::UpdateAI(UINT Num, float Amt)
{
	AI& a = mAIs[Num];
	D3DXVECTOR3 LookDir(a.Vel.x, 0.0f, a.Vel.y);
	a.Pos = a.Pos + LookDir*Amt;

	D3DXMatrixTranslation(&a.World, a.Pos.x, a.Pos.y, a.Pos.z);
	a.Sphere.Center = a.Pos;
}

void AIManager::DestroyAIWithPowerDestroy()
{
	for(UINT i = 0; i < mAIs.size(); i++)
	{
		if(mAIs[i].IsMoving == false 
			&& mAIs[i].ExplosionAmt != 0.0f)
		{
			mAIs.erase(mAIs.begin() + i);
			i--;
		}
	
		mIndex--;
	}
}

void AIManager::ChangeDirection(UINT Num, const D3DXVECTOR2& Vel)
{
	mAIs[Num].Vel = Vel;
}

UINT AIManager::GetNumAI()
{
	return mAIs.size();
}

const AI& AIManager::GetConstAI(UINT i)
{
	if(i > mAIs.size() )
		Error("GetConstAI: Error trying to access mAIs with a subscript that is over its size");
	return mAIs[i];
}

AI& AIManager::GetAI(UINT i)
{
	if(i > mAIs.size() )
		Error("GetAI: Error trying to access mAIs with a subscript that is over its size");
	return mAIs[i];
}

const D3DXMATRIX& AIManager::GetScalingMatrix()
{
	return mSphereData.ScalingMatrix;
}

void AIManager::DrawSphere(UINT Index)
{
	HR(mSphereData.Data->DrawSubset(Index));
}