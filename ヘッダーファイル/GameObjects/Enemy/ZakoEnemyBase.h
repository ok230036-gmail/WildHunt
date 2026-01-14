#pragma once
#include "GameObject.h"

#include "CameraComponent.h"
#include "FBXCharacterData.h"

// =====動く地形 対応
#include "TerrainComponent.h"
// =====動く地形 対応 END

#include "ParchmentResultUI.h"
#include <HitShapes.h>

enum class ZakoEnemyState
{
    Idle,
    Find,
    Attack, 
    Die,
};


class ZakoEnemyBase :
    public GameComponent
{
protected:
    bool m_destroy;
    wstring m_fileName;
    wstring m_Fbxid;
    XMFLOAT3 m_firstPos;
    XMFLOAT3 m_firstRot;
    XMFLOAT3 m_firstScale;

    XMFLOAT3 m_nowPlayerPos;

    ParchmentResultUI* m_pParchmentResultComp;
    ZakoEnemyState m_nowZakoEnemyState;

public:
    //  GameComponent を介して継承されました
    void InitAction() override;     // コンポーネント初期化時に呼ばれる処理
    bool FrameAction() override;    // 毎フレーム呼ばれる処理　falseを返すとこのコンポーネントは終了し削除される
    void FinishAction() override;   // 終了時に呼ばれる処理

    virtual void FirstInitAction() = 0;     // InitActionの最初に呼ばれる処理
    virtual void FirstFrameAction() = 0;    // FrameActionの最初に呼ばれる処理
    virtual void ExtraInitAction() = 0;	    // InitActionの最後に呼ばれる処理
    virtual void ExtraFrameAction() = 0;    // FrameActionの最後に呼ばれる処理

    // ヒット時リアクション処理
    virtual void ExtraHitReaction() = 0;

    // プレイヤーの現在位置を更新
    void ChangePlayerPos(float x, float y, float z)
    {
        m_nowPlayerPos.x = x;
        m_nowPlayerPos.y = y;
        m_nowPlayerPos.z = z;
    }

    void SetParchmentResultComp(ParchmentResultUI* pRComp)
    {
        m_pParchmentResultComp = pRComp;
    }
};

