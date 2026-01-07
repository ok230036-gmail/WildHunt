#include <list>
#include <algorithm>		// for_eachのため

#include "HitManager.h"

// 一応全ての組み合わせ分のヒット判定関数を宣言、使わないのは実装しない(falseを返す)
bool isHitVS(HitQuad* atk, HitQuad* def, XMFLOAT3& resHitPos);						// 矩形 VS 矩形
bool isHitVS(HitQuad* atk, HitCircle* def, XMFLOAT3& resHitPos, bool revCheck);		// 矩形 VS 円
bool isHitVS(HitQuad* atk, HitTriangle* def, XMFLOAT3& resHitPos, bool revCheck);	// 矩形 VS 三角
bool isHitVS(HitQuad* atk, HitAABB* def, XMFLOAT3& resHitPos, bool revCheck);		// 矩形 VS AABB
bool isHitVS(HitQuad* atk, HitOBB* def, XMFLOAT3& resHitPos, bool revCheck);		// 矩形 VS OBB
bool isHitVS(HitQuad* atk, HitSphere* def, XMFLOAT3& resHitPos, bool revCheck);		// 矩形 VS 球
bool isHitVS(HitQuad* atk, HitPillar* def, XMFLOAT3& resHitPos, bool revCheck);		// 矩形 VS 円柱
bool isHitVS(HitQuad* atk, HitRayLine* def, XMFLOAT3& resHitPos, bool revCheck);	// 矩形 VS 線分

bool isHitVS(HitCircle* atk, HitCircle* def, XMFLOAT3& resHitPos);					// 円 VS 円
bool isHitVS(HitCircle* atk, HitTriangle* def, XMFLOAT3& resHitPos, bool revCheck);	// 円 VS 三角
bool isHitVS(HitCircle* atk, HitAABB* def, XMFLOAT3& resHitPos, bool revCheck);		// 円 VS AABB
bool isHitVS(HitCircle* atk, HitOBB* def, XMFLOAT3& resHitPos, bool revCheck);		// 円 VS OBB
bool isHitVS(HitCircle* atk, HitSphere* def, XMFLOAT3& resHitPos, bool revCheck);	// 円 VS 球
bool isHitVS(HitCircle* atk, HitPillar* def, XMFLOAT3& resHitPos, bool revCheck);	// 円 VS 円柱
bool isHitVS(HitCircle* atk, HitRayLine* def, XMFLOAT3& resHitPos, bool revCheck);	// 円 VS 線分

bool isHitVS(HitTriangle* atk, HitTriangle* def, XMFLOAT3& resHitPos);				// 三角 VS 三角
bool isHitVS(HitTriangle* atk, HitAABB* def, XMFLOAT3& resHitPos, bool revCheck);	// 三角 VS AABB
bool isHitVS(HitTriangle* atk, HitOBB* def, XMFLOAT3& resHitPos, bool revCheck);	// 三角 VS OBB
bool isHitVS(HitTriangle* atk, HitSphere* def, XMFLOAT3& resHitPos, bool revCheck);	// 三角 VS 球
bool isHitVS(HitTriangle* atk, HitPillar* def, XMFLOAT3& resHitPos, bool revCheck);	// 三角 VS 円柱
bool isHitVS(HitTriangle* atk, HitRayLine* def, XMFLOAT3& resHitPos);				// 三角 VS 線分

bool isHitVS(HitAABB* atk, HitAABB* def, XMFLOAT3& resHitPos);						// AABB VS AABB
bool isHitVS(HitAABB* atk, HitOBB* def, XMFLOAT3& resHitPos, bool revCheck);		// AABB VS OBB
bool isHitVS(HitAABB* atk, HitSphere* def, XMFLOAT3& resHitPos, bool revCheck);		// AABB VS 球
bool isHitVS(HitAABB* atk, HitPillar* def, XMFLOAT3& resHitPos, bool revCheck);		// AABB VS 円柱
bool isHitVS(HitAABB* atk, HitRayLine* def, XMFLOAT3& resHitPos, bool revCheck);	// AABB VS 線分

bool isHitVS(HitOBB* atk, HitOBB* def, XMFLOAT3& resHitPos);						// OBB VS OBB
bool isHitVS(HitOBB* atk, HitSphere* def, XMFLOAT3& resHitPos, bool revCheck);		// OBB VS 球
bool isHitVS(HitOBB* atk, HitPillar* def, XMFLOAT3& resHitPos, bool revCheck);		// OBB VS 円柱
bool isHitVS(HitOBB* atk, HitRayLine* def, XMFLOAT3& resHitPos, bool revCheck);		// OBB VS 線分

bool isHitVS(HitSphere* atk, HitSphere* def, XMFLOAT3& resHitPos);					// 球 VS 球
bool isHitVS(HitSphere* atk, HitPillar* def, XMFLOAT3& resHitPos, bool revCheck);	// 球 VS 円柱
bool isHitVS(HitSphere* atk, HitRayLine* def, XMFLOAT3& resHitPos, bool revCheck);	// 球 VS 線分

bool isHitVS(HitPillar* atk, HitPillar* def, XMFLOAT3& resHitPos);					// 円柱 VS 円柱
bool isHitVS(HitPillar* atk, HitRayLine* def, XMFLOAT3& resHitPos, bool revCheck);	// 円柱 VS 線分

bool isHitVS(HitRayLine* atk, HitRayLine* def, XMFLOAT3& resHitPos);				// 線分 VS 線分

bool getVectorT(const float max, const float min, const float nml, const float pos, float& sT, float& eT);

