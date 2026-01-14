#include "ParchmentResultUI.h"
#include "FBXCharacterData.h"

#include <MyAccessHub.h>
#include <D3D12Helper.h>

#include "WildHuntEnum.h"
#include "WildHuntScene.h"
#include "UnityChanPlayer.h"

// 座標、文字列、色とかを設定
int ParchmentResultUI::MakeSpriteString(std::vector<std::unique_ptr<SpriteCharacter>>& sprites, int startIndex, float ltX, float ltY, float width, float height, const char* str)
{
	int count = startIndex;

	while (*str != '\0')
	{
		if (std::find(m_chList, m_chEnd, *str) != m_chEnd)
		{
			sprites[count]->SetSpritePattern(0, width, height, m_fontMap[*str]);
			sprites[count]->SetSpriteIndex(0);

			sprites[count]->SetPosition(ltX, ltY, 0.0f);
			count++;
		}

		ltX += width;

		str++;
	}

	return count;
}


// 画像の初期設定(リザルト画面で表示する画像が増えすぎたから、纏めたい)
void ParchmentResultUI::SetUpSprites(std::unique_ptr<SpriteCharacter>& sprite, const wchar_t* texId)
{
	sprite = std::make_unique<SpriteCharacter>();

	sprite->SetCameraLabel(L"HUDCamera");
	sprite->SetGraphicsPipeLine(L"Sprite");
	sprite->SetTextureId(texId);

	sprite->SetColorMix(SpriteCharacter::COLOR_MIX_OP::MIX_MUL);
	sprite->SetColor(1, 1, 1, 1);
}

const char* ParchmentResultUI::CalcOverallRank()
{
	float timeSec = (float)m_pGameTimerUIComp->GetGameTimeSeconds();
	float damageRate = m_pUnityChanPlayerComp->GetTotalDamageRate();

	float timeScore = std::exp(-timeSec / 240.0f);   // 240.0fは想定基準値
	float damageScore = 1.0f / (1.0f + damageRate * 3.0f); // 0～1

	float total = timeScore * 0.7f + damageScore * 0.3f;

	if (total >= 0.85f) return "S";
	if (total >= 0.65f) return "A";
	if (total >= 0.45f) return "B";
	if (total >= 0.25f) return "C";
	return "D";
}



// コンポーネント初期化時に呼ばれる処理
void ParchmentResultUI::InitAction()
{
	// リザルト画面用のSpriteを初期設定(リザルト画面に表示する画像が多いから関数に纏める)
	SetUpSprites(m_parchmentResultSp, L"ParchmentResult");
	SetUpSprites(m_clearTimeSp, L"ClearTime");
	SetUpSprites(m_totalDamageSp, L"TotalDamage");
	SetUpSprites(m_overallRankSp, L"OverallRank");

	m_parchmentResultSp->SetPosition(0.0f, -20.0f, 1.0f);	// 他の画像を上書きしないため、ちょっと奥に表示。
	m_parchmentResultSp->SetScale(0.7f, 0.36f, 0.36f);


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

	m_spriteCount = 50;

	SpriteCharacter* spc;

	for (int i = 0; i < m_spriteCount; i++)
	{
		spc = new SpriteCharacter();

		spc->SetTextureId(L"HUDTexture");
		spc->SetCameraLabel(L"HUDCamera");

		spc->SetColor(1, 1, 1, 1);

		spc->SetGraphicsPipeLine(L"Sprite");

		m_clearTimeStrSp.push_back(std::unique_ptr<SpriteCharacter>(spc));

		spc = new SpriteCharacter();

		spc->SetTextureId(L"HUDTexture");
		spc->SetCameraLabel(L"HUDCamera");

		spc->SetColor(1, 1, 1, 1);

		spc->SetGraphicsPipeLine(L"Sprite");
		m_totalDamageStrSp.push_back(std::unique_ptr<SpriteCharacter>(spc));

		spc = new SpriteCharacter();

		spc->SetTextureId(L"HUDTexture");
		spc->SetCameraLabel(L"HUDCamera");

		spc->SetColor(1, 1, 1, 1);

		spc->SetGraphicsPipeLine(L"Sprite");
		m_overallRankStrSp.push_back(std::unique_ptr<SpriteCharacter>(spc));

		spc = new SpriteCharacter();

		spc->SetTextureId(L"HUDTexture");
		spc->SetCameraLabel(L"HUDCamera");

		spc->SetColor(1, 1, 1, 1);

		spc->SetGraphicsPipeLine(L"Sprite");
		m_pressSpaceToTitleStrSp.push_back(std::unique_ptr<SpriteCharacter>(spc));
	}

	// FontMap
	Texture2DContainer* tex = engine->GetTextureManager()->GetTexture(L"HUDTexture");

	m_fontMap.reserve(strlen(m_chList));

	m_chEnd = m_chList + strlen(m_chList) * sizeof(m_chList[0]);

	int index = 0;
	float invW = 1.0f / tex->fWidth;
	float invH = 1.0f / tex->fHeight;

	float x = 0.0f;
	float y = 0.0f;
	float w = 32.0f;	// フォントサイズ
	float h = 56.0f;	// 等幅フォントだと計算簡単
	while (m_chList[index] != '\0')
	{
		XMFLOAT4 r = { x * invW, y * invH, w * invW, h * invH };
		m_fontMap[m_chList[index]] = r;
		x += 32.0f;

		if (x >= tex->fWidth)
		{
			x = 0.0f;
			y += h;
		}

		index++;
	}
}

