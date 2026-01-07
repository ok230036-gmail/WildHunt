#include "HeartItemComponent.h"
#include "FBXCharacterData.h"	// FBXCharacterDataを使うので

#include "WildHuntScene.h"
#include "WildHuntEnum.h"

// コンポーネント初期化時に呼ばれる処理
void HeartItemComponent::InitAction()
{
	FBXCharacterData* chdata = static_cast<FBXCharacterData*>(GetGameObject()->GetCharacterData());
	// chdata->SetGraphicsPipeLine(L"StaticFBX");
	// chdata->SetGraphicsPipeLine(L"StaticPhong");
	// chdata->SetGraphicsPipeLine(L"StaticBlinn");
	chdata->SetGraphicsPipeLine(L"StaticToon");	//アニメーションなしToon


	chdata->SetScale(0.01f, 0.01f, 0.01f);	// 元モデルがかなり大きいので縮小

	XMFLOAT3 scl = chdata->GetScale();
	XMFLOAT3 min = chdata->GetMainFbx()->GetFbxMin();
	XMFLOAT3 max = chdata->GetMainFbx()->GetFbxMax();

	m_centerY = (max.y - min.y) * scl.y * 0.5f;	// 0.5 scale

	m_itemHit.SetRadius(m_centerY * 0.8f);	// 適当な半径
	m_itemHit.SetAttackType((UINT)HIT_ORDER::HIT_ITEM, 0);
	sta = 0;
}

// 毎フレーム呼ばれる処理　falseを返すとこのコンポーネントは終了し削除される
bool HeartItemComponent::FrameAction()
{
	switch (sta)
	{
	case 0:
	{
		FBXCharacterData* chData = static_cast<FBXCharacterData*>(GetGameObject()->GetCharacterData());

		// PipeLineに登録
		chData->GetPipeline()->AddRenerObject(chData);

		XMFLOAT3 pos = chData->GetPosition();

		m_itemHit.SetCenter(pos.x, pos.y + m_centerY, pos.z);

		MyAccessHub::GetMyGameEngine()->GetHitManager()->SetHitArea(this, &m_itemHit);

	}
		break;

	default:
		// 取得後
		break;
	}


	return true;
}

// 終了時に呼ばれる処理
void HeartItemComponent::FinishAction()
{
}

// ヒット時の処理
void HeartItemComponent::HitReaction(GameObject* targetGo, HitAreaBase* hit)
{
	// 描画をしなくなります
	sta = 1;
}
