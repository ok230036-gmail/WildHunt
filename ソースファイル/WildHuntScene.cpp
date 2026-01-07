#include "WildHuntScene.h"
#include "WildHuntEnum.h"
#include "MyAccessHub.h"
#include "KeyBindComponent.h"

#include "SpriteRenderPipeline.h"

#include "FBXCharacterData.h"
#include "StandardLightingPipeline.h"
#include "LightSettingManager.h"

#include "GBufferResetCommand.h"	// GBufferのクリアを行うパイプライン

#include "EdgeDrawPipeline.h"		// 輪郭線抽出
#include "ShadowMapPipeline.h"		// Depth Shadow (Pre Pipeline)

// Fbx Loadをもうちょっと最適化（一つのメッシュを使い回せるように）
#include "FBXDataContainerSystem.h"

#include "TitleScene.h"

// GameObjects
#include "SkyDomeComponent.h"	// スカイドーム
#include "TerrainComponent.h"	// テライン
#include "CameraComponent.h"	// カメラ

#include "UnityChanPlayer.h"	// プレイヤーキャラクター

#include "CameraChangerComponent.h"			// カメラ切り替え
#include "ThirdPersonCameraController.h"	// 三人称カメラ

#include "WildHuntUIRender.h"		// Timer
#include "PlayTimerBGUI.h"			// ゲームタイマーのバックグラウンドUI

#include "HeartItemComponent.h"		// ハートアイテム

#include "PlayerWeaponSword.h"		// プレイヤーの剣
#include "SwordHit2DEffect.h"		// 剣のヒットエフェクト

#include "TurtleShellZakoEnemy.h"	// 雑魚敵

#include "PlayerHPbarUI.h"			// プレイヤーのHPバーのUI

#include "NightmareDragonEnemy.h"	// 敵キャラのドラゴン
#include "FireBreatheEffect.h"		// ドラゴンのブレス

#include "ParchmentResultUI.h"		// リザルト画面

#include "TransitionAnimator.h"		// シーン遷移時のトランジションアニメーション

void WildHuntScene::AddSceneObject(GameObject* obj)
{
	MyGameEngine* engine = MyAccessHub::GetMyGameEngine();

	// Deleterセット これでEngine内でGameObjectが消えた時もGameScene側が削除できる
	obj->SetGameObjectDeleter(m_pObjDeleter.get());
	m_SceneObjects.push_back(unique_ptr<GameObject>(obj));

	engine->SetGameObjectToAddQueue(obj);
}

void WildHuntScene::DeleteSceneObject(GameObject* obj)
{
	auto ite = m_SceneObjects.begin();
	// objを持っている場所を探す
	for (; ite != m_SceneObjects.end(); ite++)
	{
		if (ite->get() == obj)
		{
			break;
		}
	}

	// removeがあるのでループの外で処理
	if (ite != m_SceneObjects.end())
	{
		ite->release();	// 所有権放棄（deleteを他に任せる）
		m_SceneObjects.remove(*ite);
	}
}

void WildHuntScene::ClearSceneObjects()
{
	if (m_systemObject == nullptr)
		return;

	MyGameEngine* engine = MyAccessHub::GetMyGameEngine();
	for (auto ite = m_SceneObjects.begin(); ite != m_SceneObjects.end(); ite++)
	{
		GameObject* go = ite->get();
		ite->release();	// 所有権放棄（deleteを他に任せる）
		engine->RemoveGameObject(go);

		delete(go);
	}
	m_SceneObjects.clear();
	m_terrains.clear();

}

WildHuntScene::~WildHuntScene()
{
	m_terrains.clear();
	m_SceneObjects.clear();
	m_cameraComponents.clear();
}

HRESULT WildHuntScene::InitSceneController()
{
	m_scene = static_cast<UINT>(GAME_SCENES::AWAKE);

	m_pObjDeleter = std::make_unique<SceneObjectDeleter>(this);

	HRESULT res = ChangeGameScene(static_cast<UINT>(GAME_SCENES::INIT));

	if (SUCCEEDED(res))
		m_orderSceneID = m_scene;

	return res;
}

