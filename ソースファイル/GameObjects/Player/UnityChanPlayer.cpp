#include "UnityChanPlayer.h"
#include "FBXCharacterData.h"	// FBXCharacterDataを使うので

#include "WildHuntScene.h"
#include "KeyBindComponent.h"

#include "WildHuntEnum.h"

#include "FBXDataContainerSystem.h"

#define MOVE_3D (0)				// (0)にすると2Dモード

// SceneStateをswitch文
void UnityChanPlayer::UpdateSceneState()
{
	MyGameEngine* engine = MyAccessHub::GetMyGameEngine();

	WildHuntScene* scene = static_cast<WildHuntScene*>(engine->GetSceneController());
	KeyBindComponent* keyBind = static_cast<KeyBindComponent*>(scene->GetKeyComponent());

	switch (m_sceneState)
	{
	case SceneState::SceneImage:
		if (keyBind->GetCurrentInputState(InputManager::BUTTON_STATE::BUTTON_DOWN, KeyBindComponent::BUTTON_IDS::BTN_JUMP))
		{
			// Spaceキーが押されたら、画面遷移のトランジションをする
			m_pTransitionAnimatorComp->SetWipeMode(WipeMode::WipeOut);
			m_pTransitionAnimatorComp->PlayTransition();

			m_sceneState = SceneState::Transition;
		}
		break;
	case SceneState::Transition:
		if (m_pTransitionAnimatorComp->IsTransitionFinished())
		{
			// 画面遷移のトランジションが終了したら、Loading処理へ
			m_sceneState = SceneState::Loading;
		}

		break;
	case SceneState::Loading:

		// シーン切り替え呼び出し
		MyAccessHub::GetMyGameEngine()->GetSceneController()->OrderNextScene((UINT)m_nextScene);
		break;
	}
}

// コンポーネント初期化時に呼ばれる処理
void UnityChanPlayer::InitAction()
{
	// UnityChanの初期値設定
	// FBXCharacterDataはGameScene側で追加する時にGameObjectへ引数でセットする
	FBXCharacterData* chdata = static_cast<FBXCharacterData*>(GetGameObject()->GetCharacterData());

	// chdata->SetGraphicsPipeLine(L"AnimationFBX");	// スキンアニメ有りFBX
	// chdata->SetGraphicsPipeLine(L"SkeltalLambert");	// スキンアニメ有りLambert
	chdata->SetGraphicsPipeLine(L"SkeltalPhong");		// スキンアニメ有りPhong
	// chdata->SetGraphicsPipeLine(L"SkeltalBlinn");	// スキンアニメ有りBlinn
	// chdata->SetGraphicsPipeLine(L"SkeltalToon");		// スキンアニメ有りToon

	chdata->SetMainFBX(L"UnityChan");

	chdata->SetScale(0.01f, 0.01f, 0.01f);	// 元モデルがかなり大きいので0.01f
	chdata->SetPosition(0.0f, 0.0f, 0.0f);

	// Unityちゃんの判定設定を作る。
	XMFLOAT3 min = chdata->GetMainFbx()->GetFbxMin();	// モデル全体の一番値の小さい座標	
	XMFLOAT3 max = chdata->GetMainFbx()->GetFbxMax();	// モデル全体の一番値の大きい座標

	float headY = (max.y - min.y) * 0.8f;								// Unityちゃんの頭座標をなんとなくで出す。
	m_unityChanHeadHeight = headY * chdata->GetScale().y;				// スケールかけて空間中の高さに
	
	// 坂の上り下り用に、足元の「抜ける」高さを設定。
	m_walkableHeight = (max.y - min.y) * 0.25f * chdata->GetScale().y;

	chdata->SetAnime(L"WAIT00");	// 再生開始

	// Unityちゃん本体のヒット判定作成
	XMFLOAT3 scl = chdata->GetScale();
	m_hitHeight = (max.y - min.y) * scl.y;	// scale
	bodyColl.SetHeight(m_hitHeight * 0.9f);
	bodyColl.SetRadius(m_hitHeight * 0.55f * 0.5f);	// 直径がメッシュ幅の55%で、さらに半径なのでもう半分
	bodyColl.SetAttackType(static_cast<UINT>(HIT_ORDER::HIT_PLAYER_BODY), 10);

	// ジャンプの値設定
	m_gravityPower = 9.8f * 0.001f;
	m_terminalVelocity = 2.0f;	// 終端速度
	m_YSpeed = 0.0f;
	m_jumpPower = 0.2f;			// ジャンプ力
	
	m_onGround = false;			// 初期状態地面なし
	m_currentTerrain = nullptr;	// 初期状態地面なし
	m_lastMatrix = XMMatrixIdentity();

	// ユニティちゃんのState初期化
	m_nowUnityChanMotion = UnityChanMotion::Idle;
	m_extraAction = false;

	// ゲームオーバーへの遷移の初期化
	m_isTransition = false;
	m_sceneState = SceneState::SceneImage;
}

