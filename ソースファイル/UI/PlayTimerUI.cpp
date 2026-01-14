#include "PlayTimerUI.h"

#include <MyAccessHub.h>
#include <D3D12Helper.h>

#include "WildHuntEnum.h"
#include "WildHuntScene.h"

// 座標、文字列、色とかを設定
int PlayTimerUI::MakeSpriteString(int startIndex, float ltX, float ltY, float width, float height, const char* str)
{
	int count = startIndex;

	while (*str != '\0')
	{
		if (std::find(m_chList, m_chEnd, *str) != m_chEnd)
		{
			m_sprites[count]->SetSpritePattern(0, width, height, m_fontMap[*str]);
			m_sprites[count]->SetSpriteIndex(0);

			m_sprites[count]->SetPosition(ltX, ltY, 1.0f);	// 他の画像を上書きしないため、ちょっと奥に表示
			count++;
		}

		ltX += width;

		str++;
	}

	return count;
}

// コンポーネント初期化時に呼ばれる処理
void PlayTimerUI::InitAction()
{
	MyGameEngine* engine = MyAccessHub::GetMyGameEngine();
	CharacterData* chData = GetGameObject()->GetCharacterData();

	engine->InitCameraConstantBuffer(chData);

	chData->SetPosition(0.0f, 0.0f, 1.0f);	// 他の画像を上書きしないため、ちょっと奥に表示

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

		spc->SetColor(1,1,1,1);

		spc->SetGraphicsPipeLine(L"Sprite");

		m_sprites.push_back(std::unique_ptr<SpriteCharacter>(spc));
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
		XMFLOAT4 r = {x * invW, y * invH, w * invW, h * invH};
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
bool PlayTimerUI::FrameAction()
{
	MyGameEngine* engine = MyAccessHub::GetMyGameEngine();
	GraphicsPipeLineObjectBase* pipeLine = engine->GetPipelineManager()->GetPipeLineObject(L"Sprite");

	if (!m_isStopTime)	// クリアしたらタイマーは停止
		m_timer++;

	// 右上にタイマーを表示
	int minutes = m_timer / 3600;				// 分
	int seconds = (m_timer % 3600) / 60;		// 秒
	string scorestr = std::to_string(minutes) + ":" + (seconds < 10 ? "0" : "") + std::to_string(seconds);

	const char* str = scorestr.c_str();

	int count = 0;
	float x = 220.0f + 100.0f;	// 中心は 0, 0, w960, h540
	float y = 270.0f - 75.0f;

	// 座標、文字列、色とかを設定できる
	count = MakeSpriteString(count, x, y, 32, 40, str);

	// 使ったSpriteCharacterだけをパイプラインに登録
	for (int i = 0; i < count; i++)
	{
		pipeLine->AddRenerObject(m_sprites[i].get());
	}

	return true;
}

// 終了時に呼ばれる処理
void PlayTimerUI::FinishAction()
{
	m_sprites.clear();

	WildHuntScene* scene = static_cast<WildHuntScene*>(MyAccessHub::GetMyGameEngine()->GetSceneController());
	scene->RemoveCamera(this);

}
