#ifndef CollisionDetection_H
#define CollisionDetection_H

#include "D3DUtil.h"

//Intersection Tests
//==================================================================
float IT_RayPlane(D3DXVECTOR3 RayPos, D3DXVECTOR3 RayDir, 
	D3DXVECTOR3 PlanePos, D3DXVECTOR3 PlaneNormal);

bool IT_RayEllipsoid(D3DXVECTOR3 RayPos, D3DXVECTOR3 RayDir,
	D3DXVECTOR3 EllPos, D3DXVECTOR3 EllRadius, D3DXVECTOR3* DistToEll);

bool IT_RayBS(const D3DXVECTOR3& RayPos, const D3DXVECTOR3& RayDir,
			  const BS& SphereBS);

bool IT_AABBAABB(AABB BoxA, AABB BoxB);

bool IT_PointAABB(D3DXVECTOR3 P, AABB PolyBox);

bool IT_PointLine2D(D3DXVECTOR2 A, D3DXVECTOR2 B, D3DXVECTOR2 P);

bool IT_SphereAABB(const D3DXVECTOR3& SpherePos, float SphereRadius, 
	const AABB& Box);

bool IT_EllipsoidAABB(const D3DXVECTOR3& EllPos, const D3DXVECTOR3& EllRadius,
	const AABB& Box);

bool IT_SphereEllipsoid(const BS& Sphere, const D3DXVECTOR3& EllPos,
	const D3DXVECTOR3& EllRadius);

bool IT_SphereSphere(const BS& s0, const BS& s1);

bool IT_PointOnPolygon(D3DXVECTOR3 A, D3DXVECTOR3 B, D3DXVECTOR3 C, D3DXVECTOR3 P);

//Misc
D3DXVECTOR3 ClosestPtOnPolygon(D3DXVECTOR3 A, D3DXVECTOR3 B, D3DXVECTOR3 C,
	D3DXVECTOR3 P);
D3DXVECTOR3 ClosestPtOnLine(D3DXVECTOR3 A, D3DXVECTOR3 B, D3DXVECTOR3 P);

#endif