// 矩形と矩形
bool isHitVS(HitQuad* atk, HitQuad* def, XMFLOAT3& resHitPos)
{
	XMFLOAT4 myRect = atk->GetHitRect();			// このHitStructure(atk)が持つ判定領域を取得
	XMFLOAT4 targetRect = def->GetHitRect();		// 相手側のHitStructure(def)が持つ判定領域を取得

	// X軸上に投影した線分が重なって、Y軸上に投影した線分が重なったら当たってる
	// X軸チェック
	if (targetRect.x > myRect.z || targetRect.z < myRect.x)
	{
		return false;	// X軸が重なっていない
	}

	// Y軸チェック
	if (targetRect.y > myRect.w || targetRect.w < myRect.y)
	{
		return false;	// Y軸が重なっていない
	}

	// x check
	XMFLOAT2 atkCenter = { (myRect.z - myRect.x) * 0.5f, (myRect.w - myRect.y) * 0.5f};
	if (atkCenter.x < targetRect.x)
	{
		resHitPos.x = targetRect.x;
	}
	else if (atkCenter.x > targetRect.z)
	{
		resHitPos.x = targetRect.z;
	}
	else
	{
		resHitPos.x = atkCenter.x;
	}

	// y check
	if (atkCenter.y < targetRect.y)
	{
		resHitPos.y = targetRect.y;
	}
	else if (atkCenter.y > targetRect.w)
	{
		resHitPos.y = targetRect.w;
	}
	else
	{
		resHitPos.y = atkCenter.y;
	}

	return true;
}

// 矩形と円
bool isHitVS(HitQuad* atk, HitCircle* def, XMFLOAT3& resHitPos, bool revCheck)
{
	XMFLOAT4 atkRect = atk->GetHitRect();
	XMFLOAT2 defCent = def->GetCenterPosition();

	float r = def->GetRadius();	// 半径

	// 矩形の四点を半径分伸ばす、まずは縦長判定
	if ( (atkRect.w + r < defCent.y || atkRect.y - r > defCent.y) || (atkRect.z < defCent.x || atkRect.x > defCent.x) )
	{
		// 縦長矩形とは重なっていない
		// 横長判定
		if ( (atkRect.z + r < defCent.x || atkRect.x - r > defCent.x) || (atkRect.w < defCent.y || atkRect.y > defCent.y) )
		{
			// 横長判定とも重なっていないが、角の半径円上で重なっているかもしれない
			// 角判定＊４ どれかに入るのならヒットしている
			float x_dist, y_dist;
			float dR = def->GetDoubledRadius();
			float totalDist = 0.0f;

			for (int i = 0; i < 4; i++)
			{
				switch (i)
				{
				case 0:
					// Left Bottom
					x_dist = atkRect.x - defCent.x;
					y_dist = atkRect.y - defCent.y;
					break;

				case 1:
					// Right Bottom
					x_dist = atkRect.z - defCent.x;
					y_dist = atkRect.y - defCent.y;
					break;

				case 2:
					// Left Top
					x_dist = atkRect.x - defCent.x;
					y_dist = atkRect.w - defCent.y;
					break;

				default:
					// Right Top
					x_dist = atkRect.z - defCent.x;
					y_dist = atkRect.w - defCent.y;
					break;
				}

				totalDist = x_dist * x_dist + y_dist * y_dist;
				if (totalDist <= dR)
				{
					// 角に円が接触している
					// 当たった場所を保持
					switch (i)
					{
					case 0:	// lb
						resHitPos.x = atkRect.x;
						resHitPos.x = atkRect.y;
						break;

					case 1:	// rb
						resHitPos.x = atkRect.z;
						resHitPos.y = atkRect.y;
						break;

					case 2:	// lt
						resHitPos.x = atkRect.x;
						resHitPos.y = atkRect.w;
						break;

					default:// rt
						resHitPos.x = atkRect.z;
						resHitPos.y = atkRect.w;
						break;
					}

					resHitPos.z = 0.0f;
					return true;
				}
			}

			// 全ての判定で外れていた
			return false;
		}

	}

	// 横長矩形、縦長矩形、または中央の矩形に接触していた
	// 中心点と矩形の関係から矩形側の接触点を特定
	if (defCent.x < atkRect.x)
	{
		resHitPos.x = atkRect.x;
	}
	else if (defCent.x > atkRect.z)
	{
		resHitPos.x = atkRect.z;
	}
	else
	{
		resHitPos.x = defCent.x;
	}

	if (defCent.y < atkRect.y)
	{
		resHitPos.y = atkRect.y;
	}
	else if (defCent.y > atkRect.w)
	{
		resHitPos.y = atkRect.w;
	}
	else
	{
		resHitPos.y = defCent.y;
	}

	resHitPos.z = 0.0f;
	return true;
}

bool isHitVS(HitQuad* atk, HitTriangle* def, XMFLOAT3& resHitPos, bool revCheck)
{
	return false;
}

bool isHitVS(HitQuad* atk, HitAABB* def, XMFLOAT3& resHitPos, bool revCheck)
{
	return false;
}

bool isHitVS(HitQuad* atk, HitOBB* def, XMFLOAT3& resHitPos, bool revCheck)
{
	return false;
}

bool isHitVS(HitQuad* atk, HitSphere* def, XMFLOAT3& resHitPos, bool revCheck)
{
	return false;
}

bool isHitVS(HitQuad* atk, HitPillar* def, XMFLOAT3& resHitPos, bool revCheck)
{
	return false;
}

bool isHitVS(HitQuad* atk, HitRayLine* def, XMFLOAT3& resHitPos, bool revCheck)
{
	return false;
}

