#pragma once
#include "GameObject.h"
#include "SpriteCharacter.h"

#include "CameraComponent.h"

class FireBreatheEffect :
    public GameComponent
{
private:
    // ドラゴンの現在情報
    XMFLOAT3 m_nowDragonPos;
    XMFLOAT3 m_nowDragonScale;
    XMFLOAT3 m_nowDragonRotate;

    // m_nowPlayerHeadDataはドラゴンの頭を示すアフィン変換マトリクス(アニメの変形マトリクス)
    XMMATRIX m_dragonHeadData;
    XMMATRIX m_dragonNeckData;

    std::unique_ptr<SpriteCharacter> m_breathEffSp;
    CameraComponent* m_pCameraComp;  // カメラのポインタ

    HitRayLine m_fireBreatheHit;    // ドラゴンの当たり判定

    int m_animeCount = 0;   // 現在のアニメパターン番号
    bool m_isEffFlag;       // エフェクトを出すフラグ

public:
    // GameComponent を介して継承されました
    void InitAction() override;         // コンポーネント初期化時に呼ばれる処理
    bool FrameAction() override;        // 毎フレーム呼ばれる処理　falseを返すとこのコンポーネントは終了し削除される
    void FinishAction() override;       // 終了時に呼ばれる処理

    // ワールドマトリクス作成用メソッド
    XMMATRIX CreateWorldMatrix(XMMATRIX matData);

    void SetCameraComponent(CameraComponent* CameraComp)
    {
        m_pCameraComp = CameraComp;
    }

    // 炎のブレス発動フラグ
    void DrawFireBreathe()
    {
        if (!m_isEffFlag)
        {
            m_isEffFlag = true;
        }
    }

    // ドラゴンの頭のアニメーションマトリクスデータ更新
    void changeDragonHeadData(XMMATRIX dragonHeadData)
    {
        m_dragonHeadData = dragonHeadData;
    }

    // ドラゴンの首のアニメーションマトリクスデータ更新
    void changeDragonNeckData(XMMATRIX dragonNeckData)
    {
        m_dragonNeckData = dragonNeckData;
    }

    // 最新のドラゴンの状態を保持
    void changeDragonPos(float x, float y, float z)
    {
        m_nowDragonPos.x = x;
        m_nowDragonPos.y = y;
        m_nowDragonPos.z = z;
    }

    void changeDragonScale(float x, float y, float z)
    {
        m_nowDragonScale.x = x;
        m_nowDragonScale.y = y;
        m_nowDragonScale.z = z;
    }

    void changeDragonRotate(float x, float y, float z)
    {
        m_nowDragonRotate.x = x;
        m_nowDragonRotate.y = y;
        m_nowDragonRotate.z = z;
    }

    // ヒット時の処理
    virtual void HitReaction(GameObject* targetGo, HitAreaBase* hit) override;
};

