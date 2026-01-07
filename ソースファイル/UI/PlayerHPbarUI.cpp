#include "PlayerHPbarUI.h"

#include <MyAccessHub.h>
#include <D3D12Helper.h>

#include "WildHuntEnum.h"
#include "WildHuntScene.h"

// コンポーネント初期化時に呼ばれる処理
void PlayerHPbarUI::InitAction()
{
	// 3種類の画像の初期化
	m_bgSp = std::make_unique<SpriteCharacter>();
	m_redSp = std::make_unique<SpriteCharacter>();
	m_greenSp = std::make_unique<SpriteCharacter>();

	m_bgSp->SetCameraLabel(L"HUDCamera");
	m_redSp->SetCameraLabel(L"HUDCamera");
	m_greenSp->SetCameraLabel(L"HUDCamera");

	m_bgSp->SetGraphicsPipeLine(L"Sprite");
	m_redSp->SetGraphicsPipeLine(L"Sprite");
	m_greenSp->SetGraphicsPipeLine(L"Sprite");

	m_bgSp->SetTextureId(L"HPBarTexture");
	m_redSp->SetTextureId(L"HPBarTexture");
	m_greenSp->SetTextureId(L"HPBarTexture");

	Texture2DContainer* tex = MyAccessHub::GetMyGameEngine()->GetTextureManager()->GetTexture(m_bgSp->GetTextureId());

	float invH = 1.0f / tex->fHeight;

	XMFLOAT4 r = { 0.0f, 0.0f, 1.0f, 150.0f * invH };

	m_bgSp->SetSpritePattern(0, tex->fWidth, 150.0f, r);
	m_bgSp->SetSpriteIndex(0);
	m_bgSp->SetPosition(-230.0f, 220.0f, 0.0f);

	r = { 0.0f, 305.0f * invH, 0.95f, 150.0f * invH };	// 現状の画像だとHPマックスは0.95fで、0.24f以下は死亡判定かな 赤ゲージは305

	m_redSp->SetSpritePattern(0, tex->fWidth * 0.95f, 150.0f, r);
	m_redSp->SetSpriteIndex(0);
	m_redSp->SetPosition(-230.0f + -260.0f * (1.0f - 0.95f), 220.0f, 0.0f);

	r = { 0.0f, 160.0f * invH, 0.95f, 150.0f * invH};	// 現状の画像だとHPマックスは0.95fで、0.24f以下は死亡判定かな 赤ゲージは305

	m_greenSp->SetSpritePattern(0, tex->fWidth * 0.95f, 150.0f, r);
	m_greenSp->SetSpriteIndex(0);
	m_greenSp->SetPosition(-230.0f + -260.0f * (1.0f - 0.95f), 220.0f, 0.0f);

	m_bgSp->SetColorMix(SpriteCharacter::COLOR_MIX_OP::MIX_MUL);
	m_bgSp->SetColor(1, 1, 1, 1);

	m_redSp->SetColorMix(SpriteCharacter::COLOR_MIX_OP::MIX_MUL);
	m_redSp->SetColor(1, 1, 1, 1);

	m_greenSp->SetColorMix(SpriteCharacter::COLOR_MIX_OP::MIX_MUL);
	m_greenSp->SetColor(1, 1, 1, 1);

	m_hpChangeFlg = false;
	m_isRedDecrease = false;
	m_nowUvHp = 0.95f;
	m_redUvHp = 0.95f;

	MyGameEngine* engine = MyAccessHub::GetMyGameEngine();
	CharacterData* chData = GetGameObject()->GetCharacterData();

	engine->InitCameraConstantBuffer(chData);

	chData->SetPosition(0.0f, 0.0f, 0.0f);

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
bool PlayerHPbarUI::FrameAction()
{
	MyGameEngine* engine = MyAccessHub::GetMyGameEngine();
	GraphicsPipeLineObjectBase* spritePL = engine->GetPipelineManager()->GetPipeLineObject(L"Sprite");
	
	// Hpが変化した時のみ呼び出すよ
	if (m_hpChangeFlg)
	{
		// 現状の画像だとHPマックスは0.95fで、0.24f以下は死亡判定かな 赤ゲージは305
		float start = 0.24f;
		float end = 0.95f;

		// 線形補完
		m_nowUvHp = start + (m_nowPlHp / m_maxPlHp) * (end - start);

		Texture2DContainer* tex = MyAccessHub::GetMyGameEngine()->GetTextureManager()->GetTexture(m_bgSp->GetTextureId());
		float invH = 1.0f / tex->fHeight;

		XMFLOAT4 r = { 0.0f, 160.0f * invH, m_nowUvHp, 150.0f * invH };	// 現状の画像だとHPマックスは0.95fで、0.24f以下は死亡判定かな 赤ゲージは305

		m_greenSp->SetSpritePattern(0, tex->fWidth * m_nowUvHp, 150.0f, r);
		m_greenSp->SetSpriteIndex(0);
		m_greenSp->SetPosition(-230.0f + -260.0f * (1.0f - m_nowUvHp), 220.0f, 0.0f);

		m_hpChangeFlg = false;
		m_isRedDecrease = true;
	}
	
	// 赤いバーを少しずつ減少させる
	if (m_isRedDecrease)
	{
		if (m_nowUvHp < m_redUvHp)
		{
			m_redUvHp -= 0.0015f;

			Texture2DContainer* tex = MyAccessHub::GetMyGameEngine()->GetTextureManager()->GetTexture(m_bgSp->GetTextureId());
			float invH = 1.0f / tex->fHeight;

			XMFLOAT4 r = { 0.0f, 305.0f * invH, m_redUvHp, 150.0f * invH };	// 現状の画像だとHPマックスは0.95fで、0.24f以下は死亡判定かな 赤ゲージは305

			m_redSp->SetSpritePattern(0, tex->fWidth * m_redUvHp, 150.0f, r);
			m_redSp->SetSpriteIndex(0);
			m_redSp->SetPosition(-230.0f + -260.0f * (1.0f - m_redUvHp), 220.0f, 0.0f);
		}
		else
		{
			m_isRedDecrease = false;
			m_redUvHp = m_nowUvHp;
		}
	}

	// 画像を描画のため、PipeLineに登録
	spritePL->AddRenerObject(m_bgSp.get());
	spritePL->AddRenerObject(m_redSp.get());
	spritePL->AddRenerObject(m_greenSp.get());

    return true;
}

// 終了時に呼ばれる処理
void PlayerHPbarUI::FinishAction()
{
}

// HPの数値から詳細な調整をする
int PlayerHPbarUI::ConvertHpToUvValue(float nowHp)
{
	// 現状の画像だとHPマックスは0.95fで、0.24f以下は死亡判定かな
	float start = 0.24f;
	float end = 0.95f;

	m_nowUvHp = start + (nowHp / m_maxPlHp) * (end - start);
	// 線形補完
	return start + (nowHp / m_maxPlHp) * (end - start);
}