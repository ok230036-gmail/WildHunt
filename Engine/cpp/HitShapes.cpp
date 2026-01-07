#include "HitShapes.h"

void HitQuad::UpdateHitRect()				// 判定用矩形エリアの更新 中心座標、またはサイズの変更があると再計算
{
	float hw = m_hitSize.x * 0.5f;			// 幅の半分
	float hh = m_hitSize.y * 0.5f;			// 高さの半分

	m_hitRect.x = m_hitCenter.x - hw;		// 左下X
	m_hitRect.y = m_hitCenter.y - hh;		// 左下Y
	m_hitRect.z = m_hitCenter.x + hw;		// 右上X
	m_hitRect.w = m_hitCenter.y + hh;		// 右上Y
}

void HitAABB::updateAABB()
{
	m_min.x = m_center.x - m_halfSize.x;
	m_min.y = m_center.y - m_halfSize.y;
	m_min.z = m_center.z - m_halfSize.z;

	m_max.x = m_center.x + m_halfSize.x;
	m_max.y = m_center.y + m_halfSize.y;
	m_max.z = m_center.z + m_halfSize.z;
}

void HitAABB::SetAABB(const XMFLOAT3& center, const XMFLOAT3& size)
{
	m_center = center;
	m_halfSize.x = size.x * 0.5f;
	m_halfSize.y = size.y * 0.5f;
	m_halfSize.z = size.z * 0.5f;
	updateAABB();
}

void HitAABB::SetAABBMinMax(const XMFLOAT3& min, const XMFLOAT3& max)
{
	m_min = min;
	m_max = max;

	m_halfSize.x = (max.x - min.x) * 0.5f;
	m_halfSize.y = (max.y - min.y) * 0.5f;
	m_halfSize.z = (max.z - min.z) * 0.5f;

	m_center.x = min.x + m_halfSize.x;
	m_center.y = min.y + m_halfSize.y;
	m_center.z = min.z + m_halfSize.z;
}

void HitAABB::SetCenter(const XMFLOAT3& center)
{
	m_center = center;
	updateAABB();
}

void HitAABB::SetSize(const XMFLOAT3& size)
{
	m_halfSize.x = size.x * 0.5f;
	m_halfSize.y = size.y * 0.5f;
	m_halfSize.z = size.z * 0.5f;
	updateAABB();
}

void HitTriangle::SetTriangle(const XMFLOAT3& a, const XMFLOAT3& b, const XMFLOAT3& c)
{
	m_vertexes[0] = a;
	m_vertexes[1] = b;
	m_vertexes[2] = c;

	XMFLOAT3 ab = {b.x - a.x, b.y - a.y, b.z - a.z};
	XMFLOAT3 ac = {c.x - a.x, c.y - a.y, c.z - a.z};

	m_vectAB = XMLoadFloat3(&ab);
	m_vectAC = XMLoadFloat3(&ac);

	XMVECTOR nml = XMVector3Cross(m_vectAB, m_vectAC);
	m_normal = XMVector3Normalize(nml);

	XMVECTOR dot = XMVector3Dot(XMLoadFloat3(&a), m_normal);
}

void HitSphere::SetSphereMinMax(const XMFLOAT3& min, const XMFLOAT3& max)
{
	float dist_x = max.x - min.x;
	float dist_y = max.y - min.y;
	float dist_z = max.z - min.z;

	m_doubleRadius = dist_x * dist_x + dist_y * dist_y + dist_z * dist_z;
	m_radius = sqrtf(m_doubleRadius);

	m_center.x = min.x + m_radius;
	m_center.y = min.y + m_radius;
	m_center.z = min.z + m_radius;
}

void HitRayLine::SetLine(const XMFLOAT3& start, const XMFLOAT3& end, float radius)
{
	m_start = start;
	m_end = end;
	m_radius = radius;

	float dist_x = end.x - start.x;
	float dist_y = end.y - start.y;
	float dist_z = end.z - start.z;

	m_length = sqrtf(dist_x * dist_x + dist_y * dist_y + dist_z * dist_z);

	XMFLOAT3 vec = { dist_x / m_length, dist_y / m_length, dist_z / m_length};
	m_normal = XMLoadFloat3(&vec);
}

// XMMATRIXを掛けたレイを作るメソッドを追加。
void HitRayLine::GetTransformedLine(HitRayLine& ray, XMMATRIX& mat)
{
	XMVECTOR stvect = XMLoadFloat3(&ray.m_start);
	XMVECTOR edvect = XMLoadFloat3(&ray.m_end);

	stvect = XMVector3Transform(stvect, mat);
	edvect = XMVector3Transform(edvect, mat);

	ray.SetLine(XMFLOAT3(XMVectorGetX(stvect), XMVectorGetY(stvect), XMVectorGetZ(stvect)), 
		XMFLOAT3(XMVectorGetX(edvect), XMVectorGetY(edvect), XMVectorGetZ(edvect)),
		ray.GetSize());
}