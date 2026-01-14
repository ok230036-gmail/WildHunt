#pragma once
#include "GameObject.h"
#include "CameraComponent.h"

#include "TerrainComponent.h"

#include "PlayerWeaponSword.h"
#include "ZakoEnemyBase.h"
#include "PlayerHPbarUI.h"
#include "NightmareDragonEnemy.h"
#include "ThirdPersonCameraController.h"
#include "TransitionAnimator.h"
#include "WildHuntEnum.h"

#include <HitShapes.h>

enum class UnityChanMotion
{
    Idle,
    Walk,
    LAttack,
    Jump,
    Evasion,
};

class UnityChanPlayer :
    public GameComponent
{
private:
    float	m_unityChanHeadHeight;  // 頭部までのオフセット
    float	m_walkableHeight;	// 足元段差許容位置

    float m_maxPlHp;    // 最大HP
    float m_nowPlHp;    // 現在HP
    float m_plTotalDamage;    // トータルダメージ量

    // ジャンプ実装
    float   m_jumpPower;
    float   m_gravityPower;
    float   m_terminalVelocity;
    float   m_YSpeed;

    bool    m_onGround;

    // これ正直Playerの抽象クラスか管理オブジェクト作ってそこからってやる方が良いんだけど、
    // コードが多くなるとサンプル追いかけづらいと思うので纏めます。
    CameraComponent* m_currentCamera;

    // Unityちゃん本体のヒット判定
    HitPillar bodyColl;
    float m_hitHeight;

    TerrainComponent* m_currentTerrain;
    XMMATRIX m_lastMatrix;  // 最後に変換した行列

    PlayerWeaponSword* m_pSwordComp;
    PlayerHPbarUI* m_pPlHPbarUiComp;

    // 敵キャラのポインタ
    std::vector<ZakoEnemyBase*> m_pZakoEnemyCompList;
    NightmareDragonEnemy* m_pNightmareDragonComp;

    ThirdPersonCameraController* m_pTPCameraComp;
    TransitionAnimator* m_pTransitionAnimatorComp;

    UnityChanMotion m_nowUnityChanMotion;

    bool m_extraAction; // 追加攻撃するかどうか
    int m_attackSeries;

    XMVECTOR m_evadeMoveVect;   // 回避時にどちらに動くかの保持

    // シーン遷移のトランジション実装のため
    bool m_isTransition;    // 画面遷移するかどうか
    SceneState m_sceneState;
    GAME_SCENES m_nextScene;    // Spaceを押したら、どのシーンにいくか
    void UpdateSceneState();    // シーン遷移時のアップデート処理

public:
    // GameComponent を介して継承されました
    virtual void InitAction() override;     // コンポーネント初期化時に呼ばれる処理
    virtual bool FrameAction() override;    // 毎フレーム呼ばれる処理　falseを返すとこのコンポーネントは終了し削除される
    virtual void FinishAction() override;   // 終了時に呼ばれる処理

    // 必要なポインタたちをセット
    void SetCurrentCamera(CameraComponent* cam)
    {
        m_currentCamera = cam;
    }

    void SetPlayerSwordComponent(PlayerWeaponSword* swordComp)
    {
        m_pSwordComp = swordComp;
    }

    void SetZakoEnemyComponentList(ZakoEnemyBase* zakoEnemyComp)
    {
        m_pZakoEnemyCompList.push_back(zakoEnemyComp);
    }

    void SetPlayerHPbarUIComponent(PlayerHPbarUI* plHpUiComp)
    {
        m_pPlHPbarUiComp = plHpUiComp;
    }

    void SetNightmareDragonEnemyComponent(NightmareDragonEnemy* dragonComp)
    {
        m_pNightmareDragonComp = dragonComp;
    }

    void SetThirdPersonCameraComponent(ThirdPersonCameraController* tpCameraComp)
    {
        m_pTPCameraComp = tpCameraComp;
    }

    void SetTransitionAnimatorComponent(TransitionAnimator* transitionAnimatorComp)
    {
        m_pTransitionAnimatorComp = transitionAnimatorComp;
    }

    // HPを設定
    void SetMaxPlHp(float maxHp)
    {
        m_maxPlHp = maxHp;
        m_nowPlHp = m_maxPlHp;
        m_plTotalDamage = 0;    // ここでついでに初期化
    }

    string GetTotalDamageStr()
    {
        float parcentage = m_plTotalDamage / m_maxPlHp;
        string parcentageStr = std::to_string(parcentage);

        // 小数第三位以下を切り捨て
        auto dot = parcentageStr.find(".");
        if (dot != std::string::npos)
        {
            parcentageStr = parcentageStr.substr(0, dot + 3);
        }
        parcentageStr += "%";

        return parcentageStr;
    }

    // ダメージ総量をそのまま取得
    float GetTotalDamageRate()
    {
        m_isTransition = true;  // Spaceキーで画面遷移をするように
        m_nextScene = GAME_SCENES::TITLE;   // 次のシーンをタイトルシーンに

        return m_plTotalDamage / m_maxPlHp;
    }

    // Unityちゃん本体のヒットリアクション
    virtual void HitReaction(GameObject* targetGo, HitAreaBase* hit) override;
};

