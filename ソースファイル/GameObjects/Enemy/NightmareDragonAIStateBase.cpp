#include "NightmareDragonAIStateBase.h"

#include "WildHuntScene.h"
#include "WildHuntEnum.h"

#include "PlayTimerUI.h"

#include "NightmareDragonEnemy.h"

// Idleアニメーション処理のまとめ
// このステートに遷移した直後に一度だけ呼ばれる処理
void IdleNDAIState::Enter(FBXCharacterData* chData)
{

}

// ステート中の毎フレーム処理
void IdleNDAIState::Update(FBXCharacterData* chData)
{
	XMFLOAT3 enPos = chData->GetPosition();

	// 差分によって処理を変える。
	double dist = sqrt((enPos.x - m_nowPlayerPos.x) * (enPos.x - m_nowPlayerPos.x) + (enPos.z - m_nowPlayerPos.z) * (enPos.z - m_nowPlayerPos.z));
	
	// 距離が近いとRunのStateに
	if (dist <= 40.0f)
	{
		// 視界内にする処理は後で
		m_pNightmareDragonComp->ChangeNowDragonEnemyState(static_cast<UINT>(DragonEnemyState::Run));
	}
	else
	{
		chData->SetAnime(L"ND_Idle");
	}

}

// ステート終了時の後処理
void IdleNDAIState::Exit(FBXCharacterData* chData)
{
	
}

// walkアニメーション処理まとめ
// このステートに遷移した直後に一度だけ呼ばれる処理
void WalkNDAIState::Enter(FBXCharacterData* chData)
{
}

// ステート中の毎フレーム処理
void WalkNDAIState::Update(FBXCharacterData* chData)
{
}

// ステート終了時の後処理
void WalkNDAIState::Exit(FBXCharacterData* chData)
{
}

// ClawAttack攻撃アニメーション処理のまとめ
// このステートに遷移した直後に一度だけ呼ばれる処理
void ClawAttackNDAIState::Enter(FBXCharacterData* chData)
{
	attackCnt = 0;
}

// ステート中の毎フレーム処理
void ClawAttackNDAIState::Update(FBXCharacterData* chData)
{
	XMFLOAT3 pos = chData->GetPosition();

	m_pNightmareDragonComp->m_nightmareDragonAttackHit.SetCenter(pos.x + m_pNightmareDragonComp->m_forwardVect.x * 100.0f, pos.y + m_pNightmareDragonComp->m_centerY * 2.0f, pos.z + m_pNightmareDragonComp->m_forwardVect.z * 100.0f);

	attackCnt++;
	if(attackCnt > 80)
		MyAccessHub::GetMyGameEngine()->GetHitManager()->SetHitArea(m_pNightmareDragonComp, &m_pNightmareDragonComp->m_nightmareDragonAttackHit);

	chData->SetAnime(L"ND_ClawAttack");

	if (chData->GetAnimeEnd())
	{
		m_pNightmareDragonComp->ChangeNowDragonEnemyState(static_cast<UINT>(DragonEnemyState::Idle));
	}
	else
	{
		chData->SetAnime(L"ND_ClawAttack");
	}
}

// ステート終了時の後処理
void ClawAttackNDAIState::Exit(FBXCharacterData* chData)
{
	attackCnt = 0;
}

// HornAttack攻撃アニメーション処理のまとめ
// このステートに遷移した直後に一度だけ呼ばれる処理
void HornAttackNDAIState::Enter(FBXCharacterData* chData)
{
	attackCnt = 0;
}

// ステート中の毎フレーム処理
void HornAttackNDAIState::Update(FBXCharacterData* chData)
{
	XMFLOAT3 pos = chData->GetPosition();

	m_pNightmareDragonComp->m_nightmareDragonAttackHit.SetCenter(pos.x + m_pNightmareDragonComp->m_forwardVect.x * 100.0f, pos.y + m_pNightmareDragonComp->m_centerY * 2.0f, pos.z + m_pNightmareDragonComp->m_forwardVect.z * 100.0f);

	attackCnt++;
	if (attackCnt > 10)
		MyAccessHub::GetMyGameEngine()->GetHitManager()->SetHitArea(m_pNightmareDragonComp, &m_pNightmareDragonComp->m_nightmareDragonAttackHit);

	chData->SetAnime(L"ND_HornAttack");

	if (chData->GetAnimeEnd())
	{
		m_pNightmareDragonComp->ChangeNowDragonEnemyState(static_cast<UINT>(DragonEnemyState::Idle));
	}
	else
	{
		chData->SetAnime(L"ND_HornAttack");
	}
}

// ステート終了時の後処理
void HornAttackNDAIState::Exit(FBXCharacterData* chData)
{
	attackCnt = 0;
}

// Screamアニメーション処理のまとめ
// このステートに遷移した直後に一度だけ呼ばれる処理
void ScreamNDAIState::Enter(FBXCharacterData* chData)
{
}

// ステート中の毎フレーム処理
void ScreamNDAIState::Update(FBXCharacterData* chData)
{
	chData->SetAnime(L"ND_Scream");
}

// ステート終了時の後処理
void ScreamNDAIState::Exit(FBXCharacterData* chData)
{
}


// Dieアニメーション処理のまとめ
// このステートに遷移した直後に一度だけ呼ばれる処理
void DieNDAIState::Enter(FBXCharacterData* chData)
{

}

// ステート中の毎フレーム処理
void DieNDAIState::Update(FBXCharacterData* chData)
{
	chData->SetAnime(L"ND_Die");

	// アニメーションが終了したら、終了処理
	if (chData->GetAnimeEnd())
		m_pNightmareDragonComp->SetIsDestroy();
}