// 円と円
bool isHitVS(HitCircle* atk, HitCircle* def, XMFLOAT3& resHitPos)
{
	XMFLOAT2 centerA = atk->GetCenterPosition();
	XMFLOAT2 centerB = def->GetCenterPosition();
	
	float xdist = centerB.x - centerA.x;	// X成分
	float ydist = centerB.y - centerA.y;	// Y成分
	
	// ベクトルの長さと半径の合計値比較、平方根計算をしないように二乗のまま比較
	float length = (xdist * xdist + ydist * ydist);
	if ((atk->GetDoubledRadius() + def->GetDoubledRadius() + 2 * atk->GetRadius() * def->GetRadius()) < length)
	{
		// 重なっていない
		return false;
	}
	
	// 平方根
	length = sqrtf(length);
	XMFLOAT2 nmlVect = {xdist / length, ydist / length};

	if (length < atk->GetRadius())
	{
		// 接触点はatk上の点にする。
		resHitPos.x = centerA.x + atk->GetRadius() * nmlVect.x;
		resHitPos.y = centerA.y + atk->GetRadius() * nmlVect.y;
	}
	else
	{
		// def側の円周が接点
		resHitPos.x = centerB.x - def->GetRadius() * nmlVect.x;
		resHitPos.y = centerB.y - def->GetRadius() * nmlVect.y;
	}

	resHitPos.z = 0.0f;

	return true;		// 重なっている		
}

bool isHitVS(HitCircle* atk, HitTriangle* def, XMFLOAT3& resHitPos, bool revCheck)
{
	return false;
}

bool isHitVS(HitCircle* atk, HitAABB* def, XMFLOAT3& resHitPos, bool revCheck)
{
	return false;
}

bool isHitVS(HitCircle* atk, HitOBB* def, XMFLOAT3& resHitPos, bool revCheck)
{
	return false;
}

bool isHitVS(HitCircle* atk, HitSphere* def, XMFLOAT3& resHitPos, bool revCheck)
{
	return false;
}

bool isHitVS(HitCircle* atk, HitPillar* def, XMFLOAT3& resHitPos, bool revCheck)
{
	return false;
}

// 円と線分
bool isHitVS(HitCircle* atk, HitRayLine* def, XMFLOAT3& resHitPos, bool revCheck)
{
	// 上から見た図でチェック
	// 円の中心点から、線分に向けて垂線を引いてチェック
	float t = ((atk->GetCenterPosition().x - def->GetStart().x) * (def->GetEnd().x - def->GetStart().x) +
		(atk->GetCenterPosition().y - def->GetStart().z) * (def->GetEnd().z - def->GetStart().z)) /
		((def->GetEnd().x - def->GetStart().x) * (def->GetEnd().x - def->GetStart().x) +
		(def->GetEnd().z - def->GetStart().z) * (def->GetEnd().z - def->GetStart().z));
	
	if (t < 0 || t > 1)
		return false;

	float pcx = (def->GetEnd().x - def->GetStart().x) * t + def->GetStart().x;
	float pcz = (def->GetEnd().z - def->GetStart().z) * t + def->GetStart().z;

	if (((atk->GetCenterPosition().x - pcx) * (atk->GetCenterPosition().x - pcx) + 
		(atk->GetCenterPosition().y - pcz) * (atk->GetCenterPosition().y - pcz))
		>= ((atk->GetRadius() + def->GetSize()) * (atk->GetRadius() + def->GetSize())))
		return false;


	return true;
}

bool isHitVS(HitTriangle* atk, HitTriangle* def, XMFLOAT3& resHitPos)
{
	return false;
}

bool isHitVS(HitTriangle* atk, HitAABB* def, XMFLOAT3& resHitPos, bool revCheck)
{
	return false;
}

bool isHitVS(HitTriangle* atk, HitOBB* def, XMFLOAT3& resHitPos, bool revCheck)
{
	return false;
}

bool isHitVS(HitTriangle* atk,  HitSphere* def, XMFLOAT3& resHitPos, bool revCheck)
{
	return false;
}

bool isHitVS(HitTriangle* atk,  HitPillar* def, XMFLOAT3& resHitPos, bool revCheck)
{
	return false;
}

bool getVectorT(const float max, const float min, const float nml, const float pos, float& sT, float& eT)
{
	// 方向で接触面の順番が変わる
	if (nml < 0.0f)
	{
		// マイナス方向
		sT = (max - pos) / nml;	// 方向ベクトルで割る
		eT = (min - pos) / nml;
	}
	else if (nml > 0.0f)
	{
		// プラス方向 MINが先
		sT = (min - pos) / nml;	// 方向ベクトルで割る
		eT = (max - pos) / nml;
	}
	else
	{
		// 傾きなし
		if (pos < min || pos > max)
		{
			// 範囲に入らない
			return false;
		}
		else
		{
			sT = 0.0f;
			eT = FLT_MAX;
		}
	}

	// 交差開始点０未満はベクトルの逆方向になる（始点が既にAABBに埋まっているので０）
	if (sT < 0.0f) sT = 0.0f;

	// eTがマイナスなのはベクトルが完全に反対方向、つまり軸要素が全部外。
	if (eT < 0.0f) return false;

	return true;
}

#define TRIANGLE_CHECK_MODE (1)
// Triangle Ray

#if TRIANGLE_CHECK_MODE == 0
bool isHitVS(HitTriangle* atk, HitRayLine* def, XMFLOAT3& resHitPos)
{
	return false;
}

#else