// 毎フレーム呼ばれる処理　falseを返すとこのコンポーネントは終了し削除される
bool ParchmentResultUI::FrameAction()
{
	MyGameEngine* engine = MyAccessHub::GetMyGameEngine();
	GraphicsPipeLineObjectBase* spritePL = engine->GetPipelineManager()->GetPipeLineObject(L"Sprite");

	FBXCharacterData* efData =
		static_cast<FBXCharacterData*>(GetGameObject()->GetCharacterData());

	WildHuntScene* scene = static_cast<WildHuntScene*>(engine->GetSceneController());
	KeyBindComponent* keyBind = static_cast<KeyBindComponent*>(scene->GetKeyComponent());

	m_isAnimFlag = true;
	// Animフラグがtrueになったら、UI表示
	if (m_defeatCnt >= m_enemyCnt)
	//if(m_isAnimFlag)
	{
		m_frameCount++;
		if (m_frameCount > 3)
		{
			m_frameCount = 0;
			m_animeCount++;		// animeCountが進むごとに、画像を進める


			if (m_animeCount >= 52)
				m_animeCount = 52;
		}

		if (keyBind->GetCurrentInputState(InputManager::BUTTON_STATE::BUTTON_DOWN, KeyBindComponent::BUTTON_IDS::BTN_JUMP))
		{
			m_animeCount = 52;
		}

		if(m_animeCount == 1)
			engine->GetSoundManager()->Play(13);

		if (m_animeCount > 15)
		{
			string clearTimeStr = m_pGameTimerUIComp->GetGameTimeStr();
			const char* str = clearTimeStr.c_str();

			int count = 0;
			float x = 120.0f;	// 中心は 0, 0, w960, h540
			float y = 120.0f;

			// 座標、文字列、色とかを設定できる
			count = MakeSpriteString(m_clearTimeStrSp, count, x, y, 32, 40, str);

			// 使ったSpriteCharacterだけをパイプラインに登録
			for (int i = 0; i < count; i++)
			{
				// 文字列をパイプラインに登録
				spritePL->AddRenerObject(m_clearTimeStrSp[i].get());
			}

			// クリアタイム画像の表示
			Texture2DContainer* tex = MyAccessHub::GetMyGameEngine()->GetTextureManager()->GetTexture(m_clearTimeSp->GetTextureId());
			float invH = 1.0f / tex->fHeight;

			XMFLOAT4 r = { 0.0f, 0.0f, 1.0f, 200.0f * invH };

			m_clearTimeSp->SetSpritePattern(0, tex->fWidth, 200.0f, r);
			m_clearTimeSp->SetSpriteIndex(0);

			m_clearTimeSp->SetPosition(-150.0f, 120.0f, 0.0f);
			m_clearTimeSp->SetScale(0.5f, 0.5f, 0.5f);

			// PipeLineに登録
			spritePL->AddRenerObject(m_clearTimeSp.get());
		}

		if (m_animeCount > 22)
		{
			string totalDamageStr = m_pUnityChanPlayerComp->GetTotalDamageStr();
			const char* str = totalDamageStr.c_str();

			int count = 0;
			float x = 120.0f;	// 中心は 0, 0, w960, h540
			float y = 30.0f;

			// 座標、文字列、色とかを設定できる
			count = MakeSpriteString(m_totalDamageStrSp, count, x, y, 32, 40, str);

			// 使ったSpriteCharacterだけをパイプラインに登録
			for (int i = 0; i < count; i++)
			{
				// 文字列をパイプラインに登録
				spritePL->AddRenerObject(m_totalDamageStrSp[i].get());
			}

			// 被ダメージ総量画像の表示
			Texture2DContainer* tex = MyAccessHub::GetMyGameEngine()->GetTextureManager()->GetTexture(m_totalDamageSp->GetTextureId());
			float invH = 1.0f / tex->fHeight;

			XMFLOAT4 r = { 0.0f, 0.0f, 1.0f, 200.0f * invH };

			m_totalDamageSp->SetSpritePattern(0, tex->fWidth, 200.0f, r);
			m_totalDamageSp->SetSpriteIndex(0);

			m_totalDamageSp->SetPosition(-150.0f, 30.0f, 0.0f);
			m_totalDamageSp->SetScale(0.5f, 0.5f, 0.5f);

			// PipeLineに登録
			spritePL->AddRenerObject(m_totalDamageSp.get());
		}

		if (m_animeCount > 35)
		{
			const char* str = CalcOverallRank();

			int count = 0;
			float x = 120.0f;	// 中心は 0, 0, w960, h540
			float y = -100.0f;

			// 座標、文字列、色とかを設定できる
			count = MakeSpriteString(m_overallRankStrSp, count, x, y, 32, 40, str);

			// 使ったSpriteCharacterだけをパイプラインに登録
			for (int i = 0; i < count; i++)
			{
				// 文字列をパイプラインに登録
				spritePL->AddRenerObject(m_overallRankStrSp[i].get());
			}

			// 総合評価画像の表示
			Texture2DContainer* tex = MyAccessHub::GetMyGameEngine()->GetTextureManager()->GetTexture(m_overallRankSp->GetTextureId());
			float invH = 1.0f / tex->fHeight;

			XMFLOAT4 r = { 0.0f, 0.0f, 1.0f, 200.0f * invH };

			m_overallRankSp->SetSpritePattern(0, tex->fWidth, 200.0f, r);
			m_overallRankSp->SetSpriteIndex(0);

			m_overallRankSp->SetPosition(-150.0f, -100.0f, 0.0f);
			m_overallRankSp->SetScale(0.5f, 0.5f, 0.5f);

			// PipeLineに登録
			spritePL->AddRenerObject(m_overallRankSp.get());
		}

		if (m_animeCount > 50)
		{
			const char* str = "Press Space To Title";

			int count = 0;
			float x = -30.0f;	// 中心は 0, 0, w960, h540
			float y = -210.0f;

			// 座標、文字列、色とかを設定できる
			count = MakeSpriteString(m_pressSpaceToTitleStrSp, count, x, y, 20, 25, str);

			// 使ったSpriteCharacterだけをパイプラインに登録
			for (int i = 0; i < count; i++)
			{
				// 文字列をパイプラインに登録
				spritePL->AddRenerObject(m_pressSpaceToTitleStrSp[i].get());
			}
		}

		Texture2DContainer* tex = MyAccessHub::GetMyGameEngine()->GetTextureManager()->GetTexture(m_parchmentResultSp->GetTextureId());

		float invH = 1.0f / tex->fHeight;

		// U計算
		float u = (m_animeCount % 8) * 256.0f;
		// v計算
		float v = (m_animeCount / 8) * 256.0f;

		XMFLOAT4 r = { invH * u, invH * v, invH * 256.0f, invH * 256.0f };

		m_parchmentResultSp->SetSpritePattern(0, tex->fWidth * 1.0f, tex->fHeight * 1.0f, r);
		m_parchmentResultSp->SetSpriteIndex(0);

		// PipeLineに登録
		spritePL->AddRenerObject(m_parchmentResultSp.get());
	}

    return true;
}

// 終了時に呼ばれる処理
void ParchmentResultUI::FinishAction()
{
}
