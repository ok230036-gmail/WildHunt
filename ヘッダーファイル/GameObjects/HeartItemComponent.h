#pragma once
#include "GameObject.h"

#include <HitShapes.h>

class HeartItemComponent :
    public GameComponent
{
private:
    HitSphere   m_itemHit;  // 球で当たり判定
    float       m_centerY;  // 中心座標
    int         sta;        // switch文で処理を変更する用

public:
    //  GameComponent を介して継承されました
    virtual void InitAction() override;     // コンポーネント初期化時に呼ばれる処理
    virtual bool FrameAction() override;    // 毎フレーム呼ばれる処理　falseを返すとこのコンポーネントは終了し削除される
    virtual void FinishAction() override;   // 終了時に呼ばれる処理

    // ヒット時の処理
    virtual void HitReaction(GameObject* targetGo, HitAreaBase* hit) override;
};

