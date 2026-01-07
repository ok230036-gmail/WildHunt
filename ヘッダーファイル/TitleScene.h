#pragma once
#include "GameObject.h"
#include "SpriteCharacter.h"

#include "WildHuntEnum.h"
#include "TransitionAnimator.h"

#include <memory>

enum class TitleState   // TitleSceneの状態
{
    TitleImage,
    Transition,
    Loading,
};

class TitleScene :
    public GameComponent
{
private:
    std::unique_ptr<SpriteCharacter> m_bgSp;
    std::unique_ptr<SpriteCharacter> m_titleImageSp;

    GAME_SCENES m_nextScene;    // Spaceを押したら、どのシーンにいくか

    TitleState m_titleState = TitleState::TitleImage;
    TransitionAnimator* m_pTransitionAnimatorComp;  // TransitionAnimatorのポインタ

public:
    TitleScene();

    void SetBGColor(float r, float g, float b);
    void SetImagePosition(float pxY);
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

