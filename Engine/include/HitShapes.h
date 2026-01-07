#pragma once
#include <Windows.h>
#include <DirectXMath.h>

using namespace DirectX;

enum class HIT_SHAPE
{
	HS_QUAD,		// 2D 矩形
	HS_CIRCLE,		// 2D 円
	HS_TRIANGLE,	// 3D 三角形
	HS_AABB,		// 3D AABB
	HS_OBB,			// 3D OBB
	HS_SPHERE,		// 3D 球体
	HS_PILLAR,		// 3D 円柱
	HS_LINE,		// 3D 線
};

class HitAreaBase
{
protected:
	HIT_SHAPE	m_hitShape;					// 判定の形（計算式が変わる）
	UINT		m_hitType;					// 判定の種類
	int			m_hitPower;					// 判定パワー。弾の威力かな

public:
	virtual void SetAttackType(UINT area, int pow)	// 判定の属性とパワーを設定
	{
		m_hitType = area;
		m_hitPower = pow;
	}

	HIT_SHAPE GetHitShape()
	{
		return m_hitShape;
	}

	UINT GetHitType()						// 判定タイプの取得
	{
		return m_hitType;
	}

	int GetHitPower()						// 判定攻撃力の取得
	{
		return m_hitPower;
	}
};

class HitQuad : public HitAreaBase
{
private:
	XMFLOAT2		m_hitCenter;				// 判定中央座標
	XMFLOAT2		m_hitSize;					// 判定の大きさ
	XMFLOAT4		m_hitRect;					// 判定の矩形（計算用）

	void UpdateHitRect();						// 判定用矩形エリアの更新 中心座標、またはサイズの変更があると再計算

public:
	HitQuad ()
	{
		m_hitShape = HIT_SHAPE::HS_QUAD;
	}

	void SetHitSize(float width, float height)	// 判定サイズ設定
	{
		m_hitSize.x = width;
		m_hitSize.y = height;
		UpdateHitRect();						// 判定エリア更新
	}

	void SetPosition(float x, float y)			// 判定中央座標設定
	{
		m_hitCenter.x = x;
		m_hitCenter.y = y;
		UpdateHitRect();						// 判定エリア更新
	}

	XMFLOAT4 GetHitRect()						// 判定領域の取得
	{
		return m_hitRect;
	}
};

// 円
class HitCircle : public HitAreaBase
{
private:
	float radius;							// 半径
	XMFLOAT2 center;						// 中点
	float doubledR;							// 半径の2乗

public:
	HitCircle()
	{
		m_hitShape = HIT_SHAPE::HS_CIRCLE;
	}

	void SetCenter(float x, float y)		// 中点設定
	{
		center.x = x;
		center.y = y;
	}

	void SetRadius(float r)					// 半径設定
	{
		radius = r;
		doubledR = r * r;					// 平方根は毎回計算させない
	}

	float GetRadius()						// 半径取得　実は円同士の判定計算では不要
	{
		return radius;
	}

	float GetDoubledRadius()				// 判定計算で必要なのはこっち
	{
		return doubledR;
	}

	XMFLOAT2 GetCenterPosition()
	{
		return center;
	}
};

// 三角形
class HitTriangle : public HitAreaBase
{
private:
	XMFLOAT3	m_vertexes[3];
	XMVECTOR	m_normal;

	XMVECTOR	m_vectAB;
	XMVECTOR	m_vectAC;

	// float		m_dist;

public:
	HitTriangle()
	{
		m_hitShape = HIT_SHAPE::HS_TRIANGLE;
	}

	void SetTriangle(const XMFLOAT3& a, const XMFLOAT3& b, const XMFLOAT3& c);

	XMFLOAT3* GetVertex()
	{
		return m_vertexes;
	}

	XMVECTOR GetNormal()
	{
		return m_normal;
	}

	XMVECTOR GetVectAB()
	{
		return m_vectAB;
	}

	XMVECTOR GetVectAC()
	{
		return m_vectAC;
	}
};

// Axis Aligbed bounding box（軸並行バウンディングボックス）
class HitAABB : public HitAreaBase
{
private:
	XMFLOAT3 m_min;
	XMFLOAT3 m_max;

	XMFLOAT3 m_center;
	XMFLOAT3 m_halfSize;

