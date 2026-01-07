#include "GameObject.h"
#include <MyAccessHub.h>
// #include <algorithm>

void GameObject::Init()
{
	// 初期状態は何もなし。
}

bool GameObject::Action()	// フレーム処理
{
	list<GameComponent*> deleteComponents;	// 削除リスト

	if (!IsEnable()) return true;			// Enableでない場合はtrueを返して終了

	// 追加待ちコンポーネント追加処理
	if (!addComponents.empty())
	{
		GameComponent* comp = nullptr;
		while ( (comp = addComponents.front()) != nullptr)
		{
			AddComponent(comp);
			addComponents.pop();
		}
	}

	for (auto comp : components)
	{
		if (comp->IsActive() && !comp->FrameAction())	// 中身のframeActionを実行
		{
			deleteComponents.push_back(comp);
		}
	}

	// 終了コンポーネント削除モード
	if (!deleteComponents.empty())
	{
		for (auto deleteComp : deleteComponents)			// deleteComponentsの最初（begin）から最後（end）まで
		{
			RemoveComponent(deleteComp);					// この中でfinishActionを呼んでいるので、問題なく削除可能。
		}

		deleteComponents.clear();		// 削除リスト消去

		if (components.empty())			// コンポーネントが空か
		{
			return false;				// オブジェクトを削除
		}
	}

	return true;						// 次フレームもオブジェクト処理継続
}

void GameObject::CleanupGameObject()	// 完全終了処理
{
	for (auto comp : components)
	{
		comp->FinishAction();			// オブジェクトのfinishActionを実行
		delete(comp);
	}

	components.clear();					// componentsの中身を全て削除

	// Deleter実行
	if (m_pDeleter != nullptr)
	{
		m_pDeleter->ExecuteDeleter(this);
	}
}
