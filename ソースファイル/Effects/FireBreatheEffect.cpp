#include "FireBreatheEffect.h"
#include "FBXCharacterData.h"

#include <MyAccessHub.h>
#include <D3D12Helper.h>

#include "WildHuntEnum.h"
#include "WildHuntScene.h"

// コンポーネント初期化時に呼ばれる処理
void FireBreatheEffect::InitAction()
{
	// m_breathEffSpの初期設定
	m_breathEffSp = std::make_unique<SpriteCharacter>();

	m_breathEffSp->SetCameraLabel(L"MainCamera");
	m_breathEffSp->SetGraphicsPipeLine(L"Sprite");
	m_breathEffSp->SetTextureId(L"FireBreathe");

	// ブレスの初期のUV設定(値は適当)
	Texture2DContainer* tex = MyAccessHub::GetMyGameEngine()->GetTextureManager()->GetTexture(m_breathEffSp->GetTextureId());
	float invH = 1.0f / tex->fHeight;
	XMFLOAT4 r = { 0.0f, 0.0f, invH * 32.0f, invH * 32.0f };

	m_breathEffSp->SetSpritePattern(0, tex->fWidth * 1.0f, tex->fHeight * 1.0f, r);
	m_breathEffSp->SetSpriteIndex(0);

	// Scaleは小さめ
	m_breathEffSp->SetScale(0.001f, 0.001f, 0.001f);

	m_breathEffSp->SetColorMix(SpriteCharacter::COLOR_MIX_OP::MIX_MUL);
	m_breathEffSp->SetColor(1, 1, 1, 1);

	// 当たり判定の初期処理
	m_fireBreatheHit.SetAttackType((UINT)HIT_ORDER::HIT_ENEMY_ATTACK, 5);

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
bool FireBreatheEffect::FrameAction()
{
	MyGameEngine* engine = MyAccessHub::GetMyGameEngine();
	GraphicsPipeLineObjectBase* spritePL = engine->GetPipelineManager()->GetPipeLineObject(L"Sprite");

	// HeadとNeckで、ドラゴンが向いてる方向を取得しよう
	XMMATRIX headMat = CreateWorldMatrix(m_dragonHeadData);
	XMMATRIX neckMat = CreateWorldMatrix(m_dragonNeckData);

	// アフィン変換行列作成
	XMMATRIX AffineMat;
	XMMATRIX translate = XMMatrixTranslation(m_nowDragonPos.x, m_nowDragonPos.y, m_nowDragonPos.z);
	XMMATRIX rotate_x = XMMatrixRotationX(XMConvertToRadians(m_nowDragonRotate.x));
	XMMATRIX rotate_y = XMMatrixRotationY(XMConvertToRadians(m_nowDragonRotate.y));
	XMMATRIX rotate_z = XMMatrixRotationZ(XMConvertToRadians(m_nowDragonRotate.z));
	XMMATRIX scale_mat = XMMatrixScaling(m_nowDragonScale.x, m_nowDragonScale.y, m_nowDragonScale.z);

	AffineMat = scale_mat * rotate_z * rotate_x * rotate_y * translate;

	// HeadとNeckのマトリクス作成、
	headMat = headMat * AffineMat;
	neckMat = neckMat * AffineMat;

	// 方向ベクトルを正規化して、微調整するため。
	XMVECTOR breathVec = XMVectorSubtract(neckMat.r[3], headMat.r[3]);
	breathVec = XMVector3Normalize(breathVec);

	// ブレスを口の位置に調整する
	m_breathEffSp->SetPosition(headMat.r[3].m128_f32[0] + (breathVec.m128_f32[0] * -14.0f), headMat.r[3].m128_f32[1] + 0.2f, headMat.r[3].m128_f32[2] + (breathVec.m128_f32[2] * -14.0f));

	// 絶対にカメラに見せるための処理
	// カメラへのベクトルと、NeckからHeadベクトルの法線ベクトルとの内積で考える
	CharacterData* camData = m_pCameraComp->GetGameObject()->GetCharacterData();
	XMFLOAT3 camPos = camData->GetPosition();

	XMVECTOR camVec = XMVectorSubtract(XMLoadFloat3(&camPos), headMat.r[3]);
	camVec = XMVector3Normalize(camVec);

	// breathVecの法線ベクトルを取得する
	XMVECTOR arbitraryVec = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	XMVECTOR normalVec = XMVector3Cross(breathVec, arbitraryVec);

	float dot = XMVectorGetX(XMVector3Dot(normalVec, camVec));
	float breathRad = atan2f(neckMat.r[3].m128_f32[0] - headMat.r[3].m128_f32[0], neckMat.r[3].m128_f32[2] - headMat.r[3].m128_f32[2]);

	// 三項分岐で常に、画像を自分の見ている方向に
	m_breathEffSp->SetRotation((dot >= 0) ? 0.0f : 180.0f, XMConvertToDegrees(breathRad) + 90.0f, 0.0f);

	if (m_isEffFlag)
	{
		m_animeCount++;

		if(m_animeCount == 1)
			engine->GetSoundManager()->Play(8);

		if (m_animeCount >= 375)
		{
			m_isEffFlag = false;
		}

		// このブレスのアニメパターンは375で作成した
		// 休憩時間は作りません(炎がユラユラする)。
		m_animeCount %= 375;
		
		Texture2DContainer* tex = MyAccessHub::GetMyGameEngine()->GetTextureManager()->GetTexture(m_breathEffSp->GetTextureId());
		float invH = 1.0f / tex->fHeight;

		// U計算(一つのエフェクトの横幅は960.0f)
		float u = (m_animeCount % 15) * 960.0f;

		// v計算(一つのエフェクトの縦幅は539.5f)
		float v = (m_animeCount / 15) * 539.5f;

		XMFLOAT4 r = { invH * u, invH * v, invH * 960.0f, invH * 539.5f };

		// ちょっと横長で表示したい
		m_breathEffSp->SetSpritePattern(0, tex->fWidth * 1.56f, tex->fHeight * 1.0f, r);
		m_breathEffSp->SetSpriteIndex(0);

		// PipeLineに登録
		spritePL->AddRenerObject(m_breathEffSp.get());

		// 当たり判定を設定
		XMFLOAT3 startPos;
		XMStoreFloat3(&startPos, headMat.r[3]);

		XMFLOAT3 extendPos = { headMat.r[3].m128_f32[0] + (breathVec.m128_f32[0] * -30.0f), headMat.r[3].m128_f32[1], headMat.r[3].m128_f32[2] + (breathVec.m128_f32[2] * -30.0f) };

		m_fireBreatheHit.SetLine(startPos, extendPos, 5.0f);
		MyAccessHub::GetMyGameEngine()->GetHitManager()->SetHitArea(this, &m_fireBreatheHit);

	}

	return true;
}

// 終了時に呼ばれる処理
void FireBreatheEffect::FinishAction()
{
}

// ワールドマトリクス作成用メソッド、HeadとNeckで複数回使うからメソッド化
XMMATRIX FireBreatheEffect::CreateWorldMatrix(XMMATRIX matData)
{
	FBXCharacterData* efData =
		static_cast<FBXCharacterData*>(GetGameObject()->GetCharacterData());

	XMFLOAT3 nowPos = m_breathEffSp->GetPosition();
	XMFLOAT3 nowRot = m_breathEffSp->GetRotation();
	XMFLOAT3 nowScale = m_breathEffSp->GetScale();
	XMMATRIX breathRot_x = XMMatrixRotationX(XMConvertToRadians(nowRot.x));
	XMMATRIX breathRot_y = XMMatrixRotationY(XMConvertToRadians(nowRot.y));
	XMMATRIX breathRot_z = XMMatrixRotationZ(XMConvertToRadians(nowRot.z));
	XMMATRIX breathPos_mat = XMMatrixTranslation(nowPos.x, nowPos.y, nowPos.z);
	XMMATRIX breathScale_mat = XMMatrixScaling(nowScale.x, nowScale.y, nowScale.z);
	XMMATRIX breathMat = breathScale_mat * breathRot_x * breathRot_z * breathRot_y * breathPos_mat * matData;

	return breathMat;
}

// ヒット時の処理
void FireBreatheEffect::HitReaction(GameObject* targetGo, HitAreaBase* hit)
{
}
