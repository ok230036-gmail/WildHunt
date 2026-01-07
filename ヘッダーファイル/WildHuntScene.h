#pragma once

#include <memory>
#include <GameObject.h>
#include <SceneController.h>

#include <unordered_map>

#include "TerrainComponent.h"

class WildHuntScene;	// 前方宣言

class SceneObjectDeleter : public AbstractGameObjectDeleter
{
private:
	WildHuntScene* myScene;
public:
	SceneObjectDeleter(WildHuntScene* scene)
	{
		myScene = scene;
	}
	//  AbstractGameObjectDeleter を介して継承されました
	virtual void ExecuteDeleter(GameObject* go) override;
};

class WildHuntScene : public SceneController
{
private:
	std::unique_ptr<GameObject> m_systemObject;

	GameComponent* m_keyComponent;
	std::unordered_map<std::wstring, GameComponent*> m_cameraComponents;

	// 地形判定Component
	std::vector<TerrainComponent*> m_terrains;

	std::list<std::unique_ptr<GameObject>> m_SceneObjects;
	std::unique_ptr<SceneObjectDeleter> m_pObjDeleter;
	void ClearSceneObjects();

public:
	~WildHuntScene();


	HRESULT InitSceneController() override;
	HRESULT ChangeGameScene(UINT scene) override;

	GameComponent* GetKeyComponent()
	{
		return m_keyComponent;
	}

	GameComponent* GetCameraComponent(const std::wstring cameraName)
	{
		if (m_cameraComponents[cameraName] != nullptr)
		{
			return m_cameraComponents[cameraName];
		}

		return nullptr;
	}

	void RemoveCamera(GameComponent* gc);
	void RemoveCamera(std::wstring label);

	void AddSceneObject(GameObject* obj) override;
	void DeleteSceneObject(GameObject* obj) override;

	// TerrainComponent取得
	TerrainComponent* GetTerrainComponent(int index)
	{
		if (m_terrains.size() <= index)
			return nullptr;

		return m_terrains[index];
	}

	// 地形データクリア
	void ClearTerrains()
	{
		m_terrains.clear();
	}
};
