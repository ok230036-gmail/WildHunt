#pragma once
#include "GameObject.h"
#include "SpriteCharacter.h"

class PlayTimerBGUI :
    public GameComponent
{
private:
    std::unique_ptr<SpriteCharacter> m_timerBGSp;   // タイマーの背景画像

public:
    //  GameComponent を介して継承されました
    void InitAction() override;     // コンポーネント初期化時に呼ばれる処理
    bool FrameAction() override;    // 毎フレーム呼ばれる処理　falseを返すとこのコンポーネントは終了し削除される
    void FinishAction() override;   // 終了時に呼ばれる処理
};

