#pragma once
#include <GameObject.h>
#include "FBXCharacterData.h"


class SkyDomeComponent : public GameComponent
{
private:
	CharacterData* centerCharacter;

public:
	void InitAction() override;		// コンポーネント初期化時に呼ばれる処理
	bool FrameAction() override;	// 毎フレーム呼ばれる処理　falseを返すとこのコンポーネントは終了し削除される
	void FinishAction() override;	// 終了時に呼ばれる処理

	void SetCenterCharacter(CharacterData* target);
};
