#include "NightmareDragonEnemy.h"

#include "WildHuntScene.h"
#include "WildHuntEnum.h"

#include "PlayTimerUI.h"

// コンポーネント初期化時に呼ばれる処理
void NightmareDragonEnemy::InitAction()
{
	FBXCharacterData* endata =
		static_cast<FBXCharacterData*>(GetGameObject()->GetCharacterData());

	endata->SetGraphicsPipeLine(L"SkeltalLambert");		// スキンアニメアリLambert
	endata->SetMainFBX(L"NightmareDragon");

	endata->SetPosition(50.0f, 0.0f, -30.0f);	// 初期値設定
	endata->SetRotation(0.0f, 0.0f, 0.0f);		// 初期値設定
	endata->SetScale(0.01f, 0.01f, 0.01f);		// ドラゴンがデカいので、0.01f

	// 一定の当たり判定を取っておく
	XMFLOAT3 scl = endata->GetScale();
	XMFLOAT3 min = endata->GetMainFbx()->GetFbxMin();
	XMFLOAT3 max = endata->GetMainFbx()->GetFbxMax();

	m_centerY = (max.y - min.y) * scl.y * 0.5f;	// 0.01 scale

	m_nightmareDragonHit.SetRadius(m_centerY * 2.5f);	// だいたいの半径
	m_nightmareDragonHit.SetAttackType((UINT)HIT_ORDER::HIT_ENEMY_BODY, 0);

	m_nightmareDragonAttackHit.SetRadius(m_centerY * 2.0f);	// だいたいの半径
	m_nightmareDragonAttackHit.SetAttackType((UINT)HIT_ORDER::HIT_ENEMY_ATTACK, 1);

	// NightmareDragonCompでそれぞれの子クラスにポインタを渡す。
	m_idleNDState.SetNightmareDragonComp(this); 
	m_clawAttackNDState.SetNightmareDragonComp(this);
	m_hornAttackNDState.SetNightmareDragonComp(this);
	m_screamNDState.SetNightmareDragonComp(this);
	m_dieNDState.SetNightmareDragonComp(this);
	m_runNDState.SetNightmareDragonComp(this);
	m_getHitNDState.SetNightmareDragonComp(this);
	m_breatheAttackNDState.SetNightmareDragonComp(this);

	// ステートの初期設定
	m_nowDragonEnemyState = static_cast<UINT>(DragonEnemyState::Idle);

	// ポリモーフィズムの初期設定
	m_currentNDAIState = &m_idleNDState;

	// 最初はisDestroyをfalseに
	m_isDestroy = false;

	endata->SetAnime(L"ND_Idle"); // アニメーション再生
}

// 毎フレーム呼ばれる処理　falseを返すとこのコンポーネントは終了し削除される
bool NightmareDragonEnemy::FrameAction()
{
	FBXCharacterData* enData = static_cast<FBXCharacterData*>(GetGameObject()->GetCharacterData());
	XMFLOAT3 enPos = enData->GetPosition();
	
	// ドラゴン破壊チェック
	if (m_isDestroy)
	{
		m_pParchmentResultComp->DefeatEnemy();
		return false;
	}

	// プレイヤーの位置データを渡す
	m_currentNDAIState->SetPlayerPos(m_nowPlayerPos.x, m_nowPlayerPos.y, m_nowPlayerPos.z);

	// 前回ステートと違うステートの時に実行
	if (m_prevNDAIState != m_currentNDAIState)
	{
		if (m_prevNDAIState != nullptr)
		{
			m_prevNDAIState->Exit(enData);
		}

		m_prevNDAIState = m_currentNDAIState;
		m_currentNDAIState->Enter(enData);
	}

	// ステートパターンの毎フレーム処理を呼び出す
	m_currentNDAIState->Update(enData);

	XMFLOAT3 pos = enData->GetPosition();

	// ブレス用の頭と首のデータを渡す
	const char* boneName = "Head";
	XMMATRIX xmat = enData->GetAnimatedMatrixByBoneName(boneName);
	m_pFireBreathComp->changeDragonHeadData(xmat);

	boneName = "Neck02";
	xmat = enData->GetAnimatedMatrixByBoneName(boneName);
	m_pFireBreathComp->changeDragonNeckData(xmat);

	// 次は、ブレスにドラゴンの位置情報等を渡す
	m_pFireBreathComp->changeDragonPos(pos.x, pos.y, pos.z);

	XMFLOAT3 rot = enData->GetRotation();
	m_pFireBreathComp->changeDragonRotate(rot.x, rot.y, rot.z);

	XMFLOAT3 scale = enData->GetScale();
	m_pFireBreathComp->changeDragonScale(scale.x, scale.y, scale.z);

	// 現在位置の更新
	m_nightmareDragonHit.SetCenter(pos.x, pos.y + m_centerY * 2.0f, pos.z);

	MyAccessHub::GetMyGameEngine()->GetHitManager()->SetHitArea(this, &m_nightmareDragonHit);

	// アニメをすすめる
	enData->UpdateAnimation();

	// PipeLineに登録
	enData->GetPipeline()->AddRenerObject(enData);

	return true;
}

// 終了時に呼ばれる処理
void NightmareDragonEnemy::FinishAction()
{
}

// ヒット時の処理
void NightmareDragonEnemy::HitReaction(GameObject* targetGo, HitAreaBase* hit)
{
	if (hit->GetHitType() == (UINT)HIT_ORDER::HIT_PLAYER_ATTACK)
	{
		m_nowDragonHp++;
		if (m_nowDragonHp > 9)
		{
			ChangeNowDragonEnemyState(static_cast<UINT>(DragonEnemyState::Die));
		}
		// changeNowDragonEnemyState(static_cast<UINT>(DragonEnemyState::Die));
		// m_nowDragonEnemyState = static_cast<UINT>(DragonEnemyState::Die);
	}
}

// ステートパターンを変更する処理
void NightmareDragonEnemy::ResolveByNowState()
{
	// ポリフォーリズムと構造体でアニメーション処理やAI部分を分かりやすく
	// 実際の処理はNightmareDragonAnimationControllerBaseで！
	// switch文の前に、今のポインタを保存
	m_prevNDAIState = m_currentNDAIState;

	// m_nowDragonEnemyStateからポリモーフィズムの真髄
	switch (m_nowDragonEnemyState)
	{
	case static_cast<UINT>(DragonEnemyState::Idle):
		m_currentNDAIState = &m_idleNDState;
		break;

	case static_cast<UINT>(DragonEnemyState::Walk):
		m_currentNDAIState = &m_walkNDState;
		break;

	case static_cast<UINT>(DragonEnemyState::Run):
		m_currentNDAIState = &m_runNDState;
		break;

	case static_cast<UINT>(DragonEnemyState::Scream):
		m_currentNDAIState = &m_screamNDState;
		break;

	case static_cast<UINT>(DragonEnemyState::ClawAttack):
		m_currentNDAIState = &m_clawAttackNDState;
		break;

	case static_cast<UINT>(DragonEnemyState::HornAttack):
		m_currentNDAIState = &m_hornAttackNDState;
		break;

	case static_cast<UINT>(DragonEnemyState::Die):
		m_currentNDAIState = &m_dieNDState;
		break;

	case static_cast<UINT>(DragonEnemyState::GetHit):
		m_currentNDAIState = &m_getHitNDState;
		break;

	case static_cast<UINT>(DragonEnemyState::BreatheAttack):
		m_currentNDAIState = &m_breatheAttackNDState;
		break;

	default:
		break;
	}
}