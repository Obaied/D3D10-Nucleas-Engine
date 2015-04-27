#include "CollisionDetection.h"

//Closest Point to ...
D3DXVECTOR3 ClosestPtOnPolygon(D3DXVECTOR3 A, D3DXVECTOR3 B, D3DXVECTOR3 C,
	D3DXVECTOR3 P)
{
   D3DXVECTOR3 Rab = ClosestPtOnLine(A, B, P);
   D3DXVECTOR3 Rbc = ClosestPtOnLine(B, C, P);
   D3DXVECTOR3 Rca = ClosestPtOnLine(C, A, P);
	
	D3DXVECTOR3 vDist0 = Rab - P;
	float SqDist0 = D3DXVec3LengthSq(&vDist0);
	D3DXVECTOR3 vDist1 = Rbc - P;
	float SqDist1 = D3DXVec3LengthSq(&vDist1);
	D3DXVECTOR3 vDist2 = Rca - P;
	float SqDist2 = D3DXVec3LengthSq(&vDist2);

	float Min = FLT_MAX;
	D3DXVECTOR3 pClosest;
	if(SqDist0 <= Min)
	{
		Min = SqDist0;
		pClosest = Rab;
	}
	if(SqDist1 <= Min)
	{
		Min = SqDist1;
		pClosest = Rbc;
	}
	if(SqDist2 <= Min)
	{
		Min = SqDist2;
		pClosest = Rca;
	}

	return pClosest;

}

D3DXVECTOR3 ClosestPtOnLine(D3DXVECTOR3 A, D3DXVECTOR3 B, D3DXVECTOR3 P)
{
   // Determine t (the length of the vector from ‘a’ to ‘p’)
   D3DXVECTOR3 VecToPoint = P - A;
   D3DXVECTOR3 SegmentVec = B - A;
   D3DXVECTOR3 v;
   D3DXVec3Normalize(&v, &SegmentVec);
   float d = D3DXVec3Length(&SegmentVec);

   float t = D3DXVec3Dot(&VecToPoint, &v);

   // Check to see if ‘t’ is beyond the extents of the line segment

   if (t < 0) return A;
   if (t > d) return B;
 
   // Return the point between ‘a’ and ‘b’

   D3DXVECTOR3 Dist = v;
   Dist *= t;
   return A + Dist;
}

D3DXVECTOR3 ClosestPtToAABB(D3DXVECTOR3 p, AABB Box)
{
	D3DXVECTOR3 q = p;
	if(p.x <= Box.Min.x) q.x = Box.Min.x;
	if(p.y <= Box.Min.y) q.y = Box.Min.y;
	if(p.z <= Box.Min.z) q.z = Box.Min.z;

	if(p.x >= Box.Max.x) q.x = Box.Max.x;
	if(p.y >= Box.Max.y) q.y = Box.Max.y;
	if(p.z >= Box.Max.z) q.z = Box.Max.z;

	return q;
}

//Intersection Tests
float IT_RayPlane(D3DXVECTOR3 RayPos, D3DXVECTOR3 RayDir, 
	D3DXVECTOR3 PlanePos, D3DXVECTOR3 PlaneNormal)
{
	//see explanation at page 176 of "Real-Time Collsion Detection"
	D3DXVec3Normalize(&RayDir, &RayDir);
	float d = D3DXVec3Dot(&PlaneNormal, &PlanePos);
	float t = (d-D3DXVec3Dot(&PlaneNormal,&RayPos) ) / (D3DXVec3Dot(&PlaneNormal,&RayDir) );
	return t;
}

bool IT_RayEllipsoid(D3DXVECTOR3 RayPos, D3DXVECTOR3 RayDir,
	D3DXVECTOR3 EllPos, D3DXVECTOR3 EllRadius, D3DXVECTOR3* DistToEll)
{
	D3DXVECTOR3 m = RayPos - EllPos;
	D3DXVec3Normalize(&RayDir, &RayDir);

    float a = ((RayDir.x * RayDir.x) / (EllRadius.x * EllRadius.x))
            + ((RayDir.y * RayDir.y) / (EllRadius.y * EllRadius.y))
            + ((RayDir.z * RayDir.z) / (EllRadius.z * EllRadius.z));
 
    float b = ((2.0f * m.x * RayDir.x) / (EllRadius.x * EllRadius.x))
            + ((2.0f * m.y * RayDir.y) / (EllRadius.y * EllRadius.y))
            + ((2.0f * m.z * RayDir.z) / (EllRadius.z * EllRadius.z));
 
    float c = ((m.x * m.x) / (EllRadius.x * EllRadius.x))
            + ((m.y * m.y) / (EllRadius.y * EllRadius.y))
            + ((m.z * m.z) / (EllRadius.z * EllRadius.z))
            - 1.0f;
 
    float d = ((b * b) - (4.0f * a * c));
 
    if (d < 0)
    {
        return false;
    }
    else
    {
        d = sqrt(d);
    }
 
    float hit = (-b + d) / (2.0f * a);
    float hitsecond = (-b - d) / (2.0f * a);
 
	float t;
    if (hit < hitsecond)
        t = hit;
    else
        t = hitsecond;

	D3DXVECTOR3 Q = RayPos + t*RayDir;
	*DistToEll = Q - RayPos;
	return true;
}

