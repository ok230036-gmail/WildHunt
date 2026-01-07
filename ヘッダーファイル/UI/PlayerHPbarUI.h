#pragma once
#include "GameObject.h"
#include "SpriteCharacter.h"

class PlayerHPbarUI :
    public GameComponent
{
private:
    std::unique_ptr<SpriteCharacter> m_bgSp;    // HPバーの背景
    std::unique_ptr<SpriteCharacter> m_redSp;   // HPバーの赤色ゲージ
    std::unique_ptr<SpriteCharacter> m_greenSp; // HPバーの緑色ゲージ

    bool m_hpChangeFlg;     // HPが変更したときのフラグ
    bool m_isRedDecrease;   // 赤色ゲージがだんだん減っているとき

    float m_maxPlHp;        // プレイヤーの最大HP
    float m_nowPlHp;        // プレイヤーの現在HP

    float m_nowUvHp;        // HPバー表示のためのUV微調整に使用
    float m_redUvHp;        // 赤いHPバー表示のためのUV微調整に使用

    int ConvertHpToUvValue(float nowHp);
public:
    //  GameComponent を介して継承されました
    virtual void InitAction() override;     // コンポーネント初期化時に呼ばれる処理
    virtual bool FrameAction() override;    // 毎フレーム呼ばれる処理　falseを返すとこのコンポーネントは終了し削除される
    virtual void FinishAction() override;   // 終了時に呼ばれる処理

    // HP変更
    void ChangeHp(float hp)
    {
        m_nowPlHp = hp;
        m_hpChangeFlg = true;
    }

    // 最大HPを設定
    void SetMaxPlHp(float maxHp)
    {
        m_maxPlHp = maxHp;
        m_nowPlHp = m_maxPlHp;
    }
};

