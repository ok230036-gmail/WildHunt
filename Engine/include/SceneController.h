#pragma once

#include <Windows.h>
#include "GameObject.h"

class GameObject;

class SceneController
{
protected:
	UINT m_scene = 0;
	UINT m_orderSceneID;
	HRESULT virtual ChangeGameScene(UINT scene) = 0;

public:
	HRESULT virtual InitSceneController() = 0;

	void OrderNextScene(UINT scene)
	{
		m_orderSceneID = scene;
	}

	void CheckSceneOrder();

	// 仮想デストラクタ
	virtual ~SceneController() {};

	virtual void AddSceneObject(GameObject* obj) = 0;
	virtual void DeleteSceneObject(GameObject* obj) = 0;
};