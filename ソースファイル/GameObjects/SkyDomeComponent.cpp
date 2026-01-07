#include <MyAccessHub.h>

#include "SkyDomeComponent.h"
#include "WildHuntScene.h"

// コンポーネント初期化時に呼ばれる処理
void SkyDomeComponent::InitAction()
{
	// パラメータ設定の為のCharacterData取得。
	CharacterData* chdata = GetGameObject()->GetCharacterData();
	chdata->SetScale(10.0f, 10.0f, 10.0f);		// モデルが小さいのでXYZともに１０倍

	chdata->SetGraphicsPipeLine(L"StaticFBX");	// アニメなしFBX
}

// 毎フレーム呼ばれる処理　falseを返すとこのコンポーネントは終了し削除される
bool SkyDomeComponent::FrameAction()
{
	CharacterData* myData = GetGameObject()->GetCharacterData();

	// スカイドームの中心は常にキャラクタの中心に移動する。つまり端が来ない。
	if (centerCharacter != nullptr)
	{
		XMFLOAT3 charaPos = centerCharacter->GetPosition();

		myData->SetPosition(charaPos.x, charaPos.y, charaPos.z);
	}

	// PipeLineに登録
	myData->GetPipeline()->AddRenerObject(myData);

	return true;
}

// 終了時に呼ばれる処理
void SkyDomeComponent::FinishAction()
{
}

void SkyDomeComponent::SetCenterCharacter(CharacterData* target)
{
	centerCharacter = target;
}
