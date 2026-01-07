#pragma once
#include "GameObject.h"
#include "CameraComponent.h"
#include "FBXCharacterData.h"
#include "ThirdPersonCameraController.h"
#include "SwordHit2DEffect.h"

class PlayerWeaponSword : public GameComponent
{
private:
    // ワールドマトリクス作成に必要なプレイヤーの詳細情報たち
    XMFLOAT3 m_nowPlayerPos;
    XMFLOAT3 m_nowPlayerScale;
    XMFLOAT3 m_nowPlayerRotate;
    XMMATRIX m_nowPlayerHandData;
    FBXDataContainer* m_currentCharaCont; // 現在使用しているアニメ用FbxDataContainerのポインタ

    ThirdPersonCameraController* m_pTPCameraComp;   // カメラのポインタ
    SwordHit2DEffect* m_pSwordHit2DEffectComp;      // 剣のヒットエフェクトのポインタ

    HitSphere m_swordHit;   // 剣の当たり判定
    float m_centerX;
    float m_centerY;
    float m_centerZ;

    bool m_nowWeaponAttack; // 今、攻撃しているかどうか

public:
    //  GameComponent を介して継承されました
    void InitAction() override;     // コンポーネント初期化時に呼ばれる処理
    bool FrameAction() override;    // 毎フレーム呼ばれる処理　falseを返すとこのコンポーネントは終了し削除される
    void FinishAction() override;   // 終了時に呼ばれる処理

    // ヒット時の処理
    virtual void HitReaction(GameObject* targetGo, HitAreaBase* hit) override;

    // プレイヤーの手のデータを渡し続ける
    void ChangePlayerHandData(XMMATRIX handData)
    {
        m_nowPlayerHandData = handData;
    }

    // プレイヤーの位置更新
    void ChangePlayerPos(float x, float y, float z)
    {
        m_nowPlayerPos.x = x;
        m_nowPlayerPos.y = y;
        m_nowPlayerPos.z = z;
    }

    // プレイヤーのスケール更新
    void ChangePlayerScale(float x, float y, float z)
    {
        m_nowPlayerScale.x = x;
        m_nowPlayerScale.y = y;
        m_nowPlayerScale.z = z;
    }

    // プレイヤーの角度更新
    void ChangePlayerRotate(float x, float y, float z)
    {
        m_nowPlayerRotate.x = x;
        m_nowPlayerRotate.y = y;
        m_nowPlayerRotate.z = z;
    }

    void WeaponAttack(bool bl)
    {
        m_nowWeaponAttack = bl;
    }

    // カメラのポインタを設定
    void SetThirdPersonCameraComponent(ThirdPersonCameraController* tpCameraComp)
    {
        m_pTPCameraComp = tpCameraComp;
    }

    // 剣のヒットエフェクトのポインタを設定
    void SetSwordHit2DEffectComponent(SwordHit2DEffect* swordHit2DEffComp)
    {
        m_pSwordHit2DEffectComp = swordHit2DEffComp;
    }
};