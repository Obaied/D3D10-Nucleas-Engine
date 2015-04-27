#ifndef CAMERA_H
#define CAMERA_H

#include "D3DUtil.h"

class Camera
{
public:
	friend Camera& GetCamera();
	Camera();
	~Camera();

	void SetLens(float Fovy, float Aspect, float MinZ, float MaxZ);
	void LookAt(const D3DXVECTOR3& Pos, const D3DXVECTOR3& Target);

	D3DXMATRIX& ViewMatrix();
	D3DXMATRIX& ProjMatrix();

	D3DXVECTOR3& Pos();
	D3DXVECTOR3& OldPos();
	D3DXVECTOR3& Right();
	D3DXVECTOR3& Up();
	D3DXVECTOR3& Look();
	D3DXVECTOR3& RadiusVec();

private:
	D3DXMATRIX mProj;
	D3DXMATRIX mView;

	D3DXVECTOR3 mPos;
	D3DXVECTOR3 mRight;
	D3DXVECTOR3 mUp;
	D3DXVECTOR3 mLook;

	D3DXVECTOR3 mEllRadius;
};

#endif