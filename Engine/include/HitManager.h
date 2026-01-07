#pragma once
#include <Windows.h>
#include <DirectXMath.h>
#include <memory>
#include <vector>
#include <list>
#include "GameObject.h"

#include "HitShapes.h"

using namespace DirectX;
using namespace std;

class GameComponent;    // 前方宣言

class HitManager
{
private:
    class HitStructure	// クラス内クラス
    {
    private:
		HitAreaBase* m_pHitArea;                // 判定領域データ
        GameComponent* m_pGameComponent;        // 判定を設定したGameObject
    public:
        HitStructure(GameComponent* cmp, HitAreaBase* hitArea)
        {
            m_pHitArea = hitArea;
            m_pGameComponent = cmp;
        }

        GameComponent* GetGameComponent();          // 設定したGameObjectを取得
		HitAreaBase* GetHitArea();                  // 登録してある判定データ本体を取得
    };

	list<DirectX::XMUINT2> m_hitOrders;
	
    UINT m_numOfHitTypes = 0;
    vector<unique_ptr<list<HitStructure*>>> m_hitArray;

    void FlushHitList(list<HitStructure*>* p_hitlist);  // 判定保存用listのクリア

    XMFLOAT3    m_lastHitPos;

public:
	void InitHitList(UINT hitTypes);
	void SetHitOrder(UINT atk, UINT def);

    void RefreshHitSystem();                            // ヒットシステムデータクリア　フレーム開始時に実行する。
    void SetHitArea(GameComponent * cmp, HitAreaBase* box);  // 当たり判定システムに判定データを登録する

    void HitFrameAction();                              // 判定一斉処理　登録順の前後で結果が変わらないようにしている

    bool IsHit(HitAreaBase* atkHit, HitAreaBase* defHit);

    XMFLOAT3 GetLastHitPoint()
    {
        return m_lastHitPos;
    }
};