	void updateAABB();

public:
	HitAABB()
	{
		m_hitShape = HIT_SHAPE::HS_AABB;
	}

	void SetAABB(const XMFLOAT3& center, const XMFLOAT3& size);
	void SetAABBMinMax(const XMFLOAT3& min, const XMFLOAT3& max);
	void SetCenter(const XMFLOAT3& center);
	void SetSize(const XMFLOAT3& size);

	XMFLOAT3 GetMin()
	{
		return m_min;
	}

	XMFLOAT3 GetMax()
	{
		return m_max;
	}

	XMFLOAT3 GetCenter()
	{
		return m_center;
	}

	XMFLOAT3 GetHalfSize()
	{
		return m_halfSize;
	}
};

// OBB
class HitOBB : public HitAreaBase
{
public:
	HitOBB()
	{
		m_hitShape = HIT_SHAPE::HS_OBB;
	}
};

class HitSphere : public HitAreaBase
{
private:
	XMFLOAT3	m_center;
	float		m_radius;
	float		m_doubleRadius;

public:
	HitSphere()
	{
		m_hitShape = HIT_SHAPE::HS_SPHERE;
	}

	void SetSphereMinMax(const XMFLOAT3& min, const XMFLOAT3& max);

	void SetCenter(float x, float y, float z)
	{
		m_center.x = x;
		m_center.y = y;
		m_center.z = z;
	}

	void SetRadius(float r)
	{
		m_radius = r;
		m_doubleRadius = r * r;
	}

	XMFLOAT3 GetCenter()
	{
		return m_center;
	}

	float GetRadius()
	{
		return m_radius;
	}

	float GetDoubleRadius()
	{
		return m_doubleRadius;
	}
};

// 円柱
class HitPillar : public HitAreaBase
{
private:
	float m_height;		// 高さ
	XMFLOAT3 m_center;	// 中心座標
	HitCircle m_circle;	// 円判定（XZ座標＋半径）

	float m_top;		// 天辺Y
	float m_btm;		// 底辺Y

	void UpdatePillar()
	{
		// 中心座標と高さから天辺と底辺を計算
		m_btm = m_center.y - m_height * 0.5f;
		m_top = m_center.y + m_height * 0.5f;
	}

public:
	HitPillar()
	{
		m_hitShape = HIT_SHAPE::HS_PILLAR;
	}


	// 高さ設定
	void SetHeight(float height)
	{
		m_height = height;
		UpdatePillar();
	}

	// 中心点設定
	void SetCenter(float x, float y, float z)
	{
		m_center.x = x;
		m_center.y = y;
		m_center.z = z;

		m_circle.SetCenter(x, z);	// Circleは２次元でXZ平面
		UpdatePillar();
	}

	// 半径設定
	void SetRadius(float radius)
	{
		m_circle.SetRadius(radius);
	}

	// 円判定取得
	HitCircle* GetCircle()
	{
		return &m_circle;
	}

	// 底辺Y取得
	float GetBottom()
	{
		return m_btm;
	}

	// 天辺Y取得
	float GetTop()
	{
		return m_top;
	}

	// 中心点Y取得
	float GetCenter()
	{
		return m_center.y;
	}
};

// 線
class HitRayLine : public HitAreaBase
{
private:
	XMFLOAT3	m_start;	// 線分のスタート位置
	XMFLOAT3	m_end;		// 線分の終了位置
	XMVECTOR	m_normal;
	float		m_length;
	float		m_radius;	// 断面の半径

public:
	HitRayLine()
	{
		m_hitShape = HIT_SHAPE::HS_LINE;
	}

	void SetLine(const XMFLOAT3& start, const XMFLOAT3& end, float radius);

	// 線分のスタート位置取得
	XMFLOAT3 GetStart()
	{
		return m_start;
	}

	// 線分の終了位置取得
	XMFLOAT3 GetEnd()
	{
		return m_end;
	}

	XMVECTOR GetNormal()
	{
		return m_normal;
	}

	// 線分の長さ取得
	float GetLength()
	{
		return m_length;
	}

	// 線分の半径取得
	float GetSize()
	{
		return m_radius;
	}

	// XMMATRIXを掛けたレイを作るメソッド
	static void GetTransformedLine(HitRayLine& ray, XMMATRIX& mat);
};