// 3*3正行列
float det(XMFLOAT3& a, XMFLOAT3& b, XMFLOAT3& c)
{
	float res = a.x * b.y * c.z + a.z * b.x * c.y + a.y * b.z * c.x;
	res -= a.z * b.y * c.x + a.y * b.x * c.z + a.x * b.z * c.y;
	return res;
}

// 三角と線分
bool isHitVS(HitTriangle* atk, HitRayLine* def, XMFLOAT3& resHitPos)
{
	XMVECTOR vectAB = atk->GetVectAB();
	XMVECTOR vectAC = atk->GetVectAC();

	XMFLOAT3 rayStart = def->GetStart();
	XMFLOAT3 rayEnd = def->GetEnd();

	// ベクトルを取り出し易いようにXMFLOAT3に
	XMFLOAT3 flA = { XMVectorGetX(vectAB), XMVectorGetY(vectAB), XMVectorGetZ(vectAB) };
	XMFLOAT3 flB = { XMVectorGetX(vectAC), XMVectorGetY(vectAC), XMVectorGetZ(vectAC) };

	// ベクトルCは -rayなので方向を逆にしておく
	XMFLOAT3 flC = { rayStart.x - rayEnd.x, rayStart.y - rayEnd.y, rayStart.z - rayEnd.z };

	// 分母
	float detABC = det(flA, flB, flC);

	if (detABC < 0.0001f && detABC > -0.0001f)
	{
		// 分母がほぼゼロってことは、Rayと三角形がほぼ直行
		return false;
	}

	detABC = 1.0f / detABC;	// 先に逆数にしておく事で割り算の回数を減らす

	// 三角形ABCの頂点Aの座標
	XMFLOAT3 vtxA = atk->GetVertex()[0];

	// ベクトルDはSt - A
	XMFLOAT3 flD = { rayStart.x - vtxA.x, rayStart.y - vtxA.y, rayStart.z - vtxA.z };

	float u, v, t;
	u = det(flD, flB, flC) * detABC;
	v = det(flA, flD, flC) * detABC;
	t = det(flA, flB, flD) * detABC;

	if (u > 1.0f || u < 0.0f) return false;
	if (v > 1.0f || v < 0.0f) return false;
	if (t > 1.0f || t < 0.0f) return false;

	if ((u + v) > 1.0f) return false;

	// flCはRayベクトルなんだけど方向が逆なので引く
	resHitPos.x = rayStart.x - flC.x * t;
	resHitPos.y = rayStart.y - flC.y * t;
	resHitPos.z = rayStart.z - flC.z * t;

	return true;
}
#endif

// AABBとAABB
bool isHitVS(HitAABB* atk,  HitAABB* def, XMFLOAT3& resHitPos)
{
	XMFLOAT3 atkMin, atkMax;
	XMFLOAT3 defMin, defMax;
	
	atkMin = atk->GetMin();
	atkMax = atk->GetMax();
	defMin = def->GetMin();
	defMax = def->GetMax();

	// Quadの３次元版
	if (atkMax.x < defMin.x || atkMax.y < defMin.y || atkMax.z < defMin.z ||
		defMax.x < atkMin.x || defMax.y < atkMin.y || defMax.z < atkMin.z)
		return false;

	// x check
	XMFLOAT3 atkCenter = atk->GetCenter();
	if (atkCenter.x < defMin.x)
	{
		resHitPos.x = defMin.x;
	}
	else if (atkCenter.x > defMax.x)
	{
		resHitPos.x = defMax.x;
	}
	else
	{
		resHitPos.x = atkCenter.x;
	}

	// y check
	if (atkCenter.y < defMin.y)
	{
		resHitPos.y = defMin.y;
	}
	else if (atkCenter.y > defMax.y)
	{
		resHitPos.y = defMax.y;
	}
	else
	{
		resHitPos.y = atkCenter.y;
	}

	// z check
	if (atkCenter.z < defMin.z)
	{
		resHitPos.z = defMin.z;
	}
	else if (atkCenter.z > defMax.z)
	{
		resHitPos.z = defMax.z;
	}
	else
	{
		resHitPos.z = atkCenter.z;
	}


	return true;
}

bool isHitVS( HitAABB* atk,  HitOBB* def, XMFLOAT3& resHitPos, bool revCheck)
{
	return false;
}

// AABBと球
bool isHitVS( HitAABB* atk,  HitSphere* def, XMFLOAT3& resHitPos, bool revCheck)
{
	// 各軸上の最小値より小さい、または最大値より大きい場合、AABBから中点が離れている

	XMFLOAT3	atkMin = atk->GetMin();
	XMFLOAT3	atkMax = atk->GetMax();

	XMFLOAT3	center = def->GetCenter();

	XMFLOAT3	vect = {};						// 接点へのベクトル

	float		dR = def->GetDoubleRadius();	// また距離なので２乗値にしておく

	float		len = 0.0f;						// 合計距離値
	float		dist = 0.0f;					// 計算用各軸の距離

	// X軸
	if (center.x < atkMin.x)
	{
		dist = atkMin.x - center.x;
	}
	else if (center.x > atkMax.x)
	{
		dist = atkMax.x - center.x;
	}

	len += dist * dist;					// X軸上の距離を加算。２乗にする事を忘れないように
	vect.x = dist;
	dist = 0.0f;

	// Y軸
	if (center.y < atkMin.y)
	{
		dist = atkMin.y - center.y;
	}
	else if (center.y > atkMax.y)
	{
		dist = atkMax.y - center.y;
	}

	len += dist * dist;					// Y軸上の距離を加算。
	vect.y = dist;
	dist = 0.0f;

	// Z軸
	if (center.z < atkMin.z)
	{
		dist = atkMin.z - center.z;
	}
	else if (center.z > atkMax.z)
	{
		dist = atkMax.z - center.z;
	}

	len += dist * dist;					// Z軸上の距離を加算。
	vect.z = dist;

	// 判定だけだから、平方根計算はしない。
	if (len > dR) return false;

	// 接触点算出
	resHitPos.x = center.x + vect.x;
	resHitPos.y = center.y + vect.y;
	resHitPos.z = center.z + vect.z;

	return true;
}