// 毎フレーム呼ばれる処理　falseを返すとこのコンポーネントは終了し削除される
bool UnityChanPlayer::FrameAction()
{
	FBXCharacterData* chData = static_cast<FBXCharacterData*>(GetGameObject()->GetCharacterData());

	// 移動処理
	WildHuntScene* scene = static_cast<WildHuntScene*>(MyAccessHub::GetMyGameEngine()->GetSceneController());
	KeyBindComponent* keyBind = static_cast<KeyBindComponent*>(scene->GetKeyComponent());
	XMFLOAT3 moveVect = {};	// ボタン入力を方向に見立ててベクトルを作成

	// 地形判定をしよう
	TerrainComponent* terCom = nullptr;
	XMFLOAT3 hitPos = {};
	XMFLOAT3 hitNormal = {};
	XMFLOAT3 rayStart;
	XMFLOAT3 rayEnd;
	HitRayLine ray;

	if (m_currentTerrain != nullptr)
	{
		// 地形移動
		CharacterData* trData = m_currentTerrain->GetGameObject()->GetCharacterData();
		XMMATRIX& nowMat = trData->GetWorldMatrix();

		XMFLOAT3 pos = chData->GetPosition();
		XMVECTOR posVect = XMLoadFloat3(&pos);

		float rad = XMConvertToRadians(chData->GetRotation().y);

		XMFLOAT3 rot{pos.x - cos(rad), pos.y, pos.z + sin(rad)};

		XMVECTOR rotVect = XMLoadFloat3(&rot);

		posVect = XMVector3Transform(posVect, m_lastMatrix);	// 前の逆行列をかけて
		rotVect = XMVector3Transform(rotVect, m_lastMatrix);

		posVect = XMVector3Transform(posVect, nowMat);	// 今の行列をかける
		rotVect = XMVector3Transform(rotVect, nowMat);

		m_lastMatrix = trData->GetInverseWorldMatrix();	// 今の逆行列を保存

		chData->SetPosition(XMVectorGetX(posVect), XMVectorGetY(posVect), XMVectorGetZ(posVect));
		pos = chData->GetPosition();

		chData->SetRotation(0.0f, XMConvertToDegrees( atan2(XMVectorGetZ(rotVect) - pos.z, -(XMVectorGetX(rotVect) - pos.x)) ), 0.0f);
	}
	else
	{
		m_onGround = false;
		m_lastMatrix = XMMatrixIdentity();
	}


	// 入力モードで処理を分岐
	switch (keyBind->GetCurrentInputType())
	{
		case KeyBindComponent::INPUT_TYPES::KEYBOARD:
		{
			// X方向
			if (keyBind->GetCurrentInputState(InputManager::BUTTON_STATE::BUTTON_PRESS, KeyBindComponent::BUTTON_IDS::MOVE_LEFT))
			{
				moveVect.x = -1.0f;	// 左
			}
			else if (keyBind->GetCurrentInputState(InputManager::BUTTON_STATE::BUTTON_PRESS, KeyBindComponent::BUTTON_IDS::MOVE_RIGHT))
			{
				moveVect.x = 1.0f;	// 右
			}

			// Z方向
			if (keyBind->GetCurrentInputState(InputManager::BUTTON_STATE::BUTTON_PRESS, KeyBindComponent::BUTTON_IDS::MOVE_BACK))
			{
				moveVect.z = -1.0f;	// 下（手前）
			}
			else if (keyBind->GetCurrentInputState(InputManager::BUTTON_STATE::BUTTON_PRESS, KeyBindComponent::BUTTON_IDS::MOVE_FORWARD))
			{
				moveVect.z = 1.0f;	// 上（奥）
			}

			// 斜め移動対応
			if (moveVect.x != 0.0f && moveVect.z != 0.0f)
			{
				// ルート2で割って正規化
				moveVect.x /= 1.41421356f;
				moveVect.z /= 1.41421356f;
			}


			// 空中にいるときの処理
			if (m_onGround)
			{
				if (keyBind->GetCurrentInputState(InputManager::BUTTON_STATE::BUTTON_DOWN, KeyBindComponent::BUTTON_IDS::BTN_JUMP) && !m_isTransition)
				{
					m_onGround = false;
					m_YSpeed = m_jumpPower;

					chData->SetAnime(L"JUMP");

					MyGameEngine* pEngine = MyAccessHub::GetMyGameEngine();
					pEngine->GetSoundManager()->Play(11);
				}
			}
		}
			break;

		case KeyBindComponent::INPUT_TYPES::XINPUT_0:
		{
			moveVect.x = keyBind->GetAnalogValue(KeyBindComponent::INPUT_TYPES::XINPUT_0, KeyBindComponent::ANALOG_IDS::MOVE_H);
			moveVect.z = keyBind->GetAnalogValue(KeyBindComponent::INPUT_TYPES::XINPUT_0, KeyBindComponent::ANALOG_IDS::MOVE_V);
		}
			break;
	}

	// カメラ角度の反映
	XMFLOAT3 camVect = m_currentCamera->GetCameraDirection();
	XMVECTOR rotMove = {};		// 回転計算用バッファ
	float xzRadian;	// Yaw
	xzRadian = atan2f(camVect.x, camVect.z);			// Y軸回転角
	XMMATRIX qtXZ = XMMatrixRotationY(xzRadian);		// Y軸回転行列 Yaw

#if MOVE_3D
	//  3D版
	XMVECTOR workVect = XMVector3Transform(XMLoadFloat3(&camVect), XMMatrixRotationY(-xzRadian));

	float yzRadian;	// Pitch
	yzRadian = atan2f(-XMVectorGetY(workVect), XMVectorGetZ(workVect));			// X軸回転角 Yがひっくり返る事を忘れないように
	XMMATRIX qtYZ = XMMatrixRotationX(yzRadian);		// X軸回転行列 Pitch
	
	// moveVectに変形行列を掛ける
	rotMove = XMVector3Transform(XMLoadFloat3(&moveVect), qtYZ * qtXZ);

	// Unityちゃんを移動方向に向ける
	chData->setRotation(XMConvertToDegrees(yzRadian), XMConvertToDegrees(xzRadian), 0.0f);
#else
	// 2D版
	// rotMoveの変換もYawだけで良い。
	rotMove = XMVector3Transform(XMLoadFloat3(&moveVect), qtXZ);

	// 回避の時用に直前に移動した方向を保持しとく
	if (m_nowUnityChanMotion != UnityChanMotion::Evasion)
	{
		if (moveVect.x != 0.0f || moveVect.z != 0.0f)
		{
			m_evadeMoveVect = rotMove;
		}
	}

	// 攻撃ボタンが押されたか、m_nowUnityChanMotionのStateがLAttackのとき
	if (keyBind->GetCurrentInputState(InputManager::BUTTON_STATE::BUTTON_DOWN, KeyBindComponent::BUTTON_IDS::BTN_ATTACK) || m_nowUnityChanMotion == UnityChanMotion::LAttack)
	{
		MyGameEngine* pEngine = MyAccessHub::GetMyGameEngine();

		// 現在のStateがLAttackでない ⇒ 攻撃ボタンが押された瞬間
		if (m_nowUnityChanMotion != UnityChanMotion::LAttack)
		{
			pEngine->GetSoundManager()->Play(3);
			pEngine->GetSoundManager()->Play(6);
			m_pSwordComp->WeaponAttack(true);
		}

		// 現在のStateがLAttack ⇒ すでに攻撃ボタンが押されたあと
		if (m_nowUnityChanMotion == UnityChanMotion::LAttack)	// 追加攻撃の確認
		{
			// 追加攻撃
			if (keyBind->GetCurrentInputState(InputManager::BUTTON_STATE::BUTTON_DOWN, KeyBindComponent::BUTTON_IDS::BTN_ATTACK) && !m_extraAction)
			{
				pEngine->GetSoundManager()->Play(4);
				pEngine->GetSoundManager()->Play(6);
				m_pSwordComp->WeaponAttack(true);
				m_extraAction = true;	// m_extraActionをtrueにして、アニメーション再生を切り替える
			}
		}

		// ここでLAttackStateに
		m_nowUnityChanMotion = UnityChanMotion::LAttack;

		m_pTPCameraComp->ChangeCameraDistance(3.0f);

		// アニメーションの再生切り替え
		switch (m_attackSeries)
		{
		case 0:
			chData->SetAnime(L"LAttack");
			break;
		case 1:
			chData->SetAnime(L"LAttack2");
			break;
		case 3:
			chData->SetAnime(L"LAttack_Follow_Through");
			break;
		default:
			//chData->SetAnime(L"WAIT00");
			m_nowUnityChanMotion = UnityChanMotion::Idle;
			m_attackSeries = 0;
		}

		if (chData->GetAnimeEnd())	// アニメーション終了時の処理
		{
			if (m_extraAction)	// 追加の攻撃あるかどうか
			{
				m_extraAction = false;
				m_attackSeries++;
				switch (m_attackSeries)	// モーションの切れ間を起こさないため、もう一回で
				{
				case 0:
					chData->SetAnime(L"LAttack");
					break;
				case 1:
					chData->SetAnime(L"LAttack2");
					break;
				case 3:
					chData->SetAnime(L"LAttack_Follow_Through");
					break;
				default:
					chData->SetAnime(L"WAIT00");
					m_nowUnityChanMotion = UnityChanMotion::Idle;
					m_attackSeries = 0;
				}
			}
			else
			{
				m_pSwordComp->WeaponAttack(false);	// 攻撃の当たり判定を消す
				if (m_attackSeries <= 0)
				{
					chData->SetAnime(L"LAttack_Follow_Through");
					m_attackSeries = 3;
				}
				else
				{
					m_attackSeries = 0;
					m_nowUnityChanMotion = UnityChanMotion::Idle;
					chData->SetAnime(L"WAIT00");
				}
			}
		}


	}
	else if (keyBind->GetCurrentInputState(InputManager::BUTTON_STATE::BUTTON_DOWN, KeyBindComponent::BUTTON_IDS::BTN_EVADE) || m_nowUnityChanMotion == UnityChanMotion::Evasion)
	{
		// 回避ボタンが押されているか、EvasionStateのとき

		// 現在のStateがEvasionでない ⇒ 回避ボタンが押された瞬間
		if (m_nowUnityChanMotion != UnityChanMotion::Evasion)
		{
			MyGameEngine* pEngine = MyAccessHub::GetMyGameEngine();
			pEngine->GetSoundManager()->Play(11);
			
			if (moveVect.x != 0.0f || moveVect.z != 0.0f)
				m_evadeMoveVect = rotMove;
		}

		// アニメーションが終わりかどうか
		if (chData->GetAnimeEnd() && m_nowUnityChanMotion == UnityChanMotion::Evasion)
			m_nowUnityChanMotion = UnityChanMotion::Idle;
		else
		{
			// ここでEvasionStateに
			m_nowUnityChanMotion = UnityChanMotion::Evasion;
			chData->SetAnime(L"Evasion");	// アニメーション続行
		}
	}
	else if (moveVect.x != 0.0f || moveVect.z != 0.0f)	// 入力処理があった時のみ回転を更新する
	{
		chData->SetRotation(0.0f, XMConvertToDegrees(atan2f(XMVectorGetX(rotMove), XMVectorGetZ(rotMove))), 0.0f);

		if (m_onGround)	// 空中にいないので
			chData->SetAnime(L"SlowRun");
	}
	else
	{
		if (m_onGround) // 空中にいないで、入力処理なし
			chData->SetAnime(L"WAIT00");	// 停止アニメ
	}
#endif

	// 結果のrotMoveをmoveVectに反映
	if (m_nowUnityChanMotion == UnityChanMotion::Evasion)
	{
		moveVect.x = XMVectorGetX(m_evadeMoveVect); // XMVECTORはxyzでは取れないので注意
		moveVect.y = XMVectorGetY(m_evadeMoveVect);
		moveVect.z = XMVectorGetZ(m_evadeMoveVect);

		// 移動量を掛けたら移動ベクトルになる。
		float evadeValue = 1.5f;
		moveVect.x *= evadeValue;
		moveVect.y *= evadeValue;
		moveVect.z *= evadeValue;
	}
	else
	{
		moveVect.x = XMVectorGetX(rotMove); // XMVECTORはxyzでは取れないので注意
		moveVect.y = XMVectorGetY(rotMove);
		moveVect.z = XMVectorGetZ(rotMove);
	}

	// 出来た移動ベクトルに移動速度(0.1f)をかける。
	float moveValue = 0.1f;
	moveVect.x *= moveValue;
	moveVect.y *= moveValue;
	moveVect.z *= moveValue;

	XMFLOAT3 nowPos = chData->GetPosition();
	nowPos.x += moveVect.x;
	nowPos.y += moveVect.y;
	nowPos.z += moveVect.z;

	nowPos.y += m_YSpeed;

	// 判定の結果進めない場合もあるのでsetPositionより前で地形判定
	bool canWalk = true;
	
	// 進めるかチェック
	rayStart = chData->GetPosition();
	rayStart.y += m_walkableHeight;
	rayEnd = nowPos;
	rayEnd.y += m_walkableHeight;

	ray.SetLine(rayStart, rayEnd, 0.0f);

	for (int i = 0; (terCom = scene->GetTerrainComponent(i)) != nullptr; i++)
	{
		if (terCom->RayCastHit(ray, hitPos, hitNormal))
		{
			// X,Zをヒット位置に補正
			nowPos.x = hitPos.x;
			nowPos.z = hitPos.z;

			break;
		}
	}


	// 足元もチェックしよう。
	rayStart = nowPos;
	rayEnd = nowPos;

	if (m_YSpeed < 0.0f)
	{
		rayStart.y += m_walkableHeight - m_YSpeed;
	}
	else
	{
		rayStart.y += m_walkableHeight;
	}
	rayEnd.y -= m_walkableHeight;

	ray.SetLine(rayStart, rayEnd, 0.0f);

	TerrainComponent* backupTr = m_currentTerrain;	// 地形比較用
	m_currentTerrain = nullptr;

	for (int i = 0; (terCom = scene->GetTerrainComponent(i)) != nullptr; i++)
	{
 		if (terCom->RayCastHit(ray, hitPos, hitNormal))
		{
			// 床の高さに現在位置を調整
			if (m_onGround || m_YSpeed < 0.0f)
			{
				nowPos.y = hitPos.y;

				m_currentTerrain = terCom;	// 追従地形を設定
				if (backupTr != m_currentTerrain)
				{
					// 地形の逆行列を取得
					m_lastMatrix = terCom->GetGameObject()->GetCharacterData()->GetInverseWorldMatrix();
				}
				break;
			}
		}
	}

	if (m_currentTerrain != nullptr)
	{
		if (!m_onGround)
		{
			m_onGround = true;
			m_YSpeed = 0.0f;
		}
	}
	else
	{
		// 床がない
		canWalk = false;
		m_onGround = false;
	}

	if (!m_onGround)
	{
		m_YSpeed -= m_gravityPower;

		if (m_YSpeed < -m_terminalVelocity)
			m_YSpeed = -m_terminalVelocity;
		
	}

	// 現在位置更新
	if (m_nowUnityChanMotion != UnityChanMotion::LAttack)
	{
		chData->SetPosition(nowPos.x, nowPos.y, nowPos.z);
	}

	if (m_isTransition)
		UpdateSceneState();

	// アニメをすすめる
	chData->UpdateAnimation();

	// PipeLineに登録
	chData->GetPipeline()->AddRenerObject(chData);

	XMFLOAT3 plPos = chData->GetPosition(); // プレイヤ位置取得

	// 先に武器用にUnityちゃんの現在位置を渡す
	m_pSwordComp->ChangePlayerPos(plPos.x, plPos.y, plPos.z);

	XMFLOAT3 plScale = chData->GetScale();
	m_pSwordComp->ChangePlayerScale(plScale.x, plScale.y, plScale.z);

	XMFLOAT3 plRot = chData->GetRotation();
	m_pSwordComp->ChangePlayerRotate(plRot.x, plRot.y, plRot.z);

	const char* boneName = "Character1_RightHand";
	XMMATRIX xm = chData->GetAnimatedMatrixByBoneName(boneName);

	m_pSwordComp->ChangePlayerHandData(xm);

	// もう一つ、敵にUnityちゃんの位置を教える
	for (int i = 0; i < m_pZakoEnemyCompList.size(); i++)
	{
		m_pZakoEnemyCompList[i]->ChangePlayerPos(plPos.x, plPos.y, plPos.z);
	}

	m_pNightmareDragonComp->ChangePlayerPos(plPos.x, plPos.y, plPos.z);

	// カメラ制御 Focusを現在位置の「頭部」位置にする
	m_currentCamera->ChangeCameraFocus(plPos.x, plPos.y + m_unityChanHeadHeight, plPos.z);

	// Unityちゃん判定移動
	bodyColl.SetCenter(plPos.x, plPos.y + m_hitHeight * 0.5f, plPos.z);

	// 判定セット
	MyAccessHub::GetMyGameEngine()->GetHitManager()->SetHitArea(this, &bodyColl);

	return true;
}

