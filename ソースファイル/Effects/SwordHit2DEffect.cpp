#include "SwordHit2DEffect.h"
#include "FBXCharacterData.h"

#include <MyAccessHub.h>
#include <D3D12Helper.h>

#include "WildHuntEnum.h"
#include "WildHuntScene.h"

// コンポーネント初期化時に呼ばれる処理
void SwordHit2DEffect::InitAction()
{
	// m_hitEffSpの初期設定
	m_hitEffSp = std::make_unique<SpriteCharacter>();

	m_hitEffSp->SetCameraLabel(L"MainCamera");
	m_hitEffSp->SetGraphicsPipeLine(L"Sprite");
	m_hitEffSp->SetTextureId(L"HitEffect");

	// エフェクトの初期UVの設定(描画はしない)
	Texture2DContainer* tex = MyAccessHub::GetMyGameEngine()->GetTextureManager()->GetTexture(m_hitEffSp->GetTextureId());

	float invH = 1.0f / tex->fHeight;
	XMFLOAT4 r = { 0.0f, 0.0f, invH * 32.0f, invH * 32.0f};

	m_hitEffSp->SetSpritePattern(0, tex->fWidth * 1.0f, tex->fHeight * 1.0f, r);
	m_hitEffSp->SetSpriteIndex(0);
	m_hitEffSp->SetPosition(0.0f, 5.0f, 0.0f);
	m_hitEffSp->SetRotation(0.0f, 90.0f, 0.0f);
	m_hitEffSp->SetScale(0.005f, 0.005f, 0.005f);


	m_hitEffSp->SetColorMix(SpriteCharacter::COLOR_MIX_OP::MIX_MUL);
	m_hitEffSp->SetColor(1, 1, 1, 1);

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
bool SwordHit2DEffect::FrameAction()
{
	MyGameEngine* engine = MyAccessHub::GetMyGameEngine();
	GraphicsPipeLineObjectBase* spritePL = engine->GetPipelineManager()->GetPipeLineObject(L"Sprite");

	FBXCharacterData* efData =
		static_cast<FBXCharacterData*>(GetGameObject()->GetCharacterData());

	CharacterData* camData = m_camera->GetGameObject()->GetCharacterData();

	XMFLOAT3 camPos = camData->GetPosition();

	XMFLOAT3 efPos = m_hitEffSp->GetPosition();


	// プレイヤーの方を向く
	float charRad = atan2f(camPos.x - efPos.x, camPos.z - efPos.z);

	// 回転の設定値はDegree
	m_hitEffSp->SetRotation(0.0f, XMConvertToDegrees(charRad) + 180.0f, 0.0f);

	if (m_isEffFlag)
	{
		m_hitEffSp->SetPosition(m_hitPos.x, m_hitPos.y + 1.2f, m_hitPos.z);

		m_frameCount++;
		if (m_frameCount > 2)	// 連続してアニメーションを繰り返し続けると見ずらいので
		{
			m_frameCount = 0;
			m_animeCount++;

			if (m_animeCount >= 27)	// 最後の画像までいったので、エフェクトは非描画
			{
				m_isEffFlag = false;
			}

			m_animeCount %= 28;
		}

		Texture2DContainer* tex = MyAccessHub::GetMyGameEngine()->GetTextureManager()->GetTexture(m_hitEffSp->GetTextureId());

		float invH = 1.0f / tex->fHeight;

		// U計算
		float u = (m_animeCount % 4) * 146.3f;
		// v計算
		float v = (m_animeCount / 4) * 146.3f;

		XMFLOAT4 r = { invH * u, invH * v, invH * 146.3f, invH * 146.3f };

		m_hitEffSp->SetSpritePattern(0, tex->fWidth * 1.0f, tex->fHeight * 1.0f, r);
		m_hitEffSp->SetSpriteIndex(0);

		// PipeLineに登録
		spritePL->AddRenerObject(m_hitEffSp.get());
	}

	return true;
}

// 終了時に呼ばれる処理
void SwordHit2DEffect::FinishAction()
{
}