bool IT_AABBAABB(AABB BoxA, AABB BoxB)
{
	if(BoxA.Max.x < BoxB.Min.x || BoxA.Min.x > BoxB.Max.x) return false;
	if(BoxA.Max.y < BoxB.Min.y || BoxA.Min.y > BoxB.Max.y) return false;
	if(BoxA.Max.z < BoxB.Min.z || BoxA.Min.z > BoxB.Max.z) return false;
	return true;
}

bool SameSide(D3DXVECTOR3 A, D3DXVECTOR3 B, D3DXVECTOR3 P, D3DXVECTOR3 C)
{
	D3DXVECTOR3 BA = B-A;
	D3DXVECTOR3 PA = P-A;
	D3DXVECTOR3 CA = C-A;
	D3DXVec3Normalize(&BA, &BA);
	D3DXVec3Normalize(&PA, &PA);
	D3DXVec3Normalize(&CA, &CA);

	D3DXVECTOR3 cp0,cp1;
	D3DXVec3Cross(&cp0, &BA, &PA);
	D3DXVec3Cross(&cp1, &BA, &CA);

	if(D3DXVec3Dot(&cp0,&cp1) >= 0.0f)
		return true;

	return false;
}

bool IT_PointOnPolygon(D3DXVECTOR3 A, D3DXVECTOR3 B, D3DXVECTOR3 C, D3DXVECTOR3 P)
{
	if(SameSide(A,B, P,C) && 
		SameSide(A,C, P,B) &&
		SameSide(B,C, P,A) )
		return true;
	
	return false;
}

bool IT_PointAABB(D3DXVECTOR3 P, AABB PolyBox)
{
	//RFE: revise the whole function
	if(P.x < PolyBox.Min.x || P.x > PolyBox.Max.x 
		|| P.y < PolyBox.Min.y || P.y > PolyBox.Max.y
		|| P.z < PolyBox.Min.z || P.z > PolyBox.Max.z)
		return false;

	return true;
}

bool IT_PointLine2D(D3DXVECTOR2 A, D3DXVECTOR2 B, D3DXVECTOR2 P)
{
	D3DXVECTOR2 AB = B - A;
	D3DXVec2Normalize(&AB, &AB);
	
	D3DXVECTOR2 v = P - A;
	D3DXVec2Normalize(&v, &v);

	float t = D3DXVec2Dot(&AB, &v);
	//RFE: try playing with the Eps value until done reaching a satisfying tone
	float Eps = 0.01f;
	if(t >= 1.0f-Eps || t <= 1.0f+Eps)
		return true;
	
	return false;
}

bool IT_SphereAABB(const D3DXVECTOR3& SpherePos, float SphereRadius, 
	const AABB& Box)
{
	D3DXVECTOR3 ClosestPt = ClosestPtToAABB(SpherePos, Box);
	D3DXVECTOR3 v = ClosestPt - SpherePos;

	if(D3DXVec3Dot(&v,&v) <= SphereRadius*SphereRadius)
		return true;

	return false;
}

bool IT_EllipsoidAABB(const D3DXVECTOR3& EllPos, const D3DXVECTOR3& EllRadius,
	const AABB& Box)
{
	D3DXVECTOR3 ClosestPt = ClosestPtToAABB(EllPos, Box);
	D3DXVECTOR3 v = ClosestPt - EllPos;
	float r = D3DXVec3Length(&EllRadius);

	if(D3DXVec3Dot(&v,&v) <= r*r)
		return true;

	return false;
}

bool IT_SphereEllipsoid(const BS& Sphere, const D3DXVECTOR3& EllPos,
	const D3DXVECTOR3& EllRadius)
{
	D3DXVECTOR3 d = Sphere.Center - EllPos;
	float Distance2 = D3DXVec3Dot(&d,&d);

	float r0 = Sphere.Radius;
	float r1 = D3DXVec3Length(&EllRadius);
	float SumRadius = r0+r1;
	float SumRadius2 = SumRadius*SumRadius;
	if(Distance2 <= SumRadius2)
		return true;

	return false;
}

bool IT_SphereSphere(const BS& s0, const BS& s1)
{
	D3DXVECTOR3 d = s0.Center - s1.Center;
	float Distance2 = D3DXVec3Dot(&d,&d);

	float r0 = s0.Radius;
	float r1 = s1.Radius;
	float SumRadius = r0+r1;
	float SumRadius2 = SumRadius*SumRadius;
	if(Distance2 <= SumRadius2)
		return true;

	return false;
}

bool IT_RayBS(const D3DXVECTOR3& RayPos, const D3DXVECTOR3& RayDir, const BS& SphereBS)
{
	D3DXVECTOR3 Norm_RayDir;
	D3DXVec3Normalize(&Norm_RayDir, &RayDir);
	D3DXVECTOR3 m = RayPos - SphereBS.Center;
	float b = D3DXVec3Dot(&m, &Norm_RayDir);
	float c = D3DXVec3Dot(&m, &m) - SphereBS.Radius * SphereBS.Radius;

	//Exit if ray's origin is outside sphere (c > 0) and ray pointing out from sphere (b > 0)
	if(c > 0.0f && b > 0.0f) return false;

	float Discr = b*b - c;

	//A negative discriminant corresponds to ray missing sphere
	if(Discr < 0.0f) return false;

	//If we reached here, that means that the ray either intersects tangentially
	// or through-and-through, and either ways mean that it intersected so just return
	// true
	return true;
}