bool isHitVS( HitAABB* atk,  HitPillar* def, XMFLOAT3& resHitPos, bool revCheck)
{
	return false;
}

// AABBと線分
bool isHitVS(HitAABB* atk, HitRayLine* def, XMFLOAT3& resHitPos, bool revCheck)
{
	// AABBの範囲と線分の値が交差しているか判定

	XMFLOAT3 min = atk->GetMin();
	XMFLOAT3 max = atk->GetMax();

	XMFLOAT3 startP = def->GetStart();
	XMVECTOR lineNormal = def->GetNormal();
	float length = def->GetLength();
	
	// まずX軸を基に始点からの衝突座標点の線上の割合(sT)、突破座標点の線上の割合(eT)を算出していく
	float minEnd = FLT_MAX, maxStart = FLT_MIN;
	float sT, eT;

	for (int i = 0; i < 3; i++)
	{
		float max_v, min_v, nml_v, pos_v;

		switch (i)
		{
		case 0:
			max_v = max.x;
			min_v = min.x;
			nml_v = XMVectorGetX(lineNormal);
			pos_v = startP.x;
			break;

		case 1:
			max_v = max.y;
			min_v = min.y;
			nml_v = XMVectorGetY(lineNormal);
			pos_v = startP.y;
			break;

		default:
			max_v = max.z;
			min_v = min.z;
			nml_v = XMVectorGetZ(lineNormal);
			pos_v = startP.z;
			break;
		}

		if (!getVectorT(max_v, min_v, nml_v, pos_v, sT, eT)) return false;

		// 最も大きい接触点
		if (maxStart < sT) maxStart = sT;

		// 最も小さい突破点
		if (minEnd > eT) minEnd = eT;

		// 真っ平らの面があるか
		if (maxStart > minEnd) return false;

		// 突破点が接触点より大きくなったら領域が交差していない事になる
	}

	if (length < maxStart)
		return false;	// ベクトルの長さを越えてしまっているのであたっていない。

	// 交差点判明
	resHitPos.x = startP.x + XMVectorGetX(lineNormal) * maxStart;
	resHitPos.y = startP.y + XMVectorGetY(lineNormal) * maxStart;
	resHitPos.z = startP.z + XMVectorGetZ(lineNormal) * maxStart;

	return true;
}

bool isHitVS( HitOBB* atk,  HitOBB* def, XMFLOAT3& resHitPos)
{
	return false;
}

bool isHitVS( HitOBB* atk,  HitSphere* def, XMFLOAT3& resHitPos, bool revCheck)
{
	return false;
}

bool isHitVS( HitOBB* atk,  HitPillar* def, XMFLOAT3& resHitPos, bool revCheck)
{
	return false;
}

bool isHitVS(HitOBB* atk, HitRayLine* def, XMFLOAT3& resHitPos, bool revCheck)
{
	return false;
}

// 球と球
bool isHitVS( HitSphere* atk,  HitSphere* def, XMFLOAT3& resHitPos)
{
	float atk_r = atk->GetRadius();
	float def_r = def->GetRadius();

	XMFLOAT3 aC = atk->GetCenter();
	XMFLOAT3 dC = def->GetCenter();

	float dR = atk->GetDoubleRadius() + def->GetDoubleRadius() + 2.0f * atk_r * def_r;

	float dist_x, dist_y, dist_z;
	dist_x = dC.x - aC.x;
	dist_y = dC.y - aC.y;
	dist_z = dC.z - aC.z;

	float length = dist_x * dist_x + dist_y * dist_y + dist_z * dist_z;
	if (dR < length) return false;

	// 接触点計算
	// 中点間ベクトルの長さを取得。あんまり平方根計算したくないんだけどね。
	length = sqrtf(length);

	// まず単位化した方向ベクトル
	XMFLOAT3 nmlVect = {dist_x / length, dist_y / length, dist_z / length};

	// 中点間ベクトル上にatkかdefかの境界がある

	if (length < def_r)
	{
		// atkの中点がdef側に埋まっている
		// atkの球面を接触点にする
		resHitPos.x = aC.x + atk_r * nmlVect.x;
		resHitPos.y = aC.y + atk_r * nmlVect.y;
		resHitPos.z = aC.z + atk_r * nmlVect.z;
	}
	else
	{
		// defの中点がatk側に埋まっている または双方の中点が十分に離れている
		// defの球面を接触点にする
		resHitPos.x = dC.x - def_r * nmlVect.x;
		resHitPos.y = dC.y - def_r * nmlVect.y;
		resHitPos.z = dC.z - def_r * nmlVect.z;
	}

	return true;
}

