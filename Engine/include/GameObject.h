#pragma once
#include <DirectXMath.h>
#include <list>
#include <queue>
#include "HitManager.h"
#include "HitShapes.h"
#include "CharacterData.h"
#include "PipeLineManager.h"

using namespace std;

class CharacterData;
class GameObject;							// GameComponentクラスで宣言に使いたいので前方宣言
class HitQuad;

class AbstractGameObjectDeleter
{
public:
	virtual void ExecuteDeleter(GameObject* go) = 0;
};

class GameComponent								// インタフェースっぽいけど抽象クラス（メンバ変数があるから）
{
private:
	bool m_activeFlg;
	GameObject* m_gObject;

	virtual void InitAction() = 0;		// コンポーネント初期化時に呼ばれる処理

public:
	void InitFromGameObject(GameObject* objData)
	{
		m_gObject = objData;
		m_activeFlg = true;
		InitAction();
	}

	bool IsActive()
	{
		return m_activeFlg;
	}

	void SetActive(bool flg)
	{
		m_activeFlg = flg;
	}

	// 純粋仮想関数（メソッド）。このクラスでは実装出来ない　継承した別クラスで実装する必要がある
	virtual bool FrameAction() = 0;		// 毎フレーム呼ばれる処理　falseを返すとこのコンポーネントは終了し削除される
	virtual void FinishAction() = 0;	// 終了時に呼ばれる処理

	// ヒット時リアクション処理
	virtual void HitReaction(GameObject* targetGo, HitAreaBase * hit) {};

	GameObject* GetGameObject() { return m_gObject; }
};

class GameObject								// サンプルエンジン用ゲームオブジェクト基底クラス
{
protected:
	bool m_enableFlg;

	unique_ptr<CharacterData> characterData;	// キャラクタの座標とUVなどが入っている
	list<GameComponent*> components;			// このゲームオブジェクトに乗せられたコンポーネントのリスト
	queue<GameComponent*> addComponents;		// 追加待ちコンポーネント

	// Deleterオブジェクトのメンバ変数を追加
	AbstractGameObjectDeleter* m_pDeleter = nullptr;

public:
	GameObject(CharacterData* cData)
	{
		m_enableFlg = true;
		characterData.reset(cData);
	}

	CharacterData* GetCharacterData()			// CharacterDataのインスタンスを取得して座標などが直接変えられる
	{
		return characterData.get();
	}

	void AddComponentToQueue(GameComponent* com)
	{
		addComponents.push(com);
	}

	void AddComponent(GameComponent* com)				// GameObjectにコンポーネントを足す（処理をふやす）
	{
		com->InitFromGameObject(this);					// コンポーネントごとの初期化処理
		components.push_back(com);
	}

	list<GameComponent*>& GetComponents()
	{
		return components;
	}

	void RemoveComponent(GameComponent* com)	// GameObjectからコンポーネントを消す（処理を減らす）
	{
		com->FinishAction();					// コンポーネントごとの終了処理
		components.remove(com);
		delete(com);							// コンポーネントのメモリ解放
	}

	bool IsEnable()
	{
		return m_enableFlg;
	}

	void SetEnable(bool flg)
	{
		m_enableFlg = flg;
	}

	// Deleterオブジェクトの設定メソッド
	void SetGameObjectDeleter(AbstractGameObjectDeleter* deleter) {
		m_pDeleter = deleter;
	}

	virtual void CleanupGameObject();			// GameObjectの削除処理コンポーネントの全削除
	virtual bool Action();						// 毎フレーム呼ばれるメソッド falseを返すとオブジェクトの消去
	virtual void Init();						// GameEngineにADDされた時に呼ばれるメソッド
};