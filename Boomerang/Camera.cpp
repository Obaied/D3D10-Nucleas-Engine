#include "Camera.h"

Camera& GetCamera()
{
	static Camera gCam;
	return gCam;
}

Camera::Camera()
{
	mPos	= D3DXVECTOR3(0.0f,0.0f,0.0f);
	mRight	= D3DXVECTOR3(1.0f,0.0f,0.0f);
	mUp		= D3DXVECTOR3(0.0f,1.0f,0.0f);
	mLook	= D3DXVECTOR3(0.0f,0.0f,1.0f);

	mEllRadius = D3DXVECTOR3(1.0f, 1.0f, 1.0f);
}

Camera::~Camera()
{
}

void Camera::SetLens(float Fovy, float Aspect, float MinZ, float MaxZ)
{
	D3DXMatrixPerspectiveFovLH(&mProj, Fovy, Aspect, MinZ, MaxZ);
}

void Camera::LookAt(const D3DXVECTOR3& Pos, const D3DXVECTOR3& Target)
{
	D3DXVECTOR3 L = Target-Pos;
	D3DXVec3Normalize(&L, &L);

	D3DXVECTOR3 R;
	D3DXVECTOR3 PositiveYAxis(0.0f,1.0f,0.0f);
	D3DXVec3Cross(&R, &PositiveYAxis, &L);
	D3DXVec3Normalize(&R, &R);

	D3DXVECTOR3 U;
	D3DXVec3Cross(&U, &L, &R);
	D3DXVec3Normalize(&U, &U);

	mPos	= Pos;
	mRight	= R;
	mUp		= U;
	mLook	= L;
}

D3DXVECTOR3& Camera::Pos()
{	return mPos;	}

D3DXVECTOR3& Camera::Right()
{	return mRight;	}

D3DXVECTOR3& Camera::Up()
{	return mUp;		}

D3DXVECTOR3& Camera::Look()
{	return mLook;	}

D3DXVECTOR3& Camera::RadiusVec()
{	return mEllRadius;	}


D3DXMATRIX&	Camera::ViewMatrix()
{	return mView;	}

D3DXMATRIX& Camera::ProjMatrix()
{	return mProj;	}