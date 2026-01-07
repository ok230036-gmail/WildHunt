#pragma once
#include "GameObject.h"
#include "SpriteCharacter.h"

#include <vector>
#include <unordered_map>

#include <memory>

class WildHuntUIRender :
    public GameComponent
{
private:
    // フォントデータに登録されている文字一覧。左上から順番
    const char* m_chList = "0123456789+-*/!?ABCDEFGHIJKLMNOPQRSTUVWXYZ#$%&<>abcdefghijklmnopqrstuvwxyz\"()[]|:";
    const char* m_chEnd;

    int m_score;
    int m_timer;

    std::unordered_map<char, XMFLOAT4> m_fontMap;
    std::vector<std::unique_ptr<SpriteCharacter>> m_sprites;

    int m_spriteCount;

    int MakeSpriteString(int startIndex, float ltX, float ltY, float width, float height, const char* str);

public:
    //  GameComponent を介して継承されました
    virtual void InitAction() override;     // コンポーネント初期化時に呼ばれる処理
    virtual bool FrameAction() override;    // 毎フレーム呼ばれる処理　falseを返すとこのコンポーネントは終了し削除される
    virtual void FinishAction() override;   // 終了時に呼ばれる処理

    void ScoreUp()
    {
        m_score++;
    }
};

