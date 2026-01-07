#include <MyAccessHub.h>
#include <HitManager.h>
#include "TerrainComponent.h"

#include "FBXCharacterData.h"

// コンポーネント初期化時に呼ばれる処理
void TerrainComponent::InitAction()
{
	FBXCharacterData* fbxChara = static_cast<FBXCharacterData*>(GetGameObject()->GetCharacterData());
	fbxChara->SetGraphicsPipeLine(L"StaticToon"); // アニメなしToon

	// 広めのTerrainの当たり判定
	m_bloadHit.SetAABBMinMax(fbxChara->GetMainFbx()->GetFbxMin(), fbxChara->GetMainFbx()->GetFbxMax());

	MeshContainer* meshCon;
	FbxVertex *vA, *vB, *vC;
	unique_ptr<TerrainPlate> pTerrainPlate;

	int index = 0;
	UINT vcount = 0;

	while ((meshCon = fbxChara->GetMainFbx()->GetMeshContainer(index)) != nullptr)
	{
		vcount = meshCon->m_vertexCount;
		
		for (UINT i = 0; i < vcount; i+=3)
		{
			vA = &meshCon->m_vertexData[meshCon->m_indexData[i]];
			vB = &meshCon->m_vertexData[meshCon->m_indexData[i + 1]];
			vC = &meshCon->m_vertexData[meshCon->m_indexData[i + 2]];

			pTerrainPlate = make_unique<TerrainPlate>();
			pTerrainPlate.get()->SetTerrainPlate(vA->position, vB->position, vC->position);

			m_terrainHit.push_back(move(pTerrainPlate));
		}

		index++;
	}
}

// 毎フレーム呼ばれる処理　falseを返すとこのコンポーネントは終了し削除される
bool TerrainComponent::FrameAction()
{
	// PipeLineに登録
	GetGameObject()->GetCharacterData()->GetPipeline()->AddRenerObject(GetGameObject()->GetCharacterData());
	return true;
}

// 終了時に呼ばれる処理
void TerrainComponent::FinishAction()
{
}

void TerrainComponent::HitReaction(GameObject* obj, HitAreaBase* hit)
{
}

bool TerrainComponent::RayCastHit(const XMFLOAT3& rayStart, const XMFLOAT3& rayEnd, XMFLOAT3& hitPosition, XMFLOAT3& hitNormal)
{
	HitRayLine rayLine;

	rayLine.SetLine(rayStart, rayEnd, 0);

	return RayCastHit(rayLine, hitPosition, hitNormal);
}

