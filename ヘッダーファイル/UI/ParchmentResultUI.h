#pragma once
#include "GameObject.h"
#include "SpriteCharacter.h"

#include "PlayTimerUI.h"


class UnityChanPlayer;

class ParchmentResultUI :
    public GameComponent
{
private:
    // フォントデータに登録されている文字一覧。左上から順番
    const char* m_chList = "0123456789+-*/!?ABCDEFGHIJKLMNOPQRSTUVWXYZ#$%&<>abcdefghijklmnopqrstuvwxyz\"()[]|:.";
    const char* m_chEnd;

    // 文字を表示するため
    std::unordered_map<char, XMFLOAT4> m_fontMap;
    std::vector<std::unique_ptr<SpriteCharacter>> m_clearTimeStrSp;     // クリアタイムの結果を表示する文字
    std::vector<std::unique_ptr<SpriteCharacter>> m_totalDamageStrSp;   // 被ダメージ総量の結果を表示する文字
    std::vector<std::unique_ptr<SpriteCharacter>> m_overallRankStrSp;   // 総合評価の結果を表示する文字
    std::vector<std::unique_ptr<SpriteCharacter>> m_pressSpaceToTitleStrSp;   // 総合評価の結果を表示する文字


    int m_spriteCount;

    int MakeSpriteString(std::vector<std::unique_ptr<SpriteCharacter>>& sprites, int startIndex, float ltX, float ltY, float width, float height, const char* str); // 座標、文字列、色とかを設定



    std::unique_ptr<SpriteCharacter> m_parchmentResultSp;   // 羊皮紙のスプライト
    std::unique_ptr<SpriteCharacter> m_clearTimeSp;   // 羊皮紙のスプライト
    std::unique_ptr<SpriteCharacter> m_totalDamageSp;   // 羊皮紙のスプライト
    std::unique_ptr<SpriteCharacter> m_overallRankSp;   // 羊皮紙のスプライト


    int m_frameCount = 0;	// フレームカウンタ
    int m_animeCount = 0;	// 現在のアニメパターン番号

    bool m_isAnimFlag = true;

    int m_defeatCnt;        // 撃破数
    int m_enemyCnt;         // 敵の数

    PlayTimerUI* m_pGameTimerUIComp;
    UnityChanPlayer* m_pUnityChanPlayerComp;

    void SetUpSprites(std::unique_ptr<SpriteCharacter>& sprite, const wchar_t* texId);
    const char* CalcOverallRank();  // 総合評価用の計算

public:
    // GameComponent を介して継承されました
    void InitAction() override;     // コンポーネント初期化時に呼ばれる処理
    bool FrameAction() override;    // 毎フレーム呼ばれる処理　falseを返すとこのコンポーネントは終了し削除される
    void FinishAction() override;   // 終了時に呼ばれる処理

    // 結果発表
    void AnnounceResult()
    {
        if (!m_isAnimFlag)
        {
            m_isAnimFlag = true;
            m_animeCount = 0;
        }
    }

    // 敵数を保存
    void SetEnemyCnt(int cnt)
    {
        m_enemyCnt = cnt;
    }

    // 敵を撃破したよ
    void DefeatEnemy()
    {
        m_defeatCnt++;
    }

    // ゲームタイマーのコンポーネントを設定
    void SetGameTimerComponent(PlayTimerUI* gameTimerComp)
    {
        m_pGameTimerUIComp = gameTimerComp;
    }

    // ユニティちゃんのコンポーネントを設定
    void SetUnityChanPlayerComponent(UnityChanPlayer* unityChanPlayerComp)
    {
        m_pUnityChanPlayerComp = unityChanPlayerComp;
    }
};

