#pragma once
#include "GameObject.h"

#include "CameraComponent.h"
#include "FBXCharacterData.h"
#include "WildHuntUIRender.h"
#include <HitShapes.h>

#include "NightmareDragonEnum.h"

// 循環参照回避のための前方宣言
class NightmareDragonEnemy;

// アニメーションごとの処理を纏めたポリモーフィズム。
class NightmareDragonAIStateBase
{
public:
    virtual void Enter(FBXCharacterData* chData) = 0;   // このステートに遷移した直後に一度だけ呼ばれる処理
    virtual void Update(FBXCharacterData* chData) = 0;  // ステート中の毎フレーム処理
    virtual void Exit(FBXCharacterData* chData) = 0;    // ステート終了時の後処理

    // プレイヤーの位置を更新
    void SetPlayerPos(float x, float y, float z)
    {
        m_nowPlayerPos.x = x;
        m_nowPlayerPos.y = y;
        m_nowPlayerPos.z = z;
    };

    // m_pNightmareDragonCompのポインタを設定
    void SetNightmareDragonComp(NightmareDragonEnemy* NDComp)
    {
        m_pNightmareDragonComp = NDComp;
    }

    // ドラゴンのStateを変更
    void ChangeNowDragonEnemyState(UINT dragonEnemyState)
    {
        m_nowDragonEnemyState = dragonEnemyState;
    }

protected:
    XMFLOAT3                m_nowPlayerPos;         // プレイヤーの現在位置
    UINT                    m_nowDragonEnemyState;  // ドラゴンのStateを保持
    NightmareDragonEnemy*   m_pNightmareDragonComp; // NightmareDragonEnemyのポインタ

    float attackCnt;    // 攻撃フレームの管理用
};

// アニメーション群の初期設定
// 基本的にEnter、Update、Exitのみ
class IdleNDAIState : public NightmareDragonAIStateBase
{
private:
    void Enter(FBXCharacterData* chData) override;  // このステートに遷移した直後に一度だけ呼ばれる処理
    void Update(FBXCharacterData* chData) override; // ステート中の毎フレーム処理
    void Exit(FBXCharacterData* chData) override;   // ステート終了時の後処理
};

class WalkNDAIState : public NightmareDragonAIStateBase
{
private:
    void Enter(FBXCharacterData* chData) override;  // このステートに遷移した直後に一度だけ呼ばれる処理
    void Update(FBXCharacterData* chData) override; // ステート中の毎フレーム処理
    void Exit(FBXCharacterData* chData) override;   // ステート終了時の後処理
};

class ClawAttackNDAIState : public NightmareDragonAIStateBase
{
private:
    void Enter(FBXCharacterData* chData) override;  // このステートに遷移した直後に一度だけ呼ばれる処理
    void Update(FBXCharacterData* chData) override; // ステート中の毎フレーム処理
    void Exit(FBXCharacterData* chData) override;   // ステート終了時の後処理
};

class HornAttackNDAIState : public NightmareDragonAIStateBase
{
private:
    void Enter(FBXCharacterData* chData) override;  // このステートに遷移した直後に一度だけ呼ばれる処理
    void Update(FBXCharacterData* chData) override; // ステート中の毎フレーム処理
    void Exit(FBXCharacterData* chData) override;   // ステート終了時の後処理
};

class ScreamNDAIState : public NightmareDragonAIStateBase
{
private:
    void Enter(FBXCharacterData* chData) override;  // このステートに遷移した直後に一度だけ呼ばれる処理
    void Update(FBXCharacterData* chData) override; // ステート中の毎フレーム処理
    void Exit(FBXCharacterData* chData) override;   // ステート終了時の後処理
};

class DieNDAIState : public NightmareDragonAIStateBase
{
private:
    void Enter(FBXCharacterData* chData) override;  // このステートに遷移した直後に一度だけ呼ばれる処理
    void Update(FBXCharacterData* chData) override; // ステート中の毎フレーム処理
    void Exit(FBXCharacterData* chData) override;   // ステート終了時の後処理
};

class RunNDAIState : public NightmareDragonAIStateBase
{
private:
    void Enter(FBXCharacterData* chData) override;  // このステートに遷移した直後に一度だけ呼ばれる処理
    void Update(FBXCharacterData* chData) override; // ステート中の毎フレーム処理
    void Exit(FBXCharacterData* chData) override;   // ステート終了時の後処理
};

class GetHitNDAIState : public NightmareDragonAIStateBase
{
private:
    void Enter(FBXCharacterData* chData) override;  // このステートに遷移した直後に一度だけ呼ばれる処理
    void Update(FBXCharacterData* chData) override; // ステート中の毎フレーム処理
    void Exit(FBXCharacterData* chData) override;   // ステート終了時の後処理
};

class FireBreathAttackNDAIState : public NightmareDragonAIStateBase
{
private:
    void Enter(FBXCharacterData* chData) override;  // このステートに遷移した直後に一度だけ呼ばれる処理
    void Update(FBXCharacterData* chData) override; // ステート中の毎フレーム処理
    void Exit(FBXCharacterData* chData) override;   // ステート終了時の後処理
};