// ステート終了時の後処理
void DieNDAIState::Exit(FBXCharacterData* chData)
{

}

// Runアニメーション処理のまとめ
// このステートに遷移した直後に一度だけ呼ばれる処理
void RunNDAIState::Enter(FBXCharacterData* chData)
{

}

// ステート中の毎フレーム処理
void RunNDAIState::Update(FBXCharacterData* chData)
{
	chData->SetAnime(L"ND_Run");

	XMFLOAT3 enPos = chData->GetPosition();
	XMFLOAT3 moveVect = {};

	moveVect.x = (m_nowPlayerPos.x - enPos.x) * 0.01f;
	moveVect.z = (m_nowPlayerPos.z - enPos.z) * 0.01f;

	m_pNightmareDragonComp->m_forwardVect = moveVect;

	enPos.x += moveVect.x;
	enPos.z += moveVect.z;
	
	// 地面との当たり判定を取る処理
	WildHuntScene* p_scene = static_cast<WildHuntScene*>(MyAccessHub::GetMyGameEngine()->GetSceneController());
	TerrainComponent* terCom = p_scene->GetTerrainComponent(0);
	XMFLOAT3 hitPos = {}; // 判定接触点（結果用）
	XMFLOAT3 hitNormal = {};// 接触ポリゴンの法線（結果用）
	XMFLOAT3 rayStart; // 線分開始点
	XMFLOAT3 rayEnd; // 線分終了点
	HitRayLine ray; // 線分判定（レイ）

	rayStart = chData->GetPosition();
	rayStart.y += 5.0f;

	rayEnd = enPos;

	rayEnd.y -= 1.0f;

	ray.SetLine(rayStart, rayEnd, 0.0f);

	if (terCom->RayCastHit(ray, hitPos, hitNormal))
	{
		// X,Zをヒット位置に補正
		enPos.x = hitPos.x;
		enPos.y = hitPos.y;
		enPos.z = hitPos.z;
	}

	// 位置を変更
	chData->SetPosition(enPos.x, enPos.y, enPos.z);


	// プレイヤーの方を向く
	float charRad = atan2f(m_nowPlayerPos.x - enPos.x, m_nowPlayerPos.z - enPos.z);

	// 回転の設定値はDegree
	chData->SetRotation(0.0f, XMConvertToDegrees(charRad), 0.0f);

	// プレイヤーとの距離によって、次のステートを変更
	double dist = sqrt((enPos.x - m_nowPlayerPos.x) * (enPos.x - m_nowPlayerPos.x) + (enPos.z - m_nowPlayerPos.z) * (enPos.z - m_nowPlayerPos.z));
	
	if (dist <= 5.0f)
	{
		m_pNightmareDragonComp->ChangeNowDragonEnemyState(static_cast<UINT>(DragonEnemyState::HornAttack));
	}

	else if (dist <= 8.0f)
	{
		m_pNightmareDragonComp->ChangeNowDragonEnemyState(static_cast<UINT>(DragonEnemyState::ClawAttack));
	}
	else if (dist <= 30.0f)
	{
		m_pNightmareDragonComp->ChangeNowDragonEnemyState(static_cast<UINT>(DragonEnemyState::BreatheAttack));
	}

	else if (dist >= 55.0f)
	{
		m_pNightmareDragonComp->ChangeNowDragonEnemyState(static_cast<UINT>(DragonEnemyState::Idle));
	}
}

// ステート終了時の後処理
void RunNDAIState::Exit(FBXCharacterData* chData)
{
	
}

// GetHit攻撃アニメーション処理のまとめ
// このステートに遷移した直後に一度だけ呼ばれる処理
void GetHitNDAIState::Enter(FBXCharacterData* chData)
{
	
}

// ステート中の毎フレーム処理
void GetHitNDAIState::Update(FBXCharacterData* chData)
{
	if (chData->GetAnimeEnd())
	{
		m_pNightmareDragonComp->ChangeNowDragonEnemyState(static_cast<UINT>(DragonEnemyState::Idle));
	}
	else
	{
		chData->SetAnime(L"ND_GetHit");
	}
}

// ステート終了時の後処理
void GetHitNDAIState::Exit(FBXCharacterData* chData)
{

}

// ブレス攻撃のアニメーション処理のまとめ
// このステートに遷移した直後に一度だけ呼ばれる処理
void FireBreathAttackNDAIState::Enter(FBXCharacterData* chData)
{
	attackCnt = 0;
}

// ステート中の毎フレーム処理
void FireBreathAttackNDAIState::Update(FBXCharacterData* chData)
{
	XMFLOAT3 pos = chData->GetPosition();

	m_pNightmareDragonComp->m_nightmareDragonAttackHit.SetCenter(pos.x + m_pNightmareDragonComp->m_forwardVect.x * 100.0f, pos.y + m_pNightmareDragonComp->m_centerY * 2.0f, pos.z + m_pNightmareDragonComp->m_forwardVect.z * 100.0f);

	attackCnt++;
	if (attackCnt == 150)	// 一旦、固定値で
	{
		m_pNightmareDragonComp->DrawFireBreathe();
	}

	chData->SetAnime(L"ND_BreatheAttack");

	if (chData->GetAnimeEnd())
	{
		m_pNightmareDragonComp->ChangeNowDragonEnemyState(static_cast<UINT>(DragonEnemyState::Idle));
	}
}

// ステート終了時の後処理
void FireBreathAttackNDAIState::Exit(FBXCharacterData* chData)
{
}
