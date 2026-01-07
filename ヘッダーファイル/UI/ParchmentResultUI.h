#pragma once
#include "GameObject.h"
#include "SpriteCharacter.h"

class ParchmentResultUI :
    public GameComponent
{
private:
    std::unique_ptr<SpriteCharacter> m_parchmentResultSp;

    int m_frameCount = 0;	// フレームカウンタ
    int m_animeCount = 0;	// 現在のアニメパターン番号

    bool m_isAnimFlag = true;

public:
    //  GameComponent を介して継承されました
    void InitAction() override;     // コンポーネント初期化時に呼ばれる処理
    bool FrameAction() override;    // 毎フレーム呼ばれる処理　falseを返すとこのコンポーネントは終了し削除される
    void FinishAction() override;   // 終了時に呼ばれる処理

    // 攻撃エフェクトの位置更新
    void ChangeSwordEffectPos(float x, float y, float z)
    {
        if (!m_isAnimFlag)
        {
            m_isAnimFlag = true;
            m_animeCount = 0;
        }
    }
};

