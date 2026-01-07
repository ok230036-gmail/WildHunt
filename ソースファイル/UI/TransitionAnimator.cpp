#include "TransitionAnimator.h"
#include "FBXCharacterData.h"

#include <MyAccessHub.h>
#include <D3D12Helper.h>

#include "WildHuntEnum.h"
#include "WildHuntScene.h"

// コンポーネント初期化時に呼ばれる処理
void TransitionAnimator::InitAction()
{
	// m_transitionAnimSpの初期設定
	m_transitionAnimSp = std::make_unique<SpriteCharacter>();

	m_transitionAnimSp->SetCameraLabel(L"HUDCamera");
	m_transitionAnimSp->SetGraphicsPipeLine(L"Sprite");
	m_transitionAnimSp->SetTextureId(L"Transition");

	Texture2DContainer* tex = MyAccessHub::GetMyGameEngine()->GetTextureManager()->GetTexture(m_transitionAnimSp->GetTextureId());

	float invH = 1.0f / tex->fHeight;

	XMFLOAT4 r = { 0.0f, 0.0f, invH * 256.0f, invH * 256.0f };

	m_transitionAnimSp->SetSpritePattern(0, tex->fWidth * 1.0f, tex->fHeight * 1.0f, r);
	m_transitionAnimSp->SetSpriteIndex(0);
	m_transitionAnimSp->SetScale(1.0f, 0.55f, 0.1f);	//この画像が合うように微調整

	m_transitionAnimSp->SetColorMix(SpriteCharacter::COLOR_MIX_OP::MIX_MUL);
	m_transitionAnimSp->SetColor(1, 1, 1, 1);

	MyGameEngine* engine = MyAccessHub::GetMyGameEngine();
	CharacterData* chData = GetGameObject()->GetCharacterData();

	engine->InitCameraConstantBuffer(chData);

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
bool TransitionAnimator::FrameAction()
{
	MyGameEngine* engine = MyAccessHub::GetMyGameEngine();
	GraphicsPipeLineObjectBase* spritePL = engine->GetPipelineManager()->GetPipeLineObject(L"Sprite");

	FBXCharacterData* efData =
		static_cast<FBXCharacterData*>(GetGameObject()->GetCharacterData());

	
	if (m_isAnimFlag)
	{
		m_frameCount++;
		if (m_frameCount > 3)	// アニメーションの変化は3fごとに
		{
			m_frameCount = 0;
			m_animeCount++;

			// m_nowWipeModeによって再生するアニメーションを変更
			switch (m_nowWipeMode)
			{
			case WipeMode::WipeIn:
				if(m_animeCount == 13)
					engine->GetSoundManager()->Play(9);

				if (m_animeCount >= 23)
				{
					m_animeCount = 23;
					m_isAnimFlag = false;
				}
				break;
			case WipeMode::WipeOut:
				if(m_animeCount == 1)
					engine->GetSoundManager()->Play(10);

				if (m_animeCount >= 11)
				{
					m_animeCount = 11;
					m_isAnimFlag = false;
				}
				break;
			}
		}
	}

	Texture2DContainer* tex = MyAccessHub::GetMyGameEngine()->GetTextureManager()->GetTexture(m_transitionAnimSp->GetTextureId());

	float invH = 1.0f / tex->fHeight;

	// U計算
	float u = (m_animeCount % 4) * 256.0f;
	// v計算
	float v = (m_animeCount / 4) * 144.0f;

	XMFLOAT4 r = { invH * u, invH * v, invH * 256.0f, invH * 144.0f };

	m_transitionAnimSp->SetSpritePattern(0, tex->fWidth * 1.0f, tex->fHeight * 1.0f, r);
	m_transitionAnimSp->SetSpriteIndex(0);

	// PipeLineに登録
	spritePL->AddRenerObject(m_transitionAnimSp.get());

	return true;
}

// 終了時に呼ばれる処理
void TransitionAnimator::FinishAction()
{
}