// 球と円柱
bool isHitVS( HitSphere* atk,  HitPillar* def, XMFLOAT3& resHitPos, bool revCheck)
{
	HitCircle aC;

	HitCircle* pDC = def->GetCircle();
	HitQuad dQ;

	XMFLOAT3 circlePos;
	XMFLOAT3 sphereCenter = atk->GetCenter();

	// ZX平面で円判定のチェック
	aC.SetRadius(atk->GetRadius());
	aC.SetCenter(sphereCenter.x, sphereCenter.z);

	if (!isHitVS(&aC, pDC, circlePos))
		return false;


	// XYとZY 平面 円VS矩形判定
	aC.SetCenter(sphereCenter.x, sphereCenter.y);

	float defTop = def->GetTop();
	float defBottom = def->GetBottom();
	float defHeight = defTop - defBottom;
	float defCenter = defBottom + defHeight * 0.5f;
	dQ.SetHitSize(pDC->GetRadius() * 2.0f, defHeight);
	dQ.SetPosition(pDC->GetCenterPosition().x, defCenter);

	XMFLOAT3 CQPos;
	if (!isHitVS(&dQ, &aC, CQPos, !revCheck))
		return false;

	aC.SetCenter(sphereCenter.z, sphereCenter.y);

	// PillarのCircleは中心座標がX,Z
	dQ.SetPosition(pDC->GetCenterPosition().y, defCenter);

	if (!isHitVS(&dQ, &aC, CQPos, !revCheck))
		return false;

	resHitPos = circlePos;
	resHitPos.y = CQPos.y;

	return true;
}

bool isHitVS(HitSphere* atk, HitRayLine* def, XMFLOAT3& resHitPos, bool revCheck)
{
	return false;
}

// 円柱と円柱
bool isHitVS( HitPillar* atk,  HitPillar* def, XMFLOAT3& resHitPos)
{
	// XZチェック
	if (!isHitVS(atk->GetCircle(), def->GetCircle(), resHitPos))
	{
		// これが当範囲外ならば、当たっていない
		return false;
	}

	// 高さチェック
	if (atk->GetBottom() > def->GetTop() || atk->GetTop() < def->GetBottom())
		return false;	// 高さがずれている。

	// XZ判定でresHitPosにXYにXZ値が入っているのでY値をZに移動
	resHitPos.z = resHitPos.y;

	// 接触Y値
	if (atk->GetTop() < def->GetTop() && atk->GetBottom() > def->GetBottom())
	{
		// def側にatkが入っている
		resHitPos.y = atk->GetCenter();
	}
	else if (def->GetTop() < atk->GetTop() && def->GetBottom() > atk->GetBottom())
	{
		// atk側にdefが入っている
		resHitPos.y = def->GetCenter();
	}
	else
	{
		// atkとdefはズレているので、def側の底辺または天辺の値をヒット点に
		if (atk->GetBottom() > def->GetBottom())
		{
			// defの底辺は抜けている
			resHitPos.y = def->GetTop();
		}
		else
		{
			// defの天辺は抜けている
			resHitPos.y = def->GetBottom();
		}
	}

	return true;
}

// 円柱と線分
bool isHitVS(HitPillar* atk, HitRayLine* def, XMFLOAT3& resHitPos, bool revCheck)
{
	// 円柱の高さに線分があるかどうか
	// 円柱を斜めにしないから、これで十分なはず
	float maxRayPos;
	float minRayPos;

	if (def->GetStart().y > def->GetEnd().y)
	{
		maxRayPos = def->GetStart().y;
		minRayPos = def->GetEnd().y;
	}
	else
	{
		maxRayPos = def->GetEnd().y;
		minRayPos = def->GetStart().y;
	}

	if (atk->GetBottom() > maxRayPos || atk->GetTop() < minRayPos)
		return false;


	// ここからは、円と線分の当たり判定
	HitRayLine pRayLine ;
	HitCircle* pPillarCircle = atk->GetCircle();
	XMFLOAT3 circlePos;

	if (!isHitVS(pPillarCircle, def, circlePos, !revCheck))
		return false;

	return true;
}

bool isHitVS(HitRayLine* atk, HitRayLine* def, XMFLOAT3& resHitPos)
{
	return false;
}

// 判定データ（HitStructure*）保存領域をメモリ的に削除してからクリア
void HitManager::FlushHitList(std::list<HitStructure*>* p_hitlist)
{
	std::for_each(p_hitlist->begin(), p_hitlist->end(),
		[](HitStructure* hitstr)
		{
			delete(hitstr);
		}
	);

	p_hitlist->clear();
}

void HitManager::InitHitList(UINT hitTypes)
{
	m_numOfHitTypes = hitTypes;

	m_hitArray.clear();

	for (UINT i = 0; i < m_numOfHitTypes; i++)
	{
		m_hitArray.push_back( std::make_unique<list<HitStructure*>>() );
	}
}

void HitManager::SetHitOrder(UINT atk, UINT def)
{
	if (atk != def && atk < m_numOfHitTypes && def < m_numOfHitTypes)
	{
		m_hitOrders.push_back({atk, def});
	}
}

void HitManager::RefreshHitSystem()	// MyHitSystemの持つ判定データ保存領域を一斉クリア
{
	// unique_ptrだから参照に
	for_each(m_hitArray.begin(), m_hitArray.end(), [this](unique_ptr<list<HitStructure*>>& u_list) {
		FlushHitList(u_list.get());
	});
}

// 矩形判定登録メソッド
void HitManager::SetHitArea(GameComponent* cmp, HitAreaBase* box)
{
	UINT hitType = box->GetHitType();
	if (hitType < m_numOfHitTypes)
	{
		HitStructure* hit = new HitStructure(cmp, box);	// hit構造体作成。

		unique_ptr<list<HitStructure*>>& u_list = m_hitArray[hitType];
		u_list->push_back(hit);
	}
}

GameComponent* HitManager::HitStructure::GetGameComponent()	// このHitStructureが持っているGameObjectを渡す
{
	return m_pGameComponent;
}