// 終了時に呼ばれる処理
void UnityChanPlayer::FinishAction()
{
}

// ヒット時の処理
void UnityChanPlayer::HitReaction(GameObject* targetGo, HitAreaBase* hit)
{
	// HP減少
	if (hit->GetHitType() == static_cast<UINT>(HIT_ORDER::HIT_ITEM))
	{
		if (m_nowPlHp + hit->GetHitPower() >= m_maxPlHp)
			m_nowPlHp = m_maxPlHp;				// 最大HPを超えないため
		else
			m_nowPlHp += hit->GetHitPower();	// HP回復
		
		m_pPlHPbarUiComp->ChangeHp(m_nowPlHp);	// 現在のHPをUIに

		MyGameEngine* pEngine = MyAccessHub::GetMyGameEngine();
		pEngine->GetSoundManager()->Play(14);
	}
	else
	{
		m_plTotalDamage += hit->GetHitPower();	// 被ダメージ量を保存
		m_nowPlHp -= hit->GetHitPower();		// ダメージを受ける
		m_pPlHPbarUiComp->ChangeHp(m_nowPlHp);	// 現在のHPをUIに
	}


	// HPが0以下なら画面遷移のトランジションをする
	if (m_nowPlHp <= 0 && !m_isTransition)
	{
		m_isTransition = true;
		m_nextScene = GAME_SCENES::GAME_OVER;	// 次のシーンをゲームオーバーシーンに

		m_pTransitionAnimatorComp->SetWipeMode(WipeMode::WipeOut);
		m_pTransitionAnimatorComp->PlayTransition();
		m_sceneState = SceneState::Transition;
	}
}