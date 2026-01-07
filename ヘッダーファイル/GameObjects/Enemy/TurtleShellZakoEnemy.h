#pragma once
#include "ZakoEnemyBase.h"
#include "WildHuntUIRender.h"

class TurtleShellZakoEnemy :
    public ZakoEnemyBase
{
private:
    HitSphere   m_turtleShellHit;       // タートルシェルの当たり判定
    HitSphere   m_turtleShellAttackHit; // タートルシェルの攻撃の当たり判定
    XMFLOAT3    m_forwardVect;
    float       m_centerY;
    int         sta;
    
    XMFLOAT3    m_knockbackVect;
    bool        m_isKnockback;

public:
    // ヒット時の処理
    virtual void HitReaction(GameObject* targetGo, HitAreaBase* hit) override;

    void FirstInitAction() override;	// InitActionの最初に呼ばれる処理
    void FirstFrameAction() override;   // FrameActionの最初に呼ばれる処理

    void ExtraInitAction() override;	// InitActionの最後に呼ばれる処理
    void ExtraFrameAction() override;	// FrameActionの最後に呼ばれる処理

    // 追加のヒット時処理
    void ExtraHitReaction() override;
};

