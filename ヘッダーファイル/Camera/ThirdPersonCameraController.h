#pragma once

#include <GameObject.h>
#include "KeyBindComponent.h"
#include "CameraComponent.h"

class ThirdPersonCameraController : public GameComponent
{
private:
	KeyBindComponent* m_keyBind;
	CameraComponent* m_camera;

	float			m_hDeg;
	float			m_vDeg;
	float			m_distance;

	bool			m_cameraShakeFlag;
	float			m_cameraShakeCnt;
	float			m_cameraShakeMove;

	void InitAction() override;		// コンポーネント初期化時に呼ばれる処理

public:
	bool FrameAction() override;		// 毎フレーム呼ばれる処理　falseを返すとこのコンポーネントは終了し削除される
	void FinishAction() override;		// 終了時に呼ばれる処理

	void ChangeCameraDistance(float distance)	// カメラの距離を変える。
	{
		if (m_distance != distance)
		{
			if (m_distance < distance)
			{
				// カメラが遠のく
				m_distance += (distance - m_distance) * 0.10f;
			}
			else
			{
				// カメラが近ずく
				m_distance -= (m_distance - distance) * 0.05f;
			}

		}
	}

	// カメラを揺らす
	void CameraShakeFlag()
	{
		m_cameraShakeFlag = true;
		m_cameraShakeCnt = 0;
	}
};