#include <MyAccessHub.h>
#include <MyGameEngine.h>
#include "CameraComponent.h"

#include "WildHuntScene.h"

#include "LightSettingManager.h"	// 

void CameraComponent::InitAction()
{
	m_normal.x = 0.0f;
	m_normal.y = 1.0f;
	m_normal.z = 0.0f;

	m_focus.x = 0.0f;
	m_focus.y = 0.0f;
	m_focus.z = 10.0f;

	MyGameEngine* engine = MyAccessHub::GetMyGameEngine();
	CharacterData* chData = GetGameObject()->GetCharacterData();

	// カメラ用定数バッファをCharacterDataに登録
	engine->InitCameraConstantBuffer(chData);

	XMFLOAT3 fl3 = {};
	chData->AddConstantBuffer(sizeof(XMVECTOR), &fl3);

	updateFlg = true;
}

bool CameraComponent::FrameAction()
{
	if (updateFlg)
	{
		updateFlg = false;

		// カメラマトリクスを更新
		MyGameEngine* engine = MyAccessHub::GetMyGameEngine();
		CharacterData* chData = GetGameObject()->GetCharacterData();

		XMFLOAT3 pos = chData->GetPosition();

		XMVECTOR Eye = XMVectorSet(pos.x, pos.y, pos.z, 0.0f);					// 視点（カメラ）座標
		XMVECTOR At = XMVectorSet(m_focus.x, m_focus.y, m_focus.z, 0.0f);		// フォーカスする（カメラが向く）座標
		XMVECTOR Up = XMVectorSet(m_normal.x, m_normal.y, m_normal.z, 0.0f);	// カメラの上方向単位ベクトル（カメラのロール軸）

		// MyGameEngineが持つ共通処理、カメラ用定数バッファを現在のパラメータで更新
		engine->UpdateCameraMatrixForComponent(m_fov, Eye, At, Up, m_width, m_height, m_near, m_far);

		// カメラの向き
		XMVECTOR camdir = XMVector3Normalize(XMVectorSet(m_focus.x - pos.x, m_focus.y - pos.y, m_focus.z - pos.z, 0.0f) );
		m_direction.x = XMVectorGetX(camdir);
		m_direction.y = XMVectorGetY(camdir);
		m_direction.z = XMVectorGetZ(camdir);

		engine->UpdateShaderResourceOnGPU(chData->GetConstantBuffer(2), &Eye, sizeof(XMVECTOR));

		// シャドウマップ実装
		WildHuntScene* scene = static_cast<WildHuntScene*>(engine->GetSceneController());
		auto dLight = LightSettingManager::GetInstance()->GetDirectionalLight(L"SCENE_DIRECTIONAL");
		dLight->UpdateLightBaseMatrix(pos, m_focus);
	}
	return true;
}

void CameraComponent::FinishAction()
{
	WildHuntScene* scene = static_cast<WildHuntScene*>(MyAccessHub::GetMyGameEngine()->GetSceneController());
	scene->RemoveCamera(this);
}

// カメラの情報が変わったら、updateFlg = true に
void CameraComponent::ChangeCameraRatio(float width, float height)
{
	m_height = height;
	m_width = width;

	updateFlg = true;
}

void CameraComponent::ChangeCameraPosition(float x, float y, float z)
{	
	CharacterData* chData = GetGameObject()->GetCharacterData();
	chData->SetPosition(x, y, z);

	updateFlg = true;
}

void CameraComponent::ChangeCameraRotation(float x, float y, float z)
{
	CharacterData* chData = GetGameObject()->GetCharacterData();
	chData->SetRotation(x, y, z);

	updateFlg = true;
}

void CameraComponent::ChangeCameraFocus(float x, float y, float z)
{
	m_focus.x = x;
	m_focus.y = y;
	m_focus.z = z;

	updateFlg = true;
}

void CameraComponent::ChangeCameraDepth(float nearZ, float farZ)
{
	m_near = nearZ;
	m_far = farZ;
	updateFlg = true;
}

void CameraComponent::ChangeCameraFOVRadian(float fovRad)
{
	m_fov = fovRad;
	updateFlg = true;
}
