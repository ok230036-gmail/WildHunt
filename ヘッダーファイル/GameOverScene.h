#pragma once
#include "GameObject.h"
#include "SpriteCharacter.h"

#include "WildHuntEnum.h"
#include "TransitionAnimator.h"
#include "TitleScene.h"

#include <memory>

class GameOverScene :
    public GameComponent
{
private:
    std::unique_ptr<SpriteCharacter> m_gameOverImageSp;

    GAME_SCENES m_nextScene;    // Spaceを押したら、どのシーンにいくか

    bool m_isPlayGOSound;
    SceneState m_gameOverState = SceneState::SceneImage;
    TransitionAnimator* m_pTransitionAnimatorComp;  // TransitionAnimatorのポインタ

public:

    GameOverScene();

    void SetNextScene(GAME_SCENES nextSc);

    //  GameComponent を介して継承されました
    virtual void InitAction() override;     // コンポーネント初期化時に呼ばれる処理
    virtual bool FrameAction() override;    // 毎フレーム呼ばれる処理　falseを返すとこのコンポーネントは終了し削除される
    virtual void FinishAction() override;   // 終了時に呼ばれる処理

    // TransitionAnimatorのポインタをセット
    void SetTransitionAnimatorComponent(TransitionAnimator* transitionAnimator)
    {
        m_pTransitionAnimatorComp = transitionAnimator;
    }
};

