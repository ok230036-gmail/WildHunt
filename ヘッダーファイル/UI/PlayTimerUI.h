#pragma once
#include "GameObject.h"
#include "SpriteCharacter.h"

#include <vector>
#include <unordered_map>

#include <memory>

class PlayTimerUI :
    public GameComponent
{
private:
    // フォントデータに登録されている文字一覧。左上から順番
    const char* m_chList = "0123456789+-*/!?ABCDEFGHIJKLMNOPQRSTUVWXYZ#$%&<>abcdefghijklmnopqrstuvwxyz\"()[]|:.";
    const char* m_chEnd;

    int m_timer;
    bool m_isStopTime;

    std::unordered_map<char, XMFLOAT4> m_fontMap;
    std::vector<std::unique_ptr<SpriteCharacter>> m_sprites;

    int m_spriteCount;

    int MakeSpriteString(int startIndex, float ltX, float ltY, float width, float height, const char* str);

public:
    // GameComponent を介して継承されました
    virtual void InitAction() override;     // コンポーネント初期化時に呼ばれる処理
    virtual bool FrameAction() override;    // 毎フレーム呼ばれる処理　falseを返すとこのコンポーネントは終了し削除される
    virtual void FinishAction() override;   // 終了時に呼ばれる処理

    // ゲームタイムの文字列を取得
    string GetGameTimeStr()
    {
        StopTimer();    // UIの時間停止

        int minutes = m_timer / 3600;				// 分
        int seconds = (m_timer % 3600) / 60;		// 秒
        string scoreStr = std::to_string(minutes) + ":" + (seconds < 10 ? "0" : "") + std::to_string(seconds);

        return scoreStr;
    }

    // 総合評価用のクリアタイムを取得
    int GetGameTimeSeconds()
    {
        StopTimer();    // UIの時間停止、ここで呼び出すのは良くないけど、しゃあなし

        return m_timer / 60;    // 秒単位にする
    }

    // タイマーを停止するよ
    void StopTimer()
    {
        m_isStopTime = true;
    }
};