HitAreaBase* HitManager::HitStructure::GetHitArea()			// このHitStructureが持っているHitBoxClassを渡す
{
	return m_pHitArea;
}

void HitManager::HitFrameAction()
{
	// シールド判定が先に来る
	for_each(m_hitOrders.begin(), m_hitOrders.end(),

		[this](DirectX::XMUINT2& order)
		{
			unique_ptr<list<HitStructure*>>& atk = m_hitArray[order.x];

			for_each(atk->begin(), atk->end(),

				[this, order](HitStructure* p_plHit)
				{
					unique_ptr<list<HitStructure*>>& def = m_hitArray[order.y];

					for_each(def->begin(), def->end(),
						[this, p_plHit](HitStructure* p_enHit)
						{
							HitAreaBase* atkHit = nullptr;
							HitAreaBase* defHit = nullptr;

							if (p_plHit->GetHitArea()->GetHitShape() <= p_enHit->GetHitArea()->GetHitShape())
							{
								atkHit = p_plHit->GetHitArea();
								defHit = p_enHit->GetHitArea();
							}
							else
							{
								atkHit = p_enHit->GetHitArea();
								defHit = p_plHit->GetHitArea();
							}

							if (IsHit(atkHit, defHit))
							{
								// ヒットしたら両方のGameObjectに通知
								p_enHit->GetGameComponent()->HitReaction(p_plHit->GetGameComponent()->GetGameObject(), p_plHit->GetHitArea());
								p_plHit->GetGameComponent()->HitReaction(p_enHit->GetGameComponent()->GetGameObject(), p_enHit->GetHitArea());
							}
						}
					);
				}
			);
		}

	);
}

