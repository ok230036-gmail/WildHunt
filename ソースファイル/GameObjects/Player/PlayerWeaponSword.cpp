#include "PlayerWeaponSword.h"
#include "FBXCharacterData.h" // FBXCharacterDataをを使うので

#include "WildHuntScene.h"
#include "WildHuntEnum.h"

// コンポーネント初期化時に呼ばれる処理
void PlayerWeaponSword::InitAction()
{
	FBXCharacterData* weapondata =
		static_cast<FBXCharacterData*>(GetGameObject()->GetCharacterData());

	// weapondata->SetGraphicsPipeLine(L"StaticFBX"); // スキンアニメ無しFBX
	weapondata->SetGraphicsPipeLine(L"StaticPhong"); // スキンアニメ無しFBX

	weapondata->SetMainFBX(L"WeaponSword");
	// weapondata->SetMainFBX(L"./Resources/fbx/Sword12_FBX_0706.fbx", L"WeaponSword");

	// =======Change Scene
	/*weapondata->GetMainFbx()->SetMeshUniqueFlag(true, true);
	weapondata->GetMainFbx()->SetTextureUniqueFlag(true);*/
	// =======Change Scene End

	weapondata->SetPosition(-7.5f, 0.0f, -0.5f); // 初期値設定
	weapondata->SetRotation(20.0f, 0.0f, 20.0f); // 初期値設定
	weapondata->SetScale(1.0f, 1.0f, 1.0f);

	XMFLOAT3 scl = weapondata->GetScale();
	XMFLOAT3 min = weapondata->GetMainFbx()->GetFbxMin();
	XMFLOAT3 max = weapondata->GetMainFbx()->GetFbxMax();

	m_centerX = (max.x - min.z) * scl.x * 0.05f;
 	m_centerY = (max.y - min.y) * scl.y * 0.05f;	// 0.01 scale
	m_centerZ = (max.z - min.z) * scl.z * 0.05f;

	m_swordHit.SetRadius(m_centerY * 0.2f);	// かなり適当な半径
	m_swordHit.SetAttackType((UINT)HIT_ORDER::HIT_PLAYER_ATTACK, 1);
}

// 毎フレーム呼ばれる処理　falseを返すとこのコンポーネントは終了し削除される
bool PlayerWeaponSword::FrameAction()
{
	FBXCharacterData* wpData = static_cast<FBXCharacterData*>(GetGameObject()->GetCharacterData());

	// m_nowPlayerHandDataは右手ノードの位置を示すアフィン変換マトリクス(アニメの変形マトリクス)
	XMFLOAT3 wp_pos = wpData->GetPosition();
	XMFLOAT3 wp_rot = wpData->GetRotation();
	XMFLOAT3 wp_scale = wpData->GetScale();
	XMMATRIX wp_rot_x = XMMatrixRotationX(XMConvertToRadians(wp_rot.x));
	XMMATRIX wp_rot_y = XMMatrixRotationY(XMConvertToRadians(wp_rot.y));
	XMMATRIX wp_rot_z = XMMatrixRotationZ(XMConvertToRadians(wp_rot.z));
	XMMATRIX wppos_mat = XMMatrixTranslation(wp_pos.x, wp_pos.y, wp_pos.z);
	XMMATRIX wpscale_mat = XMMatrixScaling(wp_scale.x, wp_scale.y, wp_scale.z);
	XMMATRIX wpMat = wpscale_mat * wp_rot_x * wp_rot_z * wp_rot_y * wppos_mat * m_nowPlayerHandData;

	// アフィン変換行列作成
	XMMATRIX AffineMat;
	XMMATRIX translate = XMMatrixTranslation(m_nowPlayerPos.x, m_nowPlayerPos.y, m_nowPlayerPos.z);
	XMMATRIX rotate_x = XMMatrixRotationX(XMConvertToRadians(m_nowPlayerRotate.x));
	XMMATRIX rotate_y = XMMatrixRotationY(XMConvertToRadians(m_nowPlayerRotate.y));
	XMMATRIX rotate_z = XMMatrixRotationZ(XMConvertToRadians(m_nowPlayerRotate.z));
	XMMATRIX scale_mat = XMMatrixScaling(m_nowPlayerScale.x, m_nowPlayerScale.y, m_nowPlayerScale.z);

	AffineMat = scale_mat * rotate_z * rotate_x * rotate_y * translate;

	XMMATRIX xmat = wpMat * AffineMat;

	// xmat = XMMatrixTranspose(xmat); // パイプラインで転置してるから
	wpData->SetWorldMatrix(xmat);


	// wpData->SetRotation(x, y, z); // Rotation設定

	XMFLOAT3 pos = wpData->GetPosition();

	
	m_swordHit.SetCenter(m_nowPlayerPos.x, m_nowPlayerPos.y + 0.5f, m_nowPlayerPos.z);
	
	if (m_nowWeaponAttack)
	{
		MyAccessHub::GetMyGameEngine()->GetHitManager()->SetHitArea(this, &m_swordHit);
	}
	

	wpData->GetPipeline()->AddRenerObject(wpData);	// PipeLineに登録
    return true;
}

// 終了時に呼ばれる処理
void PlayerWeaponSword::FinishAction()
{
}

// ヒット時の処理
void PlayerWeaponSword::HitReaction(GameObject* targetGo, HitAreaBase* hit)
{
	// 当たった対象がHIT_ENEMY_BODYかつ、現在が攻撃判定の場合
	if (hit->GetHitType() == (UINT)HIT_ORDER::HIT_ENEMY_BODY && m_nowWeaponAttack)
	{
		// カメラを揺らしましょう
		m_pTPCameraComp->CameraShakeFlag();
		m_nowWeaponAttack = false;	// 攻撃判定オフ
		
		XMFLOAT3 targetPos = targetGo->GetCharacterData()->GetPosition();
		MyGameEngine* pEngine = MyAccessHub::GetMyGameEngine();

		// ヒット音を鳴らしましょう
		pEngine->GetSoundManager()->Play(5);

		m_pSwordHit2DEffectComp->ChangeSwordEffectPos((targetPos.x + m_nowPlayerPos.x) * 0.5f, targetPos.y,(targetPos.z + m_nowPlayerPos.z) * 0.5f);
	}
}

