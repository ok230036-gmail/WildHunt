#pragma once
#include "GameObject.h"
#include "SpriteCharacter.h"

enum class WipeMode
{
    WipeIn,
    WipeOut,
};

class TransitionAnimator :
    public GameComponent
{
private:
    std::unique_ptr<SpriteCharacter> m_transitionAnimSp;

    int m_frameCount = 0;	// フレームカウンタ
    int m_animeCount = 11;	// 現在のアニメパターン番号

    bool m_isAnimFlag;      // アニメーションのフラグ
    WipeMode m_nowWipeMode; // 次の画面遷移がWipeInかWipeOutか

    // 次の画面遷移をWipeInに
    void SetWipeIn()
    {
        m_nowWipeMode = WipeMode::WipeIn;
        m_animeCount = 12;
    }
    
    // 次の画面遷移をWipeOutに
    void SetWipeOut()
    {
        m_nowWipeMode = WipeMode::WipeOut;
        m_animeCount = 0;
    }
public:
    // GameComponent を介して継承されました
    void InitAction() override;     // コンポーネント初期化時に呼ばれる処理
    bool FrameAction() override;    // 毎フレーム呼ばれる処理　falseを返すとこのコンポーネントは終了し削除される
    void FinishAction() override;   // 終了時に呼ばれる処理

    // ワイプモードを変更
    void SetWipeMode(WipeMode wipeMode)
    {
        switch (wipeMode)
        {
        case WipeMode::WipeIn:
            SetWipeIn();
            break;
        case WipeMode::WipeOut:
            SetWipeOut();
            break;
        default:
            break;
        }
    }

    // 画面遷移のトランジションを開始
    void PlayTransition()
    {
        if (!m_isAnimFlag)
        {
            m_isAnimFlag = true;
        }
    }

    // トランジションが終了したかどうか
    bool IsTransitionFinished()
    {
        return !m_isAnimFlag;
    }
};

