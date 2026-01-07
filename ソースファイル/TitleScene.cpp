#include "TitleScene.h"

#include <MyAccessHub.h>
#include <D3D12Helper.h>

#include "WildHuntScene.h"
#include "KeyBindComponent.h"

// コンストラクタ
TitleScene::TitleScene()
{
	// m_bgSpとm_messageSpの初期設定
	m_bgSp = std::make_unique<SpriteCharacter>();
	m_titleImageSp = std::make_unique<SpriteCharacter>();

	m_bgSp->SetCameraLabel(L"TitleCamera");
	m_titleImageSp->SetCameraLabel(L"TitleCamera");

	m_bgSp->SetGraphicsPipeLine(L"Sprite");
	m_titleImageSp->SetGraphicsPipeLine(L"AlphaSprite");

	m_bgSp->SetTextureId(L"TitleTexture");
	m_titleImageSp->SetTextureId(L"TitleTexture");

	// BGは左上にある2*2ドット部分の白色を色変化をつけて使う
	Texture2DContainer* tex = MyAccessHub::GetMyGameEngine()->GetTextureManager()->GetTexture(m_bgSp->GetTextureId());
	float invH = 1.0f / tex->fHeight;
	XMFLOAT4 r = { 0.0f, 0.0f, 1.0f * invH, 1.0f * invH};

	float h = MyAccessHub::GetMyGameEngine()->GetHeight();
	float w = MyAccessHub::GetMyGameEngine()->GetWidth();

	m_bgSp->SetSpritePattern(0, w, h, r);
	m_bgSp->SetSpriteIndex(0);


	m_titleImageSp->SetColorMix(SpriteCharacter::COLOR_MIX_OP::MIX_MUL);
	m_titleImageSp->SetColor(1, 1, 1, 1);
}

void TitleScene::SetBGColor(float r, float g, float b)
{
	m_bgSp->SetColorMix(SpriteCharacter::COLOR_MIX_OP::MIX_MUL);
	m_bgSp->SetColor(r, g, b, 1.0f);
}

void TitleScene::SetImagePosition(float pxY)
{
	Texture2DContainer* tex = MyAccessHub::GetMyGameEngine()->GetTextureManager()->GetTexture(m_titleImageSp->GetTextureId());

	float invH = 1.0f / tex->fHeight;

	XMFLOAT4 r = { 0.0f, 0.0f, 1.0f, 1.0f };

	m_titleImageSp->SetSpritePattern(0, tex->fWidth * 1.0f, tex->fHeight * 1.0f, r);
	m_titleImageSp->SetSpriteIndex(0);
	m_titleImageSp->SetScale(0.48f, 0.48f, 0.48f);	// 現在の画像に合わせて微調整
}

void TitleScene::SetNextScene(GAME_SCENES nextSc)
{
	m_nextScene = nextSc;
}

// コンポーネント初期化時に呼ばれる処理
void TitleScene::InitAction()
{
	MyGameEngine* engine = MyAccessHub::GetMyGameEngine();
	CharacterData* chData = GetGameObject()->GetCharacterData();

	engine->InitCameraConstantBuffer(chData);

	chData->SetPosition(0.0f, 0.0f, 0.0f);

	// マトリクスを固定で作成
	XMVECTOR Eye = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);		// 視点座標
	XMVECTOR At = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);		// フォーカスする座標
	XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);		// カメラの上方向単位ベクトル
	XMMATRIX view = XMMatrixTranspose(MakeViewMatix(Eye, At, Up));
	XMMATRIX proj = XMMatrixTranspose(MakeOrthographicPrjectionMatrix(engine->GetWidth(), engine->GetHeight(), 0.01f, 3.0f));

	engine->UpdateShaderResourceOnGPU(chData->GetConstantBuffer(0), &view, sizeof(XMMATRIX));
	engine->UpdateShaderResourceOnGPU(chData->GetConstantBuffer(1), &proj, sizeof(XMMATRIX));
}

// 毎フレーム呼ばれる処理　falseを返すとこのコンポーネントは終了し削除される
bool TitleScene::FrameAction()
{
	MyGameEngine* engine = MyAccessHub::GetMyGameEngine();
	GraphicsPipeLineObjectBase* normalPL = engine->GetPipelineManager()->GetPipeLineObject(L"Sprite");
	GraphicsPipeLineObjectBase* alphaPL = engine->GetPipelineManager()->GetPipeLineObject(L"AlphaSprite");

	WildHuntScene* scene = static_cast<WildHuntScene*>(engine->GetSceneController());
	KeyBindComponent* keyBind = static_cast<KeyBindComponent*>(scene->GetKeyComponent());

	switch (m_titleState)
	{
	case TitleState::TitleImage:
		if (keyBind->GetCurrentInputState(InputManager::BUTTON_STATE::BUTTON_DOWN, KeyBindComponent::BUTTON_IDS::BTN_JUMP))
		{
			// Spaceキーが押されたら、画面遷移のトランジションをする
			m_pTransitionAnimatorComp->PlayTransition();
			m_titleState = TitleState::Transition;
		}
		break;

	case TitleState::Transition:
		if (m_pTransitionAnimatorComp->IsTransitionFinished())
		{
			// 画面遷移のトランジションが終了したら、Loading処理へ
			m_titleState = TitleState::Loading;
		}
		
		break;
	case TitleState::Loading:
		
		// シーン切り替え呼び出し
		MyAccessHub::GetMyGameEngine()->GetSceneController()->OrderNextScene((UINT)m_nextScene);
		break;
	}

	// PipeLineに登録
	normalPL->AddRenerObject(m_bgSp.get());
	alphaPL->AddRenerObject(m_titleImageSp.get());

    return true;
}

// 終了時に呼ばれる処理
void TitleScene::FinishAction()
{
	WildHuntScene* scene = static_cast<WildHuntScene*>(MyAccessHub::GetMyGameEngine()->GetSceneController());
	scene->RemoveCamera(this);
}