HRESULT WildHuntScene::ChangeGameScene(UINT scene)
{
	HRESULT hr = S_OK;
	MyGameEngine* engine = MyAccessHub::GetMyGameEngine();

	// 現在のシーンと引数に設定されているシーンが異なる場合のみ実行
	if (m_scene != scene)
	{
		ClearSceneObjects();

		// Fbx読み込み指示
		switch (m_scene)
		{
			case static_cast<UINT>(GAME_SCENES::IN_GAME):
			{
				FBXDataContainerSystem* fbxSys = FBXDataContainerSystem::GetInstance();

				// ゲームのオブジェクトたち
				fbxSys->DeleteModelFBX(L"SkyDome");
				fbxSys->DeleteModelFBX(L"TerrainSample");
				fbxSys->DeleteModelFBX(L"UnityChan");
				fbxSys->DeleteModelFBX(L"Platform");
				fbxSys->DeleteModelFBX(L"GoldenHeart");

				fbxSys->DeleteModelFBX(L"WeaponSword");
				fbxSys->DeleteModelFBX(L"TurtleShellZakoEnemy");
				fbxSys->DeleteModelFBX(L"NightmareDragon");

				// Unityちゃんアニメ
				fbxSys->DeleteAnimeFBX(L"WAIT00");
				fbxSys->DeleteAnimeFBX(L"WALK_F");
				fbxSys->DeleteAnimeFBX(L"JUMP");
				fbxSys->DeleteAnimeFBX(L"WALK_B");
				fbxSys->DeleteAnimeFBX(L"WALK_L");
				fbxSys->DeleteAnimeFBX(L"WALK_R");
				fbxSys->DeleteAnimeFBX(L"LAttack");
				fbxSys->DeleteAnimeFBX(L"LAttack_Follow_Through");
				fbxSys->DeleteAnimeFBX(L"LAttack2");
				fbxSys->DeleteAnimeFBX(L"Evasion");
				fbxSys->DeleteAnimeFBX(L"SlowRun");

				// TurtleShellZakoEnemy用のアニメ
				fbxSys->DeleteAnimeFBX(L"IdleNormal");
				fbxSys->DeleteAnimeFBX(L"Die");
				fbxSys->DeleteAnimeFBX(L"Walk");
				fbxSys->DeleteAnimeFBX(L"Attack01");

				// NightmareDragon用のアニメ
				fbxSys->DeleteAnimeFBX(L"ND_Idle");
				fbxSys->DeleteAnimeFBX(L"ND_Walk");
				fbxSys->DeleteAnimeFBX(L"ND_ClawAttack");
				fbxSys->DeleteAnimeFBX(L"ND_HornAttack");
				fbxSys->DeleteAnimeFBX(L"ND_BreatheAttack");
				fbxSys->DeleteAnimeFBX(L"ND_Scream");
				fbxSys->DeleteAnimeFBX(L"ND_Die");
				fbxSys->DeleteAnimeFBX(L"ND_Run");
				fbxSys->DeleteAnimeFBX(L"ND_GetHit");
				fbxSys->DeleteAnimeFBX(L"ND_Jump");
				fbxSys->DeleteAnimeFBX(L"ND_Sleep");
			}
				break;
		}

		// この中でGameObjectの切り替えを行う事になる
		switch (scene)
		{
		case static_cast<UINT>(GAME_SCENES::INIT):	// ゲームシステム全体の初期化
			{
				// テクスチャと効果音の読み込み

				engine->GetTextureManager()->CreateTextureFromFile(engine->GetDirect3DDevice(), L"Sprite00", L"./Resources/textures/texture.png");

				engine->GetTextureManager()->CreateTextureFromFile(engine->GetDirect3DDevice(), L"HUDTexture", L"./Resources/textures/HUD/WordTexture.png");

				engine->GetTextureManager()->CreateTextureFromFile(engine->GetDirect3DDevice(), L"TitleTexture", L"./Resources/textures/Title/WildHunt_TitleImage.png");

				engine->GetTextureManager()->CreateTextureFromFile(engine->GetDirect3DDevice(), L"ToneTexture",L"./Resources/textures/ToonShader/Tone.png");

				engine->GetTextureManager()->CreateTextureFromFile(engine->GetDirect3DDevice(), L"HPBarTexture", L"./Resources/textures/HPBar/HpBar_Unitytyan.png");

				engine->GetTextureManager()->CreateTextureFromFile(engine->GetDirect3DDevice(), L"HitEffect", L"./Resources/textures/HitEffect/hit_eff.png");

				engine->GetTextureManager()->CreateTextureFromFile(engine->GetDirect3DDevice(), L"ParchmentResult", L"./Resources/textures/ParchmentResult/ParchmentUI.png");

				engine->GetTextureManager()->CreateTextureFromFile(engine->GetDirect3DDevice(), L"PlayTimerBG", L"./Resources/textures/PlayTimerUI/PlayTimerBG.png");

				engine->GetTextureManager()->CreateTextureFromFile(engine->GetDirect3DDevice(), L"Transition", L"./Resources/textures/Transition/8bit-transition.png");
				
				engine->GetTextureManager()->CreateTextureFromFile(engine->GetDirect3DDevice(), L"FireBreathe", L"./Resources/textures/FireBreathe/FireBreatheEffect.png");


				engine->GetTextureManager()->CreateRenderTargetTexture(engine->GetDirect3DDevice(), L"NormalBuffer",
					engine->GetWidth(), engine->GetHeight(), DXGI_FORMAT_R32G32B32A32_FLOAT);

				engine->GetMeshManager()->CreatePresetMeshData();

				// PipeLineManager
				PipeLineManager* plMng = engine->GetPipelineManager();

				SpriteRenderPipeline* spritePL = new SpriteRenderPipeline();
				spritePL->SetSamplerMode(0);	// フィルタなし
				spritePL->SetBlendMode(0);		// カットオフのみ
				plMng->AddPipeLineObject(L"Sprite", spritePL);

				spritePL = new SpriteRenderPipeline();
				spritePL->SetSamplerMode(0);	// フィルタなし
				spritePL->SetBlendMode(1);		// アルファブレンド
				plMng->AddPipeLineObject(L"AlphaSprite", spritePL);

				// Post Effect Pipeline
				EdgeDrawPipeline* edgePL = new EdgeDrawPipeline();
				engine->GetPostEffectPipelineManager()->AddPipeLineObject(L"EdgeDraw", edgePL);

				// Pre Draw Pipeline
				PipeLineManager* pPreDrawMng = engine->GetPreDrawPipelineManager();

				// GBuffer System
				GBufferResetCommand* gbuffPL = new GBufferResetCommand();
				pPreDrawMng->AddPipeLineObject(L"GBuffReset", gbuffPL);

				// Shadow Map Pipeline
				ShadowMapPipeline* pStaticShadow = new ShadowMapPipeline();
				pStaticShadow->SetStaticMeshMode(true);
				pPreDrawMng->AddPipeLineObject(L"StaticShadowMap", pStaticShadow);

				// スケルタルメッシュ用
				ShadowMapPipeline* pSkeltalShadow = new ShadowMapPipeline();
				pSkeltalShadow = new ShadowMapPipeline();
				pSkeltalShadow->SetStaticMeshMode(false);
				pPreDrawMng->AddPipeLineObject(L"SkeltalShadowMap", pSkeltalShadow);

				gbuffPL->SetRTVTexture(engine->GetTextureManager()->GetTexture(L"NormalBuffer"));

				// FBX
				StandardLightingPipeline* fbxPL = new StandardLightingPipeline();
				fbxPL->SetPipelineFlags(0);
				plMng->AddPipeLineObject(L"StaticFBX", fbxPL);

				fbxPL = new StandardLightingPipeline();
				fbxPL->SetPipelineFlags(StandardLightingPipeline::PIPELINE_FLAGS::SKELTAL);

				plMng->AddPipeLineObject(L"AnimationFBX", fbxPL);

				// Lambert pipeline
				fbxPL = new StandardLightingPipeline();
				fbxPL->SetPipelineFlags(StandardLightingPipeline::PIPELINE_FLAGS::Lambert);
				plMng->AddPipeLineObject(L"StaticLambert", fbxPL);	// Staticメッシュ + Lambert

				fbxPL = new StandardLightingPipeline();
				fbxPL->SetPipelineFlags(StandardLightingPipeline::PIPELINE_FLAGS::SKELTAL |
					StandardLightingPipeline::PIPELINE_FLAGS::Lambert);
				plMng->AddPipeLineObject(L"SkeltalLambert", fbxPL);	// Skeltalメッシュ + Lambert

				// Phong pipeline
				fbxPL = new StandardLightingPipeline();
				fbxPL->SetPipelineFlags(StandardLightingPipeline::PIPELINE_FLAGS::Lambert |
					StandardLightingPipeline::PIPELINE_FLAGS::Phong);
				plMng->AddPipeLineObject(L"StaticPhong", fbxPL);	// Staticメッシュ + Phong

				fbxPL = new StandardLightingPipeline();
				fbxPL->SetPipelineFlags(StandardLightingPipeline::PIPELINE_FLAGS::SKELTAL |
					StandardLightingPipeline::PIPELINE_FLAGS::Lambert | StandardLightingPipeline::PIPELINE_FLAGS::Phong);
				plMng->AddPipeLineObject(L"SkeltalPhong", fbxPL);	// Skeltalメッシュ + Phong

				// BlinnPhong
				fbxPL = new StandardLightingPipeline();
				fbxPL->SetPipelineFlags(StandardLightingPipeline::PIPELINE_FLAGS::Lambert |
					StandardLightingPipeline::PIPELINE_FLAGS::Blinn);
				plMng->AddPipeLineObject(L"StaticBlinn", fbxPL);	// Staticメッシュ + BlinnPhong

				fbxPL = new StandardLightingPipeline();
				fbxPL->SetPipelineFlags(StandardLightingPipeline::PIPELINE_FLAGS::SKELTAL |
					StandardLightingPipeline::PIPELINE_FLAGS::Lambert | StandardLightingPipeline::PIPELINE_FLAGS::Blinn);
				plMng->AddPipeLineObject(L"SkeltalBlinn", fbxPL);	// Skeltalメッシュ + BlinnPhong

				// Toon Shader
				fbxPL = new StandardLightingPipeline();
				fbxPL->SetPipelineFlags(StandardLightingPipeline::PIPELINE_FLAGS::Lambert | StandardLightingPipeline::PIPELINE_FLAGS::Tone);
				plMng->AddPipeLineObject(L"StaticToon", fbxPL);		// Staticメッシュ + Toon
				fbxPL = new StandardLightingPipeline();
				fbxPL->SetPipelineFlags(StandardLightingPipeline::PIPELINE_FLAGS::SKELTAL |
					StandardLightingPipeline::PIPELINE_FLAGS::Lambert | StandardLightingPipeline::PIPELINE_FLAGS::Tone);
				plMng->AddPipeLineObject(L"SkeltalToon", fbxPL);	// Skeltalメッシュ + Toon

				// LightingSettingManager作成
				LightSettingManager* lightMng = LightSettingManager::GetInstance();
				XMFLOAT3 lightColor;
				XMFLOAT3 lightDirection;

				// ambient
				lightColor = { 0.3f, 0.3f, 0.3f };							// ほんのり明るく
				lightMng->CreateAmbientLight(L"SCENE_AMBIENT", lightColor);	// FBXCharacterDataの初期値名
				
				// directional
				lightColor = { 1.0f, 1.0f, 0.8f };							// 昼光色的な
				lightDirection = { -0.57f, -0.57f, 0.57f };					// 左斜め下Z奥向き
				lightMng->CreateDirectionalLight(L"SCENE_DIRECTIONAL", lightColor, lightDirection);	// これも初期値

				// GBufferResetCommandにDepthTextureをセットしておく
				auto shadowTex = engine->GetTextureManager()->GetDepthTexture(L"SCENE_DIRECTIONAL_Depth");
				gbuffPL->SetDepthTexture(shadowTex);

				// ShadowMapPipelineにもセット static用とskeltal用が別。
				pStaticShadow->SetDepthTexture(shadowTex);
				pSkeltalShadow->SetDepthTexture(shadowTex);

				HitManager* hitMng = engine->GetHitManager();

				// シールド判定を先にチェック
				hitMng->InitHitList(7);

				hitMng->SetHitOrder((UINT)HIT_ORDER::HIT_PLAYER_ATTACK, (UINT)HIT_ORDER::HIT_ENEMY_SHIELD);	// プレイヤーの攻撃 VS 敵の盾
				hitMng->SetHitOrder((UINT)HIT_ORDER::HIT_PLAYER_ATTACK, (UINT)HIT_ORDER::HIT_ENEMY_BODY);	// プレイヤーの攻撃 VS 敵
				hitMng->SetHitOrder((UINT)HIT_ORDER::HIT_ENEMY_ATTACK, (UINT)HIT_ORDER::HIT_PLAYER_SHIELD);	// 敵の攻撃 VS プレイヤーの盾
				hitMng->SetHitOrder((UINT)HIT_ORDER::HIT_ENEMY_ATTACK, (UINT)HIT_ORDER::HIT_PLAYER_BODY);	// 敵の攻撃 VS プレイヤー
				hitMng->SetHitOrder((UINT)HIT_ORDER::HIT_PLAYER_BODY, (UINT)HIT_ORDER::HIT_ENEMY_BODY);		// プレイヤー VS 敵
				hitMng->SetHitOrder((UINT)HIT_ORDER::HIT_PLAYER_BODY, (UINT)HIT_ORDER::HIT_ITEM);			// プレイヤー VS アイテム

				// サウンドリソースを追加
				SoundManager* soMng = engine->GetSoundManager();

				int soundId = 0;

				if (!soMng->LoadSoundFile(L"./Resources/sounds/univ1091.wav", soundId))
					return E_FAIL;
				if (!soMng->LoadSoundFile(L"./Resources/sounds/univ1093.wav", soundId))
					return E_FAIL;
				if (!soMng->LoadSoundFile(L"./Resources/sounds/univ1257.wav", soundId))
					return E_FAIL;
				if (!soMng->LoadSoundFile(L"./Resources/sounds/univ1254.wav", soundId))
					return E_FAIL;
				if (!soMng->LoadSoundFile(L"./Resources/sounds/univ1255.wav", soundId))
					return E_FAIL;
				if (!soMng->LoadSoundFile(L"./Resources/sounds/damageSE.wav", soundId))
					return E_FAIL;
				if (!soMng->LoadSoundFile(L"./Resources/sounds/swordSE.wav", soundId))
					return E_FAIL;
				if (!soMng->LoadSoundFile(L"./Resources/sounds/GrassFootSE_01.wav", soundId))
					return E_FAIL;
				if (!soMng->LoadSoundFile(L"./Resources/sounds/DragonFireBreathe.wav", soundId))
					return E_FAIL;
				if (!soMng->LoadSoundFile(L"./Resources/sounds/WipeInTransition.wav", soundId))
					return E_FAIL;
				if (!soMng->LoadSoundFile(L"./Resources/sounds/WipeOutTransition.wav", soundId))
					return E_FAIL;
				if (!soMng->LoadSoundFile(L"./Resources/sounds/univ0001.wav", soundId))
					return E_FAIL;

				// システム制御統合オブジェクト登録
				m_systemObject = make_unique<GameObject>(nullptr);
				m_keyComponent = new KeyBindComponent();
				m_systemObject->AddComponent(m_keyComponent);
				engine->AddGameObject(m_systemObject.get());

				engine->UploadCreatedTextures();

				return ChangeGameScene(static_cast<UINT>(GAME_SCENES::TITLE));	// タイトル画面へ
			}
			break;

		case static_cast<UINT>(GAME_SCENES::TITLE):	//タイトル画面
			{
				GameObject* cameraObj = new GameObject(new CharacterData());
				TitleScene* titleCamera = new TitleScene();
				cameraObj->AddComponent(titleCamera);

				titleCamera->SetNextScene(GAME_SCENES::IN_GAME);	// Space押したときに、IN_GAMEに
				titleCamera->SetBGColor(0.5, 0.5f, 0.5f);
				titleCamera->SetImagePosition(74.0f);

				AddSceneObject(cameraObj);

				// カメラリスト追加 UIモード時用
				m_cameraComponents[L"TitleCamera"] = titleCamera;

				cameraObj = new GameObject(new CharacterData());
				TransitionAnimator* transitionAnimator = new TransitionAnimator();
				cameraObj->AddComponent(transitionAnimator);
				AddSceneObject(cameraObj);

				// カメラリスト追加 UIモード時用
				m_cameraComponents[L"HUDCamera"] = transitionAnimator;

				transitionAnimator->SetWipeMode(WipeMode::WipeOut);	// アニメーションはWipeOutから

				titleCamera->SetTransitionAnimatorComponent(transitionAnimator);	// TransitionAnimatorを実行するために、コンポーネントのポインタを渡す
			}
			break;

		case static_cast<UINT>(GAME_SCENES::IN_GAME):	// 実ゲームの処理
			{
				FBXDataContainerSystem* fbxSys = FBXDataContainerSystem::GetInstance();

				// テラインを作成
				FBXCharacterData* terrainFbx = new FBXCharacterData();

				if (FAILED(fbxSys->LoadModelFBX(L"./Resources/fbx/OriginalTerrain02.fbx", L"TerrainSample")))
					return E_FAIL;
				terrainFbx->SetMainFBX(L"TerrainSample");	// メインFBXの登録
				terrainFbx->SetCastShadow(false);

				GameObject* terrainObj = new GameObject(terrainFbx);

				// 地形用コンポーネント作成。後の当たり判定の時に大幅追記
				TerrainComponent* trCom = new TerrainComponent();
				terrainObj->AddComponent(trCom);

				AddSceneObject(terrainObj);
				
				// m_terrainsにTerrainComponentを登録
				m_terrains.push_back(trCom);

				// スカイドームを作成
				FBXCharacterData* skydomeFbx = new FBXCharacterData();
				if (FAILED(fbxSys->LoadModelFBX(L"./Resources/fbx/OriginalSkyDome.fbx", L"SkyDome")))
					return E_FAIL;
				skydomeFbx->SetMainFBX(L"SkyDome");	// メインFBXの登録
				
				GameObject* skydomeObj = new GameObject(skydomeFbx);
				SkyDomeComponent* skCom = new SkyDomeComponent();
				skydomeObj->AddComponent(skCom);

				AddSceneObject(skydomeObj);

				// Unityちゃん登録
				GameObject* unityChanObj;
				FBXCharacterData* unityChanFbx = new FBXCharacterData();	// FBX用CharacterData
				UnityChanPlayer* unityChanPlayer = new UnityChanPlayer();	// Unityちゃん本体

				// UnityちゃんのFBXとモーションの読み込み
				fbxSys->LoadModelFBX(L"./Resources/fbx/unitychan.fbx", L"UnityChan");
				fbxSys->LoadAnimationFBX(L"./Resources/fbx/UnityChanAnime/unitychan_WAIT00.fbx", L"WAIT00");
				fbxSys->LoadAnimationFBX(L"./Resources/fbx/UnityChanAnime/unitychan_WALK00_F.fbx", L"WALK_F");

				fbxSys->LoadAnimationFBX(L"./Resources/fbx/UnityChanAnime/unitychan_WALK00_B.fbx", L"WALK_B");
				fbxSys->LoadAnimationFBX(L"./Resources/fbx/UnityChanAnime/unitychan_WALK00_L.fbx", L"WALK_L");
				fbxSys->LoadAnimationFBX(L"./Resources/fbx/UnityChanAnime/unitychan_WALK00_R.fbx", L"WALK_R");

				fbxSys->LoadAnimationFBX(L"./Resources/fbx/UnityChanAnime/unitychan_UMATOBI00.fbx", L"JUMP");

				fbxSys->LoadAnimationFBX(L"./Resources/fbx/UnityChanAnime/Stable_Sword_Outward_Slash.fbx", L"LAttack");
				fbxSys->LoadAnimationFBX(L"./Resources/fbx/UnityChanAnime/Stable_Sword_Slash_Follow_Through.fbx", L"LAttack_Follow_Through");
				fbxSys->LoadAnimationFBX(L"./Resources/fbx/UnityChanAnime/Stable_Sword_Inward_Slash.fbx", L"LAttack2");
				fbxSys->LoadAnimationFBX(L"./Resources/fbx/UnityChanAnime/Sprinting_Forward_Roll.fbx", L"Evasion");
				fbxSys->LoadAnimationFBX(L"./Resources/fbx/UnityChanAnime/Slow_Run.fbx", L"SlowRun");

				unityChanObj = new GameObject(unityChanFbx);	// FBXCharacterDataを持たせて初期化
				unityChanObj->AddComponent(unityChanPlayer);	// Unityちゃん本体コンポーネントをセット
				AddSceneObject(unityChanObj);

				// カメラ
				GameObject* cameraObj;
				cameraObj = new GameObject(new CharacterData());
				CameraComponent* camComp = new CameraComponent();

				// カメラリスト追加 UIモード時用
				m_cameraComponents[L"MainCamera"] = camComp;

				cameraObj->AddComponent(camComp);
				AddSceneObject(cameraObj);

				ThirdPersonCameraController* tpCam = new ThirdPersonCameraController();
				cameraObj->AddComponent(tpCam);

				tpCam->SetActive(false);	// 初期状態OFFに。

				CameraChangerComponent* camChanger = new CameraChangerComponent();
				cameraObj->AddComponent(camChanger);// CameraChangerをセット

				camChanger->SetCameraController(tpCam);	// CameraChangerに3人称カメラセット

				camChanger->ChangeCameraController(0);	// 初期状態を3人称カメラに

				engine->SetCameraData(cameraObj->GetCharacterData());

				// カメラ初期設定
				camComp->ChangeCameraRatio(engine->GetWidth(), engine->GetHeight());
				camComp->ChangeCameraDepth(0.01f, 1000.0f);
				camComp->ChangeCameraFOVRadian(DirectX::XMConvertToRadians(45.0f));

				camComp->ChangeCameraPosition(1.5f, 1.2f, 0.0f);

				// UnityちゃんにThirdPersonCameraCompを渡す。
				unityChanPlayer->SetThirdPersonCameraComponent(tpCam);

				// UnityChanPlayerにCameraComponentを登録
				unityChanPlayer->SetCurrentCamera(camComp);

				// タイマーUIのバックグラウンドの処理 PlayTimerBGUI
				cameraObj = new GameObject(new CharacterData());
				PlayTimerBGUI* playTimerBGUI = new PlayTimerBGUI();
				cameraObj->AddComponent(playTimerBGUI);
				AddSceneObject(cameraObj);

				// カメラリスト追加 UIモード時用
				m_cameraComponents[L"HUDCamera"] = playTimerBGUI;

				cameraObj = new GameObject(new CharacterData());
				WildHuntUIRender* uiRender = new WildHuntUIRender();
				cameraObj->AddComponent(uiRender);

				AddSceneObject(cameraObj);

				// カメラリスト追加 UIモード時用
				m_cameraComponents[L"HUDCamera"] = uiRender;

				// 剣の武器作成
				GameObject* playerWeaponObj;
				FBXCharacterData* playerWeaponFbx = new FBXCharacterData();		// FBX用CharacterData
				PlayerWeaponSword* playerWeaponSword = new PlayerWeaponSword();	// 剣の本体

				if (FAILED(fbxSys->LoadModelFBX(L"./Resources/fbx/Sword12_FBX_0706.fbx", L"WeaponSword")))
					return E_FAIL;
				playerWeaponFbx->SetMainFBX(L"WeaponSword");	// メインFBXの登録

				playerWeaponObj = new GameObject(playerWeaponFbx);
				playerWeaponObj->AddComponent(playerWeaponSword);

				AddSceneObject(playerWeaponObj);

				playerWeaponSword->SetThirdPersonCameraComponent(tpCam);		// カメラを揺らすように、ポインタを渡す
				unityChanPlayer->SetPlayerSwordComponent(playerWeaponSword);	// ユニティちゃん側から、剣の処理を呼び出すよね

				// ハートオブジェクト作成(3つ作成)
				FBXCharacterData* heartFbx = new FBXCharacterData();

				if (FAILED(fbxSys->LoadModelFBX(L"./Resources/fbx/Heart.fbx", L"GoldenHeart")))
					return E_FAIL;

				heartFbx->SetMainFBX(L"GoldenHeart");	// メインFBXの登録
				heartFbx->SetPosition(10.0f, 0.0f, 10.0f);

				GameObject* heartObj = new GameObject(heartFbx);
				heartObj->AddComponent(new HeartItemComponent());
				AddSceneObject(heartObj);

				heartFbx = new FBXCharacterData();
				heartFbx->SetMainFBX(L"GoldenHeart");	// メインFBXの登録
				heartFbx->SetPosition(15.0f, 0.0f, 5.0f);

				heartObj = new GameObject(heartFbx);
				heartObj->AddComponent(new HeartItemComponent());
				AddSceneObject(heartObj);

				heartFbx = new FBXCharacterData();
				heartFbx->SetMainFBX(L"GoldenHeart");	// メインFBXの登録
				heartFbx->SetPosition(10.0f, 3.0f, -10.0f);

				heartObj = new GameObject(heartFbx);
				heartObj->AddComponent(new HeartItemComponent());
				AddSceneObject(heartObj);

				// TurtleShellZakoEnemy登録
				FBXCharacterData* turtleShellZakoEnemyFbx = new FBXCharacterData();	// FBX用CharacterData

				// TurtleShellZakoEnemyのアニメーションFBXを読み込む
				fbxSys->LoadModelFBX(L"./Resources/fbx/TurtleShellMesh.fbx", L"TurtleShellZakoEnemy");
				fbxSys->LoadAnimationFBX(L"./Resources/fbx/TurtleShellAnime/IdleNormal_TurtleShell_Anim.fbx", L"IdleNormal");
				fbxSys->LoadAnimationFBX(L"./Resources/fbx/TurtleShellAnime/Die_TurtleShell_Anim.fbx", L"Die");
				fbxSys->LoadAnimationFBX(L"./Resources/fbx/TurtleShellAnime/Walk_TurtleShell_Anim.fbx", L"Walk");
				fbxSys->LoadAnimationFBX(L"./Resources/fbx/TurtleShellAnime/Attack01_TurtleShell_Anim.fbx", L"Attack01");

				turtleShellZakoEnemyFbx->SetMainFBX(L"TurtleShellZakoEnemy");	// メインFBXの登録
				turtleShellZakoEnemyFbx->SetPosition(-20.0f, 0.0f, -10.0f);

				GameObject* turtleShellZakoEnemyObj = new GameObject(turtleShellZakoEnemyFbx);
				TurtleShellZakoEnemy* turtleShellZakoEnemy = new TurtleShellZakoEnemy();
				turtleShellZakoEnemyObj->AddComponent(turtleShellZakoEnemy);	// 本体コンポーネントをセット
				AddSceneObject(turtleShellZakoEnemyObj);

				turtleShellZakoEnemy->SetUIRender(uiRender);
				unityChanPlayer->SetZakoEnemyComponentList(turtleShellZakoEnemy);

				turtleShellZakoEnemyFbx = new FBXCharacterData();
				turtleShellZakoEnemyFbx->SetMainFBX(L"TurtleShellZakoEnemy");	// メインFBXの登録
				turtleShellZakoEnemyFbx->SetPosition(-10.0f, 0.0f, -20.0f);

				turtleShellZakoEnemyObj = new GameObject(turtleShellZakoEnemyFbx);
				turtleShellZakoEnemy = new TurtleShellZakoEnemy();
				turtleShellZakoEnemyObj->AddComponent(turtleShellZakoEnemy);	// 本体コンポーネントをセット
				AddSceneObject(turtleShellZakoEnemyObj);

				turtleShellZakoEnemy->SetUIRender(uiRender);
				unityChanPlayer->SetZakoEnemyComponentList(turtleShellZakoEnemy);

				turtleShellZakoEnemyFbx = new FBXCharacterData();
				turtleShellZakoEnemyFbx->SetMainFBX(L"TurtleShellZakoEnemy");	// メインFBXの登録
				turtleShellZakoEnemyFbx->SetPosition(-30.0f, 0.0f, -10.0f);

				turtleShellZakoEnemyObj = new GameObject(turtleShellZakoEnemyFbx);
				turtleShellZakoEnemy = new TurtleShellZakoEnemy();
				turtleShellZakoEnemyObj->AddComponent(turtleShellZakoEnemy);	// 本体コンポーネントをセット
				AddSceneObject(turtleShellZakoEnemyObj);

				turtleShellZakoEnemy->SetUIRender(uiRender);
				unityChanPlayer->SetZakoEnemyComponentList(turtleShellZakoEnemy);

				turtleShellZakoEnemyFbx = new FBXCharacterData();
				turtleShellZakoEnemyFbx->SetMainFBX(L"TurtleShellZakoEnemy");	// メインFBXの登録
				turtleShellZakoEnemyFbx->SetPosition(10.0f, 0.0f, 0.0f);

				turtleShellZakoEnemyObj = new GameObject(turtleShellZakoEnemyFbx);
				turtleShellZakoEnemy = new TurtleShellZakoEnemy();
				turtleShellZakoEnemyObj->AddComponent(turtleShellZakoEnemy);	// 本体コンポーネントをセット
				AddSceneObject(turtleShellZakoEnemyObj);

				turtleShellZakoEnemy->SetUIRender(uiRender);
				unityChanPlayer->SetZakoEnemyComponentList(turtleShellZakoEnemy);

				// PlayerHPbarUI登録
				cameraObj = new GameObject(new CharacterData());
				PlayerHPbarUI* playerHPBar = new PlayerHPbarUI();
				cameraObj->AddComponent(playerHPBar);

				AddSceneObject(cameraObj);

				// カメラリスト追加 UIモード時用
				m_cameraComponents[L"HUDCamera"] = playerHPBar;

				playerHPBar->SetMaxPlHp(120.0f);		// ここでMaxHpを設定するよ
				unityChanPlayer->SetMaxPlHp(120.0f);	// unityChanPlayerにもMaxHpを設定します
				unityChanPlayer->SetPlayerHPbarUIComponent(playerHPBar);	// ついでに

				// NightmareDragon登録
				GameObject* nightmareDragonEnemyObj;	// まだnewしない。
				FBXCharacterData* nightmareDragonFbx = new FBXCharacterData();	// FBX用CharacterData
				NightmareDragonEnemy* nightmareDragonEnemy = new NightmareDragonEnemy();	// ドラゴン本体

				fbxSys->LoadModelFBX(L"./Resources/fbx/DragonTheNightmareMeshByMaya.fbx", L"NightmareDragon");
				fbxSys->LoadAnimationFBX(L"./Resources/fbx/NightmareDragonAnime/idle01ByMaya.fbx", L"ND_Idle");
				fbxSys->LoadAnimationFBX(L"./Resources/fbx/NightmareDragonAnime/walkByMaya.fbx", L"ND_Walk");
				fbxSys->LoadAnimationFBX(L"./Resources/fbx/NightmareDragonAnime/Claw_AttackByMaya.fbx", L"ND_ClawAttack");
				fbxSys->LoadAnimationFBX(L"./Resources/fbx/NightmareDragonAnime/Horn_AttackByMaya.fbx", L"ND_HornAttack");
				fbxSys->LoadAnimationFBX(L"./Resources/fbx/NightmareDragonAnime/screamByMaya.fbx", L"ND_Scream");
				fbxSys->LoadAnimationFBX(L"./Resources/fbx/NightmareDragonAnime/dieByMaya.fbx", L"ND_Die");
				fbxSys->LoadAnimationFBX(L"./Resources/fbx/NightmareDragonAnime/runByMaya.fbx", L"ND_Run");
				fbxSys->LoadAnimationFBX(L"./Resources/fbx/NightmareDragonAnime/getHitByMaya.fbx", L"ND_GetHit");
				fbxSys->LoadAnimationFBX(L"./Resources/fbx/NightmareDragonAnime/JumpByMaya.fbx", L"ND_Jump");
				fbxSys->LoadAnimationFBX(L"./Resources/fbx/NightmareDragonAnime/SleepByMaya.fbx", L"ND_Sleep");
				fbxSys->LoadAnimationFBX(L"./Resources/fbx/NightmareDragonAnime/FireBreathAttackByMaya.fbx", L"ND_BreatheAttack");

				nightmareDragonEnemyObj = new GameObject(nightmareDragonFbx);	// FBXCharacterDataを持たせて初期化
				nightmareDragonEnemyObj->AddComponent(nightmareDragonEnemy);	// 本体をコンポーネントをセット
				AddSceneObject(nightmareDragonEnemyObj);

				unityChanPlayer->SetNightmareDragonEnemyComponent(nightmareDragonEnemy);

				// ドラゴンのブレスを登録
				cameraObj = new GameObject(new CharacterData());
				FireBreatheEffect* fireBreathEff = new FireBreatheEffect();
				cameraObj->AddComponent(fireBreathEff);
				AddSceneObject(cameraObj);

				fireBreathEff->SetCameraComponent(camComp);
				nightmareDragonEnemy->SetFireBreatheEffectComponent(fireBreathEff);

				// プレイヤーのヒットエフェクトを登録
				cameraObj = new GameObject(new CharacterData());
				SwordHit2DEffect* swordHit2DEff = new SwordHit2DEffect();
				cameraObj->AddComponent(swordHit2DEff);
				AddSceneObject(cameraObj);

				swordHit2DEff->SetCameraComponent(camComp);
				playerWeaponSword->SetSwordHit2DEffectComponent(swordHit2DEff);

				// シーン遷移時のトランジションアニメーションを登録
				cameraObj = new GameObject(new CharacterData());
				TransitionAnimator* transitionAnimator = new TransitionAnimator();
				cameraObj->AddComponent(transitionAnimator);
				AddSceneObject(cameraObj);

				// カメラリスト追加 UIモード時用
				m_cameraComponents[L"HUDCamera"] = transitionAnimator;

				transitionAnimator->SetWipeMode(WipeMode::WipeIn);
				transitionAnimator->PlayTransition();


				engine->UploadCreatedTextures();
			}
			break;

		case static_cast<UINT>(GAME_SCENES::GAME_OVER):	// ゲームオーバー画面
			
			break;

		case static_cast<UINT>(GAME_SCENES::GAME_CLEAR):
			
			break;

		default:
			return E_FAIL;		// シーン指定がおかしい
		}

		m_scene = scene;

		engine->WaitForGpu();	// GPU待機（テクスチャアップロード等）
	}

	return hr;
}

void WildHuntScene::RemoveCamera(GameComponent* gc)
{
	std::wstring label = L"";
	for (auto cameraPair : m_cameraComponents)
	{
		if (cameraPair.second == gc)
		{
			label = cameraPair.first;
			break;
		}
	}

	if (label != L"")
	{
		RemoveCamera(label);
	}
}

void WildHuntScene::RemoveCamera(std::wstring label)
{
	if (m_cameraComponents [label] != nullptr)
	{
		m_cameraComponents.erase(label);
	}
}

void SceneObjectDeleter::ExecuteDeleter(GameObject* go)
{
	myScene->DeleteSceneObject(go); // Deleterを経由すればGameObjectの削除からこれが呼べる。
}
