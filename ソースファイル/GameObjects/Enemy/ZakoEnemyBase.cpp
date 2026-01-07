#include "ZakoEnemyBase.h"
#include "FBXCharacterData.h" // FBXCharacterDataをを使うので

#include "WildHuntScene.h"
#include "WildHuntEnum.h"

// コンポーネント初期化時に呼ばれる処理
void ZakoEnemyBase::InitAction()
{
	// InitActionの最初に呼ばれる処理
	FirstInitAction();

	// だいたい同じ処理はまとめよう
	if (m_fileName.size())
	{
		m_destroy = false;
		FBXCharacterData* enemydata =
			static_cast<FBXCharacterData*>(GetGameObject()->GetCharacterData());
		enemydata->SetGraphicsPipeLine(L"SkeltalLambert"); // スキンアニメアリLambert

		enemydata->SetMainFBX(m_Fbxid);

		if (m_firstPos.x)	// 全ての雑魚敵を同じ初期位置にしないため
			enemydata->SetPosition(m_firstPos.x, m_firstPos.y, m_firstPos.z);
		
		// 初期値設定
		enemydata->SetRotation(m_firstRot.x, m_firstRot.y, m_firstRot.z);
		enemydata->SetScale(m_firstScale.x, m_firstScale.y, m_firstScale.z);

		// InitActionの最後に呼ばれる処理
		ExtraInitAction();
	}
}

// 毎フレーム呼ばれる処理　falseを返すとこのコンポーネントは終了し削除される
bool ZakoEnemyBase::FrameAction()
{
	// FrameActionの最初に呼ばれる処理
	FirstFrameAction();

	if (!m_fileName.size())
	{
		return false;
	}
	
	FBXCharacterData* ecData = static_cast<FBXCharacterData*>(GetGameObject()->GetCharacterData());
	XMFLOAT3 throwVect = {};

	if (m_nowZakoEnemyState == ZakoEnemyState::Idle)
	{
		XMFLOAT3 enPos = ecData->GetPosition();

		// 差分によって処理を変える
		double dist = sqrt((enPos.x - m_nowPlayerPos.x) * (enPos.x - m_nowPlayerPos.x) + (enPos.z - m_nowPlayerPos.z) * (enPos.z - m_nowPlayerPos.z));
		
		if (dist <= 10.0f)
		{
			// 視界内にする処理は後で
			m_nowZakoEnemyState = ZakoEnemyState::Find;
		}

	}
	else if (m_nowZakoEnemyState == ZakoEnemyState::Find)
	{
		XMFLOAT3 enPos = ecData->GetPosition();

		// プレイヤーの方を向く
		float charRad = atan2f(m_nowPlayerPos.x - enPos.x, m_nowPlayerPos.z - enPos.z);

		// 回転の設定値はDegree
		ecData->SetRotation(0.0f, XMConvertToDegrees(charRad), 0.0f);

		double dist = sqrt((enPos.x - m_nowPlayerPos.x) * (enPos.x - m_nowPlayerPos.x) + (enPos.z - m_nowPlayerPos.z) * (enPos.z - m_nowPlayerPos.z));
		
		if (dist <= 2.0f)
		{
			// 視界内にする処理は後で
			m_nowZakoEnemyState = ZakoEnemyState::Attack;
		}
		else if (dist >= 5.0f)
		{
			m_nowZakoEnemyState = ZakoEnemyState::Idle;
		}
	}

	// FrameActionの最後に呼ばれる処理(正確には描画の前...)
	ExtraFrameAction();

	if (m_destroy)
		return false;
	

	// アニメをすすめる
	ecData->UpdateAnimation();

	// PipeLineに登録
	ecData->GetPipeline()->AddRenerObject(ecData);

	return true;
}

// 終了時に呼ばれる処理
void ZakoEnemyBase::FinishAction()
{
}
