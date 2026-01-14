#pragma once

enum class GAME_SCENES	// 使用するゲームのシーン
{
	AWAKE,
	INIT,
	TITLE,
	IN_GAME,
	GAME_OVER,
	GAME_CLEAR,
};

enum class HIT_ORDER	// 判定タイプの enum class
{
	HIT_PLAYER_BODY,
	HIT_PLAYER_ATTACK,
	HIT_PLAYER_SHIELD,
	HIT_ENEMY_BODY,
	HIT_ENEMY_ATTACK,
	HIT_ENEMY_SHIELD,
	HIT_ITEM,
};

enum class SceneState   // TransitionをアリSceneの状態
{
	SceneImage,
	Transition,
	Loading,
};
