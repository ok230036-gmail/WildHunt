#include "PlayTimerBGUI.h"

#include <MyAccessHub.h>
#include <D3D12Helper.h>

#include "WildHuntEnum.h"
#include "WildHuntScene.h"

// コンポーネント初期化時に呼ばれる処理
void PlayTimerBGUI::InitAction()
{
	// m_timerBGSpの初期設定
	m_timerBGSp = std::make_unique<SpriteCharacter>();

	m_timerBGSp->SetCameraLabel(L"HUDCamera");
	m_timerBGSp->SetGraphicsPipeLine(L"Sprite");
	m_timerBGSp->SetTextureId(L"PlayTimerBG");

	Texture2DContainer* tex = MyAccessHub::GetMyGameEngine()->GetTextureManager()->GetTexture(m_timerBGSp->GetTextureId());

	float invH = 1.0f / tex->fHeight;

	XMFLOAT4 r = { 0.0f, 0.0f, 1.0f, 1.0f };

	m_timerBGSp->SetSpritePattern(0, tex->fWidth * 1.0f, tex->fHeight * 1.0f, r);
	m_timerBGSp->SetSpriteIndex(0);
	m_timerBGSp->SetPosition(340.0f, 200.0f, 2.0f);
	m_timerBGSp->SetScale(0.15f, 0.15f, 0.15f);

	m_timerBGSp->SetColorMix(SpriteCharacter::COLOR_MIX_OP::MIX_MUL);
	m_timerBGSp->SetColor(1, 1, 1, 1);

	MyGameEngine* engine = MyAccessHub::GetMyGameEngine();
	CharacterData* chData = GetGameObject()->GetCharacterData();

	engine->InitCameraConstantBuffer(chData);

	// マトリクスを固定で作成
	XMVECTOR Eye = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);		// 視点座標
	XMVECTOR At = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);		// カメラが向く座標
	XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);		// カメラの上方向単位ベクトル
	XMMATRIX view = XMMatrixTranspose(MakeViewMatix(Eye, At, Up));
	XMMATRIX proj = XMMatrixTranspose(MakeOrthographicPrjectionMatrix(engine->GetWidth(), engine->GetHeight(), 0.01f, 3.0f));

	engine->UpdateShaderResourceOnGPU(chData->GetConstantBuffer(0), &view, sizeof(XMMATRIX));
	engine->UpdateShaderResourceOnGPU(chData->GetConstantBuffer(1), &proj, sizeof(XMMATRIX));
}

// 毎フレーム呼ばれる処理　falseを返すとこのコンポーネントは終了し削除される
bool PlayTimerBGUI::FrameAction()
{
	MyGameEngine* engine = MyAccessHub::GetMyGameEngine();
	GraphicsPipeLineObjectBase* spritePL = engine->GetPipelineManager()->GetPipeLineObject(L"Sprite");

	// PipeLineに登録
	spritePL->AddRenerObject(m_timerBGSp.get());
	return true;
}

// 終了時に呼ばれる処理
void PlayTimerBGUI::FinishAction()
{
}
