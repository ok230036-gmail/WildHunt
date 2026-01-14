#include "GameOverScene.h"

#include <MyAccessHub.h>
#include <D3D12Helper.h>

#include "WildHuntScene.h"
#include "KeyBindComponent.h"

// コンストラクタ
GameOverScene::GameOverScene()
{
	// m_titleImageSpの初期設定
	m_gameOverImageSp = std::make_unique<SpriteCharacter>();

	m_gameOverImageSp->SetCameraLabel(L"TitleCamera");
	m_gameOverImageSp->SetGraphicsPipeLine(L"AlphaSprite");
	m_gameOverImageSp->SetTextureId(L"GameOver");



	m_gameOverImageSp->SetColorMix(SpriteCharacter::COLOR_MIX_OP::MIX_MUL);
	m_gameOverImageSp->SetColor(1, 1, 1, 1);
}

void GameOverScene::SetNextScene(GAME_SCENES nextSc)
{
	m_nextScene = nextSc;
}

// コンポーネント初期化時に呼ばれる処理
void GameOverScene::InitAction()
{
	MyGameEngine* engine = MyAccessHub::GetMyGameEngine();
	CharacterData* chData = GetGameObject()->GetCharacterData();

	engine->InitCameraConstantBuffer(chData);

	Texture2DContainer* tex = MyAccessHub::GetMyGameEngine()->GetTextureManager()->GetTexture(m_gameOverImageSp->GetTextureId());

	float invH = 1.0f / tex->fHeight;

	XMFLOAT4 r = { 0.0f, 0.0f, 1.0f, 1.0f };

	m_gameOverImageSp->SetSpritePattern(0, tex->fWidth * 1.0f, tex->fHeight * 1.0f, r);
	m_gameOverImageSp->SetSpriteIndex(0);
	m_gameOverImageSp->SetPosition(0.0f, -220.0f, 0.0f);
	m_gameOverImageSp->SetScale(0.96f, 0.96f, 0.96f);	// 現在の画像に合わせて微調整

	// マトリクスを固定で作成
	XMVECTOR Eye = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);		// 視点座標
	XMVECTOR At = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);		// フォーカスする座標
	XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);		// カメラの上方向単位ベクトル
	XMMATRIX view = XMMatrixTranspose(MakeViewMatix(Eye, At, Up));
	XMMATRIX proj = XMMatrixTranspose(MakeOrthographicPrjectionMatrix(engine->GetWidth(), engine->GetHeight(), 0.01f, 3.0f));

	engine->UpdateShaderResourceOnGPU(chData->GetConstantBuffer(0), &view, sizeof(XMMATRIX));
	engine->UpdateShaderResourceOnGPU(chData->GetConstantBuffer(1), &proj, sizeof(XMMATRIX));

	// ゲームオーバーサウンドの初期化
	m_isPlayGOSound = true;
}

// 毎フレーム呼ばれる処理　falseを返すとこのコンポーネントは終了し削除される
bool GameOverScene::FrameAction()
{
	MyGameEngine* engine = MyAccessHub::GetMyGameEngine();
	GraphicsPipeLineObjectBase* normalPL = engine->GetPipelineManager()->GetPipeLineObject(L"Sprite");
	GraphicsPipeLineObjectBase* alphaPL = engine->GetPipelineManager()->GetPipeLineObject(L"AlphaSprite");

	WildHuntScene* scene = static_cast<WildHuntScene*>(engine->GetSceneController());
	KeyBindComponent* keyBind = static_cast<KeyBindComponent*>(scene->GetKeyComponent());

	MyGameEngine* pEngine = MyAccessHub::GetMyGameEngine();

	switch (m_gameOverState)
	{
	case SceneState::SceneImage:
		// 画面遷移が終了したら、一回だけゲームオーバーSEを鳴らす
		if (m_pTransitionAnimatorComp->IsTransitionFinished() && m_isPlayGOSound)
		{
			pEngine->GetSoundManager()->Play(12);
			m_isPlayGOSound = false;
		}

		if (keyBind->GetCurrentInputState(InputManager::BUTTON_STATE::BUTTON_DOWN, KeyBindComponent::BUTTON_IDS::BTN_JUMP))
		{
			// Spaceキーが押されたら、画面遷移のトランジションをする
			m_pTransitionAnimatorComp->SetWipeMode(WipeMode::WipeOut);
			m_pTransitionAnimatorComp->PlayTransition();
			m_gameOverState = SceneState::Transition;
		}
		break;

	case SceneState::Transition:
		if (m_pTransitionAnimatorComp->IsTransitionFinished())
		{
			// 画面遷移のトランジションが終了したら、Loading処理へ
			m_gameOverState = SceneState::Loading;
		}

		break;
	case SceneState::Loading:

		// シーン切り替え呼び出し
		MyAccessHub::GetMyGameEngine()->GetSceneController()->OrderNextScene((UINT)m_nextScene);
		break;
	}

	// PipeLineに登録
	alphaPL->AddRenerObject(m_gameOverImageSp.get());

	return true;

}

// 終了時に呼ばれる処理
void GameOverScene::FinishAction()
{
	WildHuntScene* scene = static_cast<WildHuntScene*>(MyAccessHub::GetMyGameEngine()->GetSceneController());
	scene->RemoveCamera(this);
}