bool HitManager::IsHit(HitAreaBase* atkHit, HitAreaBase* defHit)
{
	bool hitcheck = false;

	// 最後のヒット位置チェックでどちらからのヒット座標を出すのか
	bool revCheck = (atkHit->GetHitShape() > defHit->GetHitShape());

	if (revCheck)
	{
		// メソッドの数を減らすため、defとatkを逆にします。
		HitAreaBase* tempHit = defHit;
		defHit = atkHit;
		atkHit = tempHit;
	}

	// HIT_SHAPEのswitch文で、該当する当たり判定同士を呼び出す
	switch (atkHit->GetHitShape())
	{
	case HIT_SHAPE::HS_QUAD:
		switch (defHit->GetHitShape())
		{
		case HIT_SHAPE::HS_QUAD:
			hitcheck = isHitVS(static_cast<HitQuad*>(atkHit), static_cast<HitQuad*>(defHit), m_lastHitPos);
			break;
		case HIT_SHAPE::HS_CIRCLE:
			hitcheck = isHitVS(static_cast<HitQuad*>(atkHit), static_cast<HitCircle*>(defHit), m_lastHitPos, revCheck);
			break;
		case HIT_SHAPE::HS_TRIANGLE:
			hitcheck = isHitVS(static_cast<HitQuad*>(atkHit), static_cast<HitTriangle*>(defHit), m_lastHitPos, revCheck);
			break;
		case HIT_SHAPE::HS_AABB:
			hitcheck = isHitVS(static_cast<HitQuad*>(atkHit), static_cast<HitAABB*>(defHit), m_lastHitPos, revCheck);
			break;
		case HIT_SHAPE::HS_OBB:
			hitcheck = isHitVS(static_cast<HitQuad*>(atkHit), static_cast<HitOBB*>(defHit), m_lastHitPos, revCheck);
			break;
		case HIT_SHAPE::HS_SPHERE:
			hitcheck = isHitVS(static_cast<HitQuad*>(atkHit), static_cast<HitSphere*>(defHit), m_lastHitPos, revCheck);
			break;
		case HIT_SHAPE::HS_PILLAR:
			hitcheck = isHitVS(static_cast<HitQuad*>(atkHit), static_cast<HitPillar*>(defHit), m_lastHitPos, revCheck);
			break;

		case HIT_SHAPE::HS_LINE:
			hitcheck = isHitVS(static_cast<HitQuad*>(atkHit), static_cast<HitRayLine*>(defHit), m_lastHitPos, revCheck);
			break;
		}
		break;

	case HIT_SHAPE::HS_CIRCLE:
		switch (defHit->GetHitShape())
		{
		case HIT_SHAPE::HS_CIRCLE:
			hitcheck = isHitVS(static_cast<HitCircle*>(atkHit), static_cast<HitCircle*>(defHit), m_lastHitPos);
			break;
		case HIT_SHAPE::HS_TRIANGLE:
			hitcheck = isHitVS(static_cast<HitCircle*>(atkHit), static_cast<HitTriangle*>(defHit), m_lastHitPos, revCheck);
			break;
		case HIT_SHAPE::HS_AABB:
			hitcheck = isHitVS(static_cast<HitCircle*>(atkHit), static_cast<HitAABB*>(defHit), m_lastHitPos, revCheck);
			break;
		case HIT_SHAPE::HS_OBB:
			hitcheck = isHitVS(static_cast<HitCircle*>(atkHit), static_cast<HitOBB*>(defHit), m_lastHitPos, revCheck);
			break;
		case HIT_SHAPE::HS_SPHERE:
			hitcheck = isHitVS(static_cast<HitCircle*>(atkHit), static_cast<HitSphere*>(defHit), m_lastHitPos, revCheck);
			break;
		case HIT_SHAPE::HS_PILLAR:
			hitcheck = isHitVS(static_cast<HitCircle*>(atkHit), static_cast<HitPillar*>(defHit), m_lastHitPos, revCheck);
			break;

		case HIT_SHAPE::HS_LINE:
			hitcheck = isHitVS(static_cast<HitCircle*>(atkHit), static_cast<HitRayLine*>(defHit), m_lastHitPos, revCheck);
			break;
		}
		break;

	case HIT_SHAPE::HS_TRIANGLE:
		switch (defHit->GetHitShape())
		{
		case HIT_SHAPE::HS_TRIANGLE:
			hitcheck = isHitVS(static_cast<HitTriangle*>(atkHit), static_cast<HitTriangle*>(defHit), m_lastHitPos);
			break;
		case HIT_SHAPE::HS_AABB:
			hitcheck = isHitVS(static_cast<HitTriangle*>(atkHit), static_cast<HitAABB*>(defHit), m_lastHitPos, revCheck);
			break;
		case HIT_SHAPE::HS_OBB:
			hitcheck = isHitVS(static_cast<HitTriangle*>(atkHit), static_cast<HitOBB*>(defHit), m_lastHitPos, revCheck);
			break;
		case HIT_SHAPE::HS_SPHERE:
			hitcheck = isHitVS(static_cast<HitTriangle*>(atkHit), static_cast<HitSphere*>(defHit), m_lastHitPos, revCheck);
			break;
		case HIT_SHAPE::HS_PILLAR:
			hitcheck = isHitVS(static_cast<HitTriangle*>(atkHit), static_cast<HitPillar*>(defHit), m_lastHitPos, revCheck);
			break;

		case HIT_SHAPE::HS_LINE:
			hitcheck = isHitVS(static_cast<HitTriangle*>(atkHit), static_cast<HitRayLine*>(defHit), m_lastHitPos);
			break;
		}
		break;

	case HIT_SHAPE::HS_AABB:
		switch (defHit->GetHitShape())
		{
		case HIT_SHAPE::HS_AABB:
			hitcheck = isHitVS(static_cast<HitAABB*>(atkHit), static_cast<HitAABB*>(defHit), m_lastHitPos);
			break;
		case HIT_SHAPE::HS_OBB:
			hitcheck = isHitVS(static_cast<HitAABB*>(atkHit), static_cast<HitOBB*>(defHit), m_lastHitPos, revCheck);
			break;
		case HIT_SHAPE::HS_SPHERE:
			hitcheck = isHitVS(static_cast<HitAABB*>(atkHit), static_cast<HitSphere*>(defHit), m_lastHitPos, revCheck);
			break;
		case HIT_SHAPE::HS_PILLAR:
			hitcheck = isHitVS(static_cast<HitAABB*>(atkHit), static_cast<HitPillar*>(defHit), m_lastHitPos, revCheck);
			break;

		case HIT_SHAPE::HS_LINE:
			hitcheck = isHitVS(static_cast<HitAABB*>(atkHit), static_cast<HitRayLine*>(defHit), m_lastHitPos, revCheck);
			break;
		}
		break;

	case HIT_SHAPE::HS_OBB:
		switch (defHit->GetHitShape())
		{
		case HIT_SHAPE::HS_OBB:
			hitcheck = isHitVS(static_cast<HitOBB*>(atkHit), static_cast<HitOBB*>(defHit), m_lastHitPos);
			break;
		case HIT_SHAPE::HS_SPHERE:
			hitcheck = isHitVS(static_cast<HitOBB*>(atkHit), static_cast<HitSphere*>(defHit), m_lastHitPos, revCheck);
			break;
		case HIT_SHAPE::HS_PILLAR:
			hitcheck = isHitVS(static_cast<HitOBB*>(atkHit), static_cast<HitPillar*>(defHit), m_lastHitPos, revCheck);
			break;

		case HIT_SHAPE::HS_LINE:
			hitcheck = isHitVS(static_cast<HitOBB*>(atkHit), static_cast<HitRayLine*>(defHit), m_lastHitPos, revCheck);
			break;
		}
		break;

	case HIT_SHAPE::HS_SPHERE:
		switch (defHit->GetHitShape())
		{
		case HIT_SHAPE::HS_SPHERE:
			hitcheck = isHitVS(static_cast<HitSphere*>(atkHit), static_cast<HitSphere*>(defHit), m_lastHitPos);
			break;
		case HIT_SHAPE::HS_PILLAR:
			hitcheck = isHitVS(static_cast<HitSphere*>(atkHit), static_cast<HitPillar*>(defHit), m_lastHitPos, revCheck);
			break;

		case HIT_SHAPE::HS_LINE:
			hitcheck = isHitVS(static_cast<HitSphere*>(atkHit), static_cast<HitRayLine*>(defHit), m_lastHitPos, revCheck);
			break;
		}
		break;

	case HIT_SHAPE::HS_PILLAR:
		switch (defHit->GetHitShape())
		{
		case HIT_SHAPE::HS_PILLAR:
			hitcheck = isHitVS(static_cast<HitPillar*>(atkHit), static_cast<HitPillar*>(defHit), m_lastHitPos);
			break;

		case HIT_SHAPE::HS_LINE:
			hitcheck = isHitVS(static_cast<HitPillar*>(atkHit), static_cast<HitRayLine*>(defHit), m_lastHitPos, revCheck);
			break;
		}
		break;

	case HIT_SHAPE::HS_LINE:
		switch (defHit->GetHitShape())
		{
		case HIT_SHAPE::HS_LINE:
			hitcheck = isHitVS(static_cast<HitRayLine*>(atkHit), static_cast<HitRayLine*>(defHit), m_lastHitPos);
			break;
		}
		break;
	}

	return hitcheck;
}

