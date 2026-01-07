#pragma once
#include "GameObject.h"
#include "SpriteCharacter.h"

#include "CameraComponent.h"

class SwordHit2DEffect :
    public GameComponent
{
private:
    std::unique_ptr<SpriteCharacter> m_hitEffSp;
    CameraComponent* m_camera;

    int m_frameCount = 0;	// フレームカウンタ
    int m_animeCount = 0;	// 現在のアニメパターン番号

    bool m_isEffFlag;
    XMFLOAT3 m_hitPos;  // 攻撃が当たった位置

public:
    //  GameComponent を介して継承されました
    void InitAction() override;     // コンポーネント初期化時に呼ばれる処理
    bool FrameAction() override;    // 毎フレーム呼ばれる処理　falseを返すとこのコンポーネントは終了し削除される
    void FinishAction() override;   // 終了時に呼ばれる処理

    // カメラのポインタをセット
    void SetCameraComponent(CameraComponent* CameraComp)
    {
        m_camera = CameraComp;
    }

    // 剣のヒットエフェクトの位置更新
    void ChangeSwordEffectPos(float x, float y, float z)
    {
        if (!m_isEffFlag)
        {
            m_hitPos.x = x;
            m_hitPos.y = y;
            m_hitPos.z = z;

            m_isEffFlag = true;
        }  
        m_animeCount = 0;
    }
};

