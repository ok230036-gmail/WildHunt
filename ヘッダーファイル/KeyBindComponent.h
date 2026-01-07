#pragma once
#include <Windows.h>
#include <DirectXMath.h>
#include <memory>
#include <vector>
// #include <list>
#include <unordered_map>

#include <MyGameEngine.h>
#include <GameObject.h>
#include <InputManager.h>

using namespace DirectX;
using namespace std;


class KeyBindComponent : public GameComponent
{

public:
	enum class INPUT_TYPES
	{
		KEYBOARD,
		XINPUT_0,
		XINPUT_1,
		XINPUT_2,
		XINPUT_3,
		DINPUT_0,
		DINPUT_1,
		DINPUT_2,
		DINPUT_3,
	};

	enum class ANALOG_IDS {
		MOVE_H,
		MOVE_V,

		CAMERA_H,
		CAMERA_V,
		WHEEL,
		WHEEL_H
	};

	enum class BUTTON_IDS {
		MOVE_LEFT,
		MOVE_RIGHT,
		MOVE_FORWARD,
		MOVE_BACK,
		ALT_MOVE_LEFT,
		ALT_MOVE_RIGHT,
		ALT_MOVE_FORWARD,
		ALT_MOVE_BACK,
		BTN_OK,
		BTN_CANCEL,

		BTN_JUMP,
		BTN_ATTACK,
		BTN_EVADE,

		MOUSE_R,
		MOUSE_L,
		MOUSE_C,
	};

	struct InputConfigData
	{
	public:
		unordered_map<ANALOG_IDS, UINT32> m_analogMap;
		unordered_map<BUTTON_IDS, UINT32> m_buttonMap;

		InputConfigData();
		~InputConfigData();
	};

	void InitAction() override;			// コンポーネント初期化時に呼ばれる処理
	bool FrameAction() override;		// 毎フレーム呼ばれる処理　falseを返すとこのコンポーネントは終了し削除される
	void FinishAction() override;		// 終了時に呼ばれる処理

	bool GetCurrentInputState(InputManager::BUTTON_STATE state, BUTTON_IDS buttonId);
	LONG GetCurrentAnalogValue(ANALOG_IDS analogId);
	bool GetInputState(INPUT_TYPES inputDev, InputManager::BUTTON_STATE state, BUTTON_IDS buttonId);
	LONG GetAnalogValue(INPUT_TYPES inputDev, ANALOG_IDS analogId);

private:
	unordered_map<INPUT_TYPES, unique_ptr<InputConfigData>> m_configData;
	INPUT_TYPES m_currentInputType;

	XMINT2		m_mousePos;
	XMINT2		m_mousePrePos;

	InputManager* m_inputMng;

public:
	INPUT_TYPES GetCurrentInputType()
	{
		return m_currentInputType;
	}

};