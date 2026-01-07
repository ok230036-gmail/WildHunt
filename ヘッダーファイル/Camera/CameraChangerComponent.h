#pragma once
#include <vector>
#include "GameObject.h"
#include "KeyBindComponent.h"
#include "CameraComponent.h"

class CameraChangerComponent :
    public GameComponent
{
private:
	KeyBindComponent* m_keyBind;
	std::vector<GameComponent*> m_cameraComponents;

	int m_currentCamera = -1;

	void InitAction() override;			// コンポーネント初期化時に呼ばれる処理

public:
	bool FrameAction() override;		// 毎フレーム呼ばれる処理　falseを返すとこのコンポーネントは終了し削除される
	void FinishAction() override;		// 終了時に呼ばれる処理

	void SetCameraController(GameComponent* camCon);
	void ChangeCameraController(int index);
};