bool TerrainComponent::RayCastHit(HitRayLine& ray, XMFLOAT3& hitPosition, XMFLOAT3& hitNormal)
{
	HitManager* hitMng = MyAccessHub::GetMyGameEngine()->GetHitManager();

	// 地形のアフィン変換をレイに適応する（逆行列をかける）
	HitRayLine trRay = ray;
	CharacterData* chData = GetGameObject()->GetCharacterData();

	HitRayLine::GetTransformedLine(trRay, chData->GetInverseWorldMatrix());

	XMVECTOR nmlVect = trRay.GetNormal();

	XMFLOAT3 rayNormal = {XMVectorGetX(nmlVect), XMVectorGetY(nmlVect), XMVectorGetZ(nmlVect)};
	XMFLOAT3 tempHit = {};
	XMFLOAT3 tempNml = {};

	bool res = false;


	if (hitMng->IsHit(&trRay, &m_bloadHit))
	{
		// 概形にヒットしたので詳細部分チェック
		for (int i = 0; i < m_terrainHit.size(); i++)
		{
			TerrainPlate* terHit = m_terrainHit[i].get();
			
			if (terHit->RayCastHit(trRay, tempHit, tempNml))
			{
				if (!res)
				{
					// 初ヒット
					res = true;
					hitPosition = tempHit;
					hitNormal = tempNml;
		 		}
				else
				{
					// より始点に近い座標をヒットとする。
					bool ow = false;
					if (rayNormal.x > 0.0f)
					{
						if (hitPosition.x > tempHit.x)
						{
							hitPosition.x = tempHit.x;
							ow = true;
						}
					}
					else
					{
						if (hitPosition.x < tempHit.x)
						{
							hitPosition.x = tempHit.x;
							ow = true;
						}
					}

					if (rayNormal.y > 0.0f)
					{
						if (hitPosition.y > tempHit.y)
						{
							hitPosition.y = tempHit.y;
							ow = true;
						}
					}
					else
					{
						if (hitPosition.y < tempHit.y)
						{
							hitPosition.y = tempHit.y;
							ow = true;
						}
					}

					if (rayNormal.z > 0.0f)
					{
						if (hitPosition.z > tempHit.z)
						{
							hitPosition.z = tempHit.z;
							ow = true;
						}
					}
					else
					{
						if (hitPosition.z < tempHit.z)
						{
							hitPosition.z = tempHit.z;
							ow = true;
						}
					}

					// 座標の書き換えがあった
					if (ow)
					{
						// ノーマルを合成
						hitNormal.x = (hitNormal.x + tempNml.x) * 0.5f;
						hitNormal.y = (hitNormal.y + tempNml.y) * 0.5f;
						hitNormal.z = (hitNormal.z + tempNml.z) * 0.5f;
					}
				}
			}
		}
	}

	// 結果値に現在のアフィン変換を掛ける
	if (res)
	{
		XMVECTOR posV = XMLoadFloat3(&hitPosition);
		XMVECTOR nmlV = XMLoadFloat3(&hitNormal);
		XMMATRIX& mtx = chData->GetWorldMatrix();

		posV = XMVector3Transform(posV, mtx);
		nmlV = XMVector3Transform(nmlV, mtx);
		nmlV = XMVector3Normalize(nmlV);

		hitPosition.x = XMVectorGetX(posV);
		hitPosition.y = XMVectorGetY(posV);
		hitPosition.z = XMVectorGetZ(posV);

		hitNormal.x = XMVectorGetX(nmlV);
		hitNormal.y = XMVectorGetY(nmlV);
		hitNormal.z = XMVectorGetZ(nmlV);
	}

	return res;
}

void TerrainComponent::TerrainPlate::SetTerrainPlate(const XMFLOAT3& pA, const XMFLOAT3& pB, const XMFLOAT3& pC)
{
	mainHit.SetTriangle(pA, pB, pC);

	// Bload Hit
	XMFLOAT3 max = pA;
	XMFLOAT3 min = pA;

	// max min check
	for (int i = 0; i < 2; i++)
	{
		XMFLOAT3 target;
		switch (i)
		{
		case 0:
			target = pB;
			break;

		default:
			target = pC;
			break;
		}

		// max check
		if (max.x < target.x)
			max.x = target.x;
		if (max.y < target.y)
			max.y = target.y;
		if (max.z < target.z)
			max.z = target.z;

		// min check
		if (min.x > target.x)
			min.x = target.x;
		if (min.y > target.y)
			min.y = target.y;
		if (min.z > target.z)
			min.z = target.z;

	}

	subHit.SetAABBMinMax(min, max);
}

bool TerrainComponent::TerrainPlate::RayCastHit(HitRayLine& ray, XMFLOAT3& hitPosition, XMFLOAT3& hitNormal)
{
	HitManager* hitMng = MyAccessHub::GetMyGameEngine()->GetHitManager();

	// bload hit
	if (!hitMng->IsHit(&ray, &subHit))
	{
		return false;
	}

	// local hit
	if (hitMng->IsHit(&ray, &mainHit))
	{
		XMFLOAT3 pos = hitMng->GetLastHitPoint();
		hitPosition.x = pos.x;
		hitPosition.y = pos.y;
		hitPosition.z = pos.z;

		XMVECTOR nml = mainHit.GetNormal();
		hitNormal.x = XMVectorGetX(nml);
		hitNormal.y = XMVectorGetY(nml);
		hitNormal.z = XMVectorGetZ(nml);

		return true;
	}

	return false;
}