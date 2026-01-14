#pragma once
#include "GameObject.h"

#include "CameraComponent.h"
#include "FBXCharacterData.h"

// =====動く地形 対応
#include "TerrainComponent.h"
// =====動く地形 対応 END

#include "PlayTimerUI.h"

#include <HitShapes.h>

#include "NightmareDragonEnum.h"
#include "NightmareDragonAIStateBase.h"

#include "FireBreatheEffect.h"
#include "ParchmentResultUI.h"

class NightmareDragonEnemy :
    public GameComponent
{
private:
    HitSphere   m_nightmareDragonHit;   // ドラゴンの当たり判定
    int         m_nowDragonHp;          // ドラゴンのHP
    bool        m_isDestroy;            // 破壊フラグ

    void ResolveByNowState();           // ステートパターンを変更する処理

    // アニメーション用の処理集
    NightmareDragonAIStateBase* m_currentNDAIState;
    NightmareDragonAIStateBase* m_prevNDAIState;
    IdleNDAIState m_idleNDState;
    WalkNDAIState m_walkNDState;
    ClawAttackNDAIState m_clawAttackNDState;
    HornAttackNDAIState m_hornAttackNDState;
    ScreamNDAIState m_screamNDState;
    DieNDAIState m_dieNDState;
    RunNDAIState m_runNDState;
    GetHitNDAIState m_getHitNDState;
    FireBreathAttackNDAIState m_breatheAttackNDState;

    // ブレス処理用に追加
    FireBreatheEffect* m_pFireBreathComp;

    ParchmentResultUI* m_pParchmentResultComp;

protected:
    UINT m_nowDragonEnemyState;     // 現在のステート
    UINT m_prevDragonEnemyState;    // 前回のステート
    XMFLOAT3 m_nowPlayerPos;        // プレイヤー現在の位置

public:
    HitSphere   m_nightmareDragonAttackHit; // ドラゴンの攻撃の当たり判定
    float       m_centerY;                  // ドラゴンの中心位置
    XMFLOAT3    m_forwardVect;

    //  GameComponent を介して継承されました
    void InitAction() override;     // コンポーネント初期化時に呼ばれる処理
    bool FrameAction() override;    // 毎フレーム呼ばれる処理　falseを返すとこのコンポーネントは終了し削除される
    void FinishAction() override;   // 終了時に呼ばれる処理

    // ヒット時の処理
    virtual void HitReaction(GameObject* targetGo, HitAreaBase* hit) override;

    // プレイヤーの位置更新
    void ChangePlayerPos(float x, float y, float z)
    {
        m_nowPlayerPos.x = x;
        m_nowPlayerPos.y = y;
        m_nowPlayerPos.z = z;
    }

    // m_nowDragonEnemyStateを変更
    void ChangeNowDragonEnemyState(UINT dragonEnemyState)
    {
        m_nowDragonEnemyState = dragonEnemyState;
        ResolveByNowState();
    }

    // 破壊します
    void SetIsDestroy()
    {
        m_isDestroy = true;
    }

    // ブレス用のFireBreatheEffectコンポーネント
    void SetFireBreatheEffectComponent(FireBreatheEffect* fireBreathComp)
    {
        m_pFireBreathComp = fireBreathComp;
    }

    // 倒された報告用のParchmentResultUIコンポーネントをセット
    void SetParchmentResultComp(ParchmentResultUI* pRComp)
    {
        m_pParchmentResultComp = pRComp;
    }

    // ブレスを呼び出す
    void DrawFireBreathe()
    {
        m_pFireBreathComp->DrawFireBreathe();
    }
};


