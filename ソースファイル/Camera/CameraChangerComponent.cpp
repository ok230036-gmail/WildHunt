#include "WildHuntScene.h"
#include "CameraChangerComponent.h"

void CameraChangerComponent::InitAction()
{
	WildHuntScene* scene = dynamic_cast<WildHuntScene*>(MyAccessHub::GetMyGameEngine()->GetSceneController());
	m_keyBind = dynamic_cast<KeyBindComponent*>(scene->GetKeyComponent());
	m_currentCamera = -1;
}

bool CameraChangerComponent::FrameAction()
{
	// カメラチェンジテスト
	if (m_keyBind->GetCurrentInputState(InputManager::BUTTON_STATE::BUTTON_DOWN, KeyBindComponent::BUTTON_IDS::MOUSE_R))
	{
		int index = (m_currentCamera + 1) % m_cameraComponents.size();

		ChangeCameraController(index);
	}

	return true;
}

void CameraChangerComponent::FinishAction()
{
	m_cameraComponents.clear();
}

void CameraChangerComponent::SetCameraController(GameComponent* camCon)
{
	m_cameraComponents.push_back(camCon);
	camCon->SetActive(false);
}

void CameraChangerComponent::ChangeCameraController(int index)
{
	if (index >= m_cameraComponents.size()) return;

	GameComponent* cam = m_cameraComponents[index];
	if (cam != nullptr)
	{
		// 新しいCameraをONにして現在のCameraをOFF
		if (m_currentCamera > -1)
			m_cameraComponents[m_currentCamera]->SetActive(false);

		m_currentCamera = index;
		cam->SetActive(true);
	}
}
