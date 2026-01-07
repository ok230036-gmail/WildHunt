#pragma once
#include <HitShapes.h>

#include "GameObject.h"

#include <DirectXMath.h>
#include <vector>

using namespace DirectX;

class TerrainComponent : public GameComponent
{
private:
	// 当たり判定データクラス
	class TerrainPlate
	{
	private:
		HitAABB subHit;			// ブロードヒット
		HitTriangle mainHit;	// ローカルヒット

	public:
		// データ設定用メソッド
		void SetTerrainPlate(const XMFLOAT3& pA, const XMFLOAT3& pB, const XMFLOAT3& pC);

		// 当たり判定データへのレイキャスト（線分VS三角形）
		bool RayCastHit(HitRayLine& ray, XMFLOAT3& hitPosition, XMFLOAT3& hitNormal);
	};

	HitAABB m_bloadHit;	// 地形データ全体のブロードヒット
	std::vector<unique_ptr<TerrainPlate>> m_terrainHit;	// ヒット判定データ配列

public:
	void InitAction() override;		// コンポーネント初期化時に呼ばれる処理
	bool FrameAction() override;	// 毎フレーム呼ばれる処理　falseを返すとこのコンポーネントは終了し削除される
	void FinishAction() override;	// 終了時に呼ばれる処理

	// ヒット時の処理
	void HitReaction(GameObject* obj, HitAreaBase* hit) override;

	bool RayCastHit(const XMFLOAT3& rayStart, const XMFLOAT3& rayEnd, XMFLOAT3& hitPosition, XMFLOAT3& hitNormal);
	bool RayCastHit(HitRayLine& ray, XMFLOAT3& hitPosition, XMFLOAT3& hitNormal);
};
