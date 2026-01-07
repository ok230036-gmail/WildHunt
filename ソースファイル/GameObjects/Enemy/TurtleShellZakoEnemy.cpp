#include "TurtleShellZakoEnemy.h"

#include "WildHuntScene.h"
#include "WildHuntEnum.h"

#include "WildHuntUIRender.h"

// InitActionの最初に呼ばれる処理
void TurtleShellZakoEnemy::FirstInitAction()
{
	m_fileName = L"./Resources/fbx/TurtleShellMesh.fbx";
	m_Fbxid = L"TurtleShellZakoEnemy";

	// 位置以外の初期設定はここで
	m_firstRot.x = 0.0f;
	m_firstRot.y = 90.0f;
	m_firstRot.z = 0.0f;
	m_firstScale.x = 0.01f;
	m_firstScale.y = 0.01f;
	m_firstScale.z = 0.01f;

	m_isKnockback = false;

	// 一番最初のStateはIdleで
	m_nowZakoEnemyState = ZakoEnemyState::Idle;
}

// FrameActionの最初に呼ばれる処理
void TurtleShellZakoEnemy::FirstFrameAction()
{

}

// InitActionの最後に呼ばれる処理
void TurtleShellZakoEnemy::ExtraInitAction()
{
	FBXCharacterData* endata = static_cast<FBXCharacterData*>(GetGameObject()->GetCharacterData());

	XMFLOAT3 scl = endata->GetScale();
	XMFLOAT3 min = endata->GetMainFbx()->GetFbxMin();
	XMFLOAT3 max = endata->GetMainFbx()->GetFbxMax();

	m_centerY = (max.y - min.y) * scl.y * 0.5f;

	m_turtleShellHit.SetRadius(m_centerY * 0.8f);	// だいたいの半径
	m_turtleShellHit.SetAttackType((UINT)HIT_ORDER::HIT_ENEMY_BODY, 1);

	m_turtleShellAttackHit.SetRadius(m_centerY * 0.8f);	// だいたいの半径
	m_turtleShellAttackHit.SetAttackType((UINT)HIT_ORDER::HIT_ENEMY_ATTACK, 1);

	endata->SetAnime(L"IdleNormal"); // 再生開始
}

// FrameActionの最後に呼ばれる処理
void TurtleShellZakoEnemy::ExtraFrameAction()
{
	FBXCharacterData* endata = static_cast<FBXCharacterData*>(GetGameObject()->GetCharacterData());
	XMFLOAT3 enPos = endata->GetPosition();
	XMFLOAT3 moveVect = {};

	// protectedのZakoEnemyStateを参照にアニメーション系統の処理
	if (m_nowZakoEnemyState == ZakoEnemyState::Idle)
	{
		endata->SetAnime(L"IdleNormal"); // 再生開始
	}
	else if (m_nowZakoEnemyState == ZakoEnemyState::Find)
	{	
		endata->SetAnime(L"Walk");
		moveVect.x = (m_nowPlayerPos.x - enPos.x) * 0.01f;
		moveVect.z = (m_nowPlayerPos.z - enPos.z) * 0.01f;

		m_forwardVect = moveVect;
	}
	else if (m_nowZakoEnemyState == ZakoEnemyState::Attack)
	{
		// 攻撃するぜ
		endata->SetAnime(L"Attack01"); 

		m_turtleShellAttackHit.SetCenter(enPos.x + m_forwardVect.x * 100.0f, enPos.y + m_centerY * 2.0f, enPos.z + m_forwardVect.z * 100.0f);
		MyAccessHub::GetMyGameEngine()->GetHitManager()->SetHitArea(this, &m_turtleShellAttackHit);

		if (endata->GetAnimeEnd())	// アニメーション終了で次
		{
			m_nowZakoEnemyState = ZakoEnemyState::Idle;
		}
	}
	else if(m_nowZakoEnemyState == ZakoEnemyState::Die)
	{
		endata->SetAnime(L"Die"); // 再生開始

		if (endata->GetAnimeEnd())	// アニメーション終了で次
		{
			m_pUIComp->ScoreUp();
			m_destroy = true;
		}
	}

	if (m_isKnockback)	// ノックバックがまだだったら
	{
		moveVect.x = m_knockbackVect.x;
		moveVect.z = m_knockbackVect.z;
		
		m_knockbackVect.x = m_knockbackVect.x * 0.95f;
		m_knockbackVect.z = m_knockbackVect.z * 0.95f;

		if (abs(m_knockbackVect.x) <= 0.01f || abs(m_knockbackVect.z) <= 0.01f)
		{
			m_isKnockback = false;

			m_knockbackVect.x = 0.0f;
			m_knockbackVect.z = 0.0f;
		}
	}

	// 地面との当たり判定の処理、地面に埋まらないように
	WildHuntScene* p_scene = static_cast<WildHuntScene*>(MyAccessHub::GetMyGameEngine()->GetSceneController());
	TerrainComponent* terCom = p_scene->GetTerrainComponent(0);
	XMFLOAT3 hitPos = {}; // 判定接触点（結果用）
	XMFLOAT3 hitNormal = {};// 接触ポリゴンの法線（結果用）
	XMFLOAT3 rayStart; // 線分開始点
	XMFLOAT3 rayEnd; // 線分終了点
	HitRayLine ray; // 線分判定（レイ）

	rayStart = endata->GetPosition();
	rayStart.y += 5.0f;

	enPos.x += moveVect.x;
	enPos.z += moveVect.z;

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

	endata->SetPosition(enPos.x, enPos.y, enPos.z);

	XMFLOAT3 pos = endata->GetPosition();

	m_turtleShellHit.SetCenter(pos.x, pos.y + m_centerY * 2.0f, pos.z);

	if (m_nowZakoEnemyState != ZakoEnemyState::Die)
	{
		MyAccessHub::GetMyGameEngine()->GetHitManager()->SetHitArea(this, &m_turtleShellHit);
	}
}

// ヒット時の処理
void TurtleShellZakoEnemy::HitReaction(GameObject* targetGo, HitAreaBase* hit)
{
	if (hit->GetHitType() == (UINT)HIT_ORDER::HIT_PLAYER_ATTACK)
	{
		m_nowZakoEnemyState = ZakoEnemyState::Die;

		m_isKnockback = true;

		FBXCharacterData* endata = static_cast<FBXCharacterData*>(GetGameObject()->GetCharacterData());
		XMFLOAT3 enPos = endata->GetPosition();

		m_knockbackVect.x = (enPos.x - m_nowPlayerPos.x) * 0.1f;
		m_knockbackVect.z = (enPos.z - m_nowPlayerPos.z) * 0.1f;
	}
}

// 追加で行うヒット時の処理
void TurtleShellZakoEnemy::ExtraHitReaction()
{
}
