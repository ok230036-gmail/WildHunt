#pragma once

#include <GameObject.h>

using namespace DirectX;

class CameraComponent : public GameComponent
{
private:
	XMFLOAT3	m_normal;
	XMFLOAT3	m_focus;
	XMFLOAT3	m_direction;

	float		m_near;
	float		m_far;
	float		m_fov;

	float		m_width;
	float		m_height;

	bool updateFlg;

	void InitAction() override;			// コンポーネント初期化時に呼ばれる処理

public:
	bool FrameAction() override;		// 毎フレーム呼ばれる処理　falseを返すとこのコンポーネントは終了し削除される
	void FinishAction() override;		// 終了時に呼ばれる処理

	void ChangeCameraRatio(float width, float height);
	void ChangeCameraPosition(float x, float y, float z);
	void ChangeCameraRotation(float x, float y, float z);
	void ChangeCameraFocus(float x, float y, float z);
	void ChangeCameraDepth(float nearZ, float farZ);
	void ChangeCameraFOVRadian(float fovRad);

	XMFLOAT3 GetCameraNormal()
	{
		return m_normal;
	}

	XMFLOAT3 GetCameraDirection()
	{
		return m_direction;
	}

	XMFLOAT3 GetCameraFocus()
	{
		return m_focus;
	}

	float GetViewRatio()
	{
		return m_width / m_height;
	}

	float GetSetWidth()
	{
		return m_width;
	}

	float GetSetHeight()
	{
		return m_height;
	}

	float GetCameraFOVRad()
	{
		return m_fov;
	}

	XMFLOAT2 GetCameraRange()
	{
		return XMFLOAT2(m_near, m_far);
	}

};