#include "InputManager.h"
#include <memory>

HWND					g_hWnd;

#if DIRECT_INPUT_ACTIVE

LPDIRECTINPUT8			g_pDInput = nullptr;			//  IDirectInput8インターフェースへのポインタ
LPDIRECTINPUTDEVICE8	g_pDIGamePad[MAX_DI_PADS];		//  IDirectInputDevice8インターフェースへのポインタ（ゲームパッド）

#if DI_KEY_MOUSE
LPDIRECTINPUTDEVICE8	g_pDIKeyboard;					//  IDirectInputDevice8インターフェースへのポインタ（キーボード）
LPDIRECTINPUTDEVICE8	g_pDIMouse;						//  IDirectInputDevice8インターフェースへのポインタ（マウス）
#endif

UINT					g_DICount = 0;					//  DirectInput用コントローラの接続数

// EnumDeviceがDirectInputデバイスを見つけたら呼ばれる。
BOOL CALLBACK EnumDIJoysticksCallback(const DIDEVICEINSTANCE* pdidInstance, VOID* pContext)
{
	HRESULT hr;

	// デバイス作成
	hr = g_pDInput->CreateDevice(pdidInstance->guidInstance, &g_pDIGamePad[g_DICount], NULL);
	if (FAILED(hr)) return DIENUM_CONTINUE;	// 失敗したらここで終了

	// デバイスタイプ設定
	hr = g_pDIGamePad[g_DICount]->SetDataFormat(&c_dfDIJoystick2);	// このデバイスはジョイスティック（ゲームパッド:アナログ有）
																	// c_dfDIJoystick : ジョイスティック（アナログなし）
																	// c_dfDIKeyboard : キーボード / c_dfDIMouse : マウス
	if (FAILED(hr))
	{
		goto FAILED_POINT;
	}

	// アナログ軸モード設定	基本的にパッドなら絶対値モード。
	DIPROPDWORD diProp;								// パラメータ設定構造体
	ZeroMemory(&diProp, sizeof(DIPROPDWORD));
	diProp.diph.dwSize = sizeof(DIPROPDWORD);
	diProp.diph.dwHeaderSize = sizeof(diProp.diph);
	diProp.diph.dwHow = DIPH_DEVICE;
	diProp.diph.dwObj = 0;
	diProp.dwData = DIPROPAXISMODE_ABS;				// 軸モード絶対値(DIPROPAXISMODE_RELにしたら相対値)
	hr = g_pDIGamePad[g_DICount]->SetProperty(DIPROP_AXISMODE, &diProp.diph);

	if (FAILED(hr))
	{
		goto FAILED_POINT;
	}

	// アナログ軸数値範囲設定
	DIPROPRANGE diPrg;
	ZeroMemory(&diPrg, sizeof(DIPROPRANGE));
	diPrg.diph.dwSize = sizeof(DIPROPRANGE);
	diPrg.diph.dwHeaderSize = sizeof(diPrg.diph);
	diPrg.diph.dwHow = DIPH_BYOFFSET;
	diPrg.diph.dwObj = DIJOFS_X;
	diPrg.lMin = -32767;				// xInput基準
	diPrg.lMax = 32767;

	// X軸
	hr = g_pDIGamePad[g_DICount]->SetProperty(DIPROP_RANGE, &diPrg.diph);
	if (FAILED(hr))
	{
		goto FAILED_POINT;
	}
	// Y軸
	diPrg.diph.dwObj = DIJOFS_Y;
	hr = g_pDIGamePad[g_DICount]->SetProperty(DIPROP_RANGE, &diPrg.diph);
	if (FAILED(hr))
	{
		goto FAILED_POINT;
	}

	hr = g_pDIGamePad[g_DICount]->SetCooperativeLevel(g_hWnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);
	if (FAILED(hr))
	{
		goto FAILED_POINT;
	}

	// パッド入力開始
	hr = g_pDIGamePad[g_DICount]->Acquire();
	if (FAILED(hr))
	{
		goto FAILED_POINT;
	}

	goto SUCCESS_POINT;		// 初期化完了

FAILED_POINT:

	g_pDIGamePad[g_DICount]->Release();
	g_pDIGamePad[g_DICount] = nullptr;
	return DIENUM_CONTINUE;

SUCCESS_POINT:

	g_DICount++;
	if (g_DICount < MAX_DI_PADS)
		return DIENUM_CONTINUE;

	return DIENUM_STOP;	// DIENUM_STOPが来るとこれ以上の列挙を停止する。
}
#endif

InputManager::InputManager()
{
	int i, j;

	// 00:押されていない 001:押されている 011:押された瞬間 010:離された瞬間
	for (i = 0; i < MAX_KEYS; i++)
	{
		m_keyState[i] = 0;
	}
	
	m_wheelDelta = 0;

#if DIRECT_INPUT_ACTIVE & DI_KEY_MOUSE
	for (i = 0; i < MAX_MOUSE_BUTTONS; i++)
	{
		m_mouseState[i] = 0;
	}
#endif

	for (i = 0; i < MAX_PADS; i++)
	{
		m_xPadActive[i] = false;
		
		for (j = 0; j < MAX_PAD_BUTTONS; j++)
		{
			m_buttonState[i][j] = 0;
		}

		for (j = 0; j < MAX_PAD_ANALOGIN; j++)
		{
			m_analogState[i][j] = 0;
		}
	}

#if DIRECT_INPUT_ACTIVE
	for (i = 0; i < MAX_DI_PADS; i++)
	{
		// POV
		m_diDirection[i][0] = 0;
		m_diDirection[i][1] = 0;
		m_diDirection[i][2] = 0;
		m_diDirection[i][3] = 0;

		// BUTTON
		for (j = 0; j < MAX_DI_BUTTONS; j++)
		{
			m_diButtonState[i][j] = 0;
		}

		// ANALOG
		for (j = 0; j < MAX_DI_ANALOGIN; j++)
		{
			m_diAnalogState[i][j] = 0;
		}
	}

#endif

}

InputManager* InputManager::GetInstance()
{
	static std::unique_ptr<InputManager> instance = nullptr;

	if (instance == nullptr)
	{
		instance.reset(new InputManager());
	}

	return instance.get();
}

InputManager::~InputManager()
{
	SetInputEnable(false);

#if DIRECT_INPUT_ACTIVE
	ReleaseDirectInput();
#endif
}

HRESULT InputManager::InitInputManager(HINSTANCE hInst, HWND hwnd)
{
	g_hWnd = hwnd;		// DirectInputがグローバル関数を使うのでHWNDはメンバに出来ない。
	RefreshBuffer();	// 入力バッファ初期化

#if DIRECT_INPUT_ACTIVE
	return InitDirectInput(hInst);
#else
	return S_OK;
#endif
}

void InputManager::Update()
{
	int i;	// ループカウンタ

#if !DI_KEY_MOUSE

	// キーボード＋マウスボタン
	BYTE currentKeys[256];

	if (GetKeyboardState(currentKeys))
	{
		// GetKeyStateで個別に取る。
		for (i = 0; i < MAX_KEYS; i++)
		{
			m_keyState[i] <<= 1;	// 左１ビットシフト

			if (currentKeys[i] & 0x80)
			{
				m_keyState[i] |= 0x01;
			}
		}

	}

	// マウス座標
	POINT mousepos;
	GetCursorPos(&mousepos);
	ScreenToClient(g_hWnd, &mousepos);

	// カーソル移動量
	m_mouseVect.x = mousepos.x - m_mousePos.x;
	m_mouseVect.y = mousepos.y - m_mousePos.y;

	m_mousePos.x = mousepos.x;
	m_mousePos.y = mousepos.y;

	// wheelだけはDirectInputかWMに任せる。
#endif

	// xInputのポーリング（入力データをXINPUT_STATEに取得）
	DWORD dwResult;
	XINPUT_STATE xstate;
	for (i = 0; i < MAX_PADS; i++)
	{
		//  Simply get the state of the controller from XInput.
		ZeroMemory(&xstate, sizeof(XINPUT_STATE));
		dwResult = XInputGetState(i, &xstate);

		if (dwResult == ERROR_SUCCESS)
		{
			m_xPadActive[i] = true;

			// btn更新
			SetXButton(i, (int)X_BUTTONS::DPAD_UP, (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP));
			SetXButton(i, (int)X_BUTTONS::DPAD_DOWN, (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN));
			SetXButton(i, (int)X_BUTTONS::DPAD_LEFT, (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT));
			SetXButton(i, (int)X_BUTTONS::DPAD_RIGHT, (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT));
			SetXButton(i, (int)X_BUTTONS::BTN_START, (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_START));
			SetXButton(i, (int)X_BUTTONS::BTN_BACK, (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_BACK));
			SetXButton(i, (int)X_BUTTONS::BTN_L3, (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB));
			SetXButton(i, (int)X_BUTTONS::BTN_R3, (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB));
			SetXButton(i, (int)X_BUTTONS::BTN_L, (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER));
			SetXButton(i, (int)X_BUTTONS::BTN_R, (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER));

			SetXButton(i, (int)X_BUTTONS::BTN_GUIDE, (xstate.Gamepad.wButtons & 0x0400));
			SetXButton(i, (int)X_BUTTONS::BTN_UNKNOWN, (xstate.Gamepad.wButtons & 0x0800));

			SetXButton(i, (int)X_BUTTONS::BTN_A, (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_A));
			SetXButton(i, (int)X_BUTTONS::BTN_B, (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_B));
			SetXButton(i, (int)X_BUTTONS::BTN_X, (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_X));
			SetXButton(i, (int)X_BUTTONS::BTN_Y, (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_Y));

			// スティックのデッドゾーン反映(細かい部分取りすぎるとキャラがプルプルする)
			//  Zero value if thumbsticks are within the dead zone 
			if ((xstate.Gamepad.sThumbLX < INPUT_DEADZONE_L &&
				xstate.Gamepad.sThumbLX > -INPUT_DEADZONE_L) &&
				(xstate.Gamepad.sThumbLY < INPUT_DEADZONE_L &&
					xstate.Gamepad.sThumbLY > -INPUT_DEADZONE_L))
			{
				xstate.Gamepad.sThumbLX = 0;
				xstate.Gamepad.sThumbLY = 0;
			}

			if ((xstate.Gamepad.sThumbRX < INPUT_DEADZONE_R &&
				xstate.Gamepad.sThumbRX > -INPUT_DEADZONE_R) &&
				(xstate.Gamepad.sThumbRY < INPUT_DEADZONE_R &&
					xstate.Gamepad.sThumbRY > -INPUT_DEADZONE_R))
			{
				xstate.Gamepad.sThumbRX = 0;
				xstate.Gamepad.sThumbRY = 0;
			}

			// アナログスティック更新、LRトリガもアナログ。
			m_analogState[i][(int)X_ANALOGS::L_X] = xstate.Gamepad.sThumbLX;
			m_analogState[i][(int)X_ANALOGS::L_Y] = xstate.Gamepad.sThumbLY;
			m_analogState[i][(int)X_ANALOGS::R_X] = xstate.Gamepad.sThumbRX;
			m_analogState[i][(int)X_ANALOGS::R_Y] = xstate.Gamepad.sThumbRY;
			m_analogState[i][(int)X_ANALOGS::L_TRIGGER] = xstate.Gamepad.bLeftTrigger;
			m_analogState[i][(int)X_ANALOGS::R_TRIGGER] = xstate.Gamepad.bRightTrigger;

		}
		else
		{
			if (m_xPadActive[i])
			{
				// btnリセット
				ZeroMemory(&m_buttonState[i], MAX_PAD_BUTTONS);
				m_xPadActive[i] = false;
			}
		} // if
	} // for

#if DIRECT_INPUT_ACTIVE
	
#if DI_KEY_MOUSE
	// DirectInputでキーボードとマウスを取得する。
	// キーボード取得
	HRESULT hr;
	hr = g_pDIKeyboard->Poll();

	if (FAILED(hr))	// FOREGROUND設定にCOOPRATIVELEVELを設定しているので、後ろに行くと失敗する。
	{
		g_pDIKeyboard->Acquire();	// キーボードチェック開始
		g_pDIMouse->Acquire();		// マウスチェック開始

		g_pDIKeyboard->Poll();		// キーボード取得やり直し
	}

	BYTE currentKeys[MAX_KEYS];
	hr = g_pDIKeyboard->GetDeviceState(MAX_KEYS, currentKeys);	// byteは１バイトなのでMAX_KEYSがそのままサイズ

	if (SUCCEEDED(hr))
	{
		for (i = 0; i < MAX_KEYS; i++)
		{
			m_keyState[i] <<= 1;	// 左１ビットシフト

			if (currentKeys[i] & 0x80)
			{
				m_keyState[i] |= 0x01;
			}
		}
	}

	// マウス取得
	g_pDIMouse->Poll();
	DIMOUSESTATE2 currentMouse;
	hr = g_pDIMouse->GetDeviceState(sizeof(DIMOUSESTATE2), &currentMouse);

	if (SUCCEEDED(hr))
	{
		for (i = 0; i < MAX_MOUSE_BUTTONS; i++)
		{
			m_mouseState[i] <<= 1;	// 左１ビットシフト
			
			if (currentMouse.rgbButtons[i] & 0x80)
			{
				m_mouseState[i] |= 0x01;
			}
		}

		// カーソル移動量
		m_mouseVect.x = currentMouse.lX;
		m_mouseVect.y = currentMouse.lY;

		// マウス座標
		POINT mousepos;
		GetCursorPos(&mousepos);
		ScreenToClient(g_hWnd, &mousepos);
		m_mousePos.x = mousepos.x;
		m_mousePos.y = mousepos.y;

		// ホイール移動量
		m_wheelPos.x = 0;
		m_wheelPos.y = currentMouse.lZ;
	}
#endif

	DIJOYSTATE2 jstate;	// DIフォースフィードバックとか色々対応版。
	int j;

	// 失敗したら削除するだけの処理。
	for (i = 0; i < MAX_DI_PADS; i++)
	{
		if (g_pDIGamePad[i])
		{
			g_pDIGamePad[i]->Poll();

			if (FAILED(g_pDIGamePad[i]->GetDeviceState(sizeof(DIJOYSTATE2), &jstate)))
			{
				// デバイスが外れた可能性
				// 再起動を試みる
				if (FAILED(g_pDIGamePad[i]->Acquire()))
				{
					ReleaseDIGameController(i);
				}
				else
				{
					g_pDIGamePad[i]->Poll();
				}

				// 次フレームへ
			}
			else
			{
				if ((jstate.lX < INPUT_DEADZONE_L &&
					jstate.lX > -INPUT_DEADZONE_L) &&
					(jstate.lY < INPUT_DEADZONE_L &&
						jstate.lY > -INPUT_DEADZONE_L) &&
					(jstate.lZ < INPUT_DEADZONE_L &&
						jstate.lZ > -INPUT_DEADZONE_L))
				{
					jstate.lX = 0;
					jstate.lY = 0;
					jstate.lZ = 0;
				}

				if ((jstate.lRx < INPUT_DEADZONE_R &&
					jstate.lRx > -INPUT_DEADZONE_R) &&
					(jstate.lRy < INPUT_DEADZONE_R &&
						jstate.lRy > -INPUT_DEADZONE_R) &&
					(jstate.lRz < INPUT_DEADZONE_R &&
						jstate.lRz > -INPUT_DEADZONE_R))
				{
					jstate.lRx = 0;
					jstate.lRy = 0;
					jstate.lRz = 0;
				}

				for (j = 0; j < MAX_DI_BUTTONS; j++)
				{
					SetXButton(MAX_PADS + i, j, (jstate.rgbButtons[i] & 0x80));
				}

				for (j = 0; j < 4; j++)
				{
					m_diDirection[i][j] &= 0xf0;
					switch (jstate.rgdwPOV[j])
					{
					case -1:	// 押してない
						break;

					case 0:		// 上
						m_diDirection[i][j] |= (BYTE)DI_POV::DPOV_UP;
						break;

					case 4500:	// 右上
						m_diDirection[i][j] |= (BYTE)(DI_POV::DPOV_UP) | (BYTE)(DI_POV::DPOV_RIGHT);
						break;

					case 9000:	// 右
						m_diDirection[i][j] |= (BYTE)DI_POV::DPOV_RIGHT;
						break;

					case 13500:	// 右下
						m_diDirection[i][j] |= (BYTE)(DI_POV::DPOV_DOWN) | (BYTE)(DI_POV::DPOV_RIGHT);
						break;

					case 18000:	// 下
						m_diDirection[i][j] |= (BYTE)DI_POV::DPOV_DOWN;
						break;

					case 22500:	// 左下
						m_diDirection[i][j] |= (BYTE)(DI_POV::DPOV_DOWN) | (BYTE)(DI_POV::DPOV_LEFT);
						break;

					case 27000:	// 左
						m_diDirection[i][j] |= (BYTE)DI_POV::DPOV_LEFT;
						break;

					case 31500:	// 左上
						m_diDirection[i][j] |= (BYTE)(DI_POV::DPOV_UP) | (BYTE)(DI_POV::DPOV_LEFT);
						break;
					}
				}

				// アナログIN保存。
				m_diAnalogState[i][(int)DI_ANALOGS::L_X] = jstate.lX;
				m_diAnalogState[i][(int)DI_ANALOGS::L_Y] = jstate.lY;
				m_diAnalogState[i][(int)DI_ANALOGS::L_Z] = jstate.lZ;

				m_diAnalogState[i][(int)DI_ANALOGS::R_X] = jstate.lRx;
				m_diAnalogState[i][(int)DI_ANALOGS::R_Y] = jstate.lRy;
				m_diAnalogState[i][(int)DI_ANALOGS::R_Z] = jstate.lRz;

				m_diAnalogState[i][(int)DI_ANALOGS::LA_X] = jstate.lAX;
				m_diAnalogState[i][(int)DI_ANALOGS::LA_Y] = jstate.lAY;
				m_diAnalogState[i][(int)DI_ANALOGS::LA_Z] = jstate.lAZ;

				m_diAnalogState[i][(int)DI_ANALOGS::SLIDER_X] = jstate.rglSlider[0];
				m_diAnalogState[i][(int)DI_ANALOGS::SLIDER_Y] = jstate.rglSlider[1];

				m_diAnalogState[i][(int)DI_ANALOGS::V_SLIDER_X] = jstate.rglVSlider[0];
				m_diAnalogState[i][(int)DI_ANALOGS::V_SLIDER_Y] = jstate.rglVSlider[1];

				m_diAnalogState[i][(int)DI_ANALOGS::F_SLIDER_X] = jstate.rglFSlider[0];
				m_diAnalogState[i][(int)DI_ANALOGS::F_SLIDER_Y] = jstate.rglFSlider[1];

				m_diAnalogState[i][(int)DI_ANALOGS::A_SLIDER_X] = jstate.rglASlider[0];
				m_diAnalogState[i][(int)DI_ANALOGS::A_SLIDER_Y] = jstate.rglASlider[1];

				m_diAnalogState[i][(int)DI_ANALOGS::LAR_X] = jstate.lARx;
				m_diAnalogState[i][(int)DI_ANALOGS::LAR_Y] = jstate.lARy;
				m_diAnalogState[i][(int)DI_ANALOGS::LAR_Z] = jstate.lARz;

				m_diAnalogState[i][(int)DI_ANALOGS::LF_X] = jstate.lFX;
				m_diAnalogState[i][(int)DI_ANALOGS::LF_Y] = jstate.lFY;
				m_diAnalogState[i][(int)DI_ANALOGS::LF_Z] = jstate.lFZ;

				m_diAnalogState[i][(int)DI_ANALOGS::LFR_X] = jstate.lFRx;
				m_diAnalogState[i][(int)DI_ANALOGS::LFR_Y] = jstate.lFRy;
				m_diAnalogState[i][(int)DI_ANALOGS::LFR_Z] = jstate.lFRz;

				m_diAnalogState[i][(int)DI_ANALOGS::LV_X] = jstate.lVX;
				m_diAnalogState[i][(int)DI_ANALOGS::LV_Y] = jstate.lVY;
				m_diAnalogState[i][(int)DI_ANALOGS::LV_Z] = jstate.lVZ;

				m_diAnalogState[i][(int)DI_ANALOGS::LVR_X] = jstate.lVRx;
				m_diAnalogState[i][(int)DI_ANALOGS::LVR_Y] = jstate.lVRy;
				m_diAnalogState[i][(int)DI_ANALOGS::LVR_Z] = jstate.lVRz;
			}
		}
	}
#endif
}

void InputManager::RefreshBuffer()
{
	int i, j;

	// キー状態のリセット
	for (i = 0; i < MAX_KEYS; i++)
	{
		m_keyState[i] &= 0x01;	// Pressのフラグだけ残す
	}

	// ホイルデータのリセット
	m_wheelPos.x = 0;
	m_wheelPos.y = 0;
	m_wheelDelta = 0;

#if DIRECT_INPUT_ACTIVE & DI_KEY_MOUSE
	for (i = 0; i < MAX_MOUSE_BUTTONS; i++)
	{
		m_mouseState[i] &= 0x01;	// DirectInput用マウスボタンのリセット
	}
#endif

	// xInputゲームパッド用バッファのリセット
	for (i = 0; i < MAX_PADS; i++)
	{
		for (j = 0; j < MAX_PAD_ANALOGIN; j++)
		{
			m_analogState[i][j] = 0;
		}

		for (j = 0; j < MAX_PAD_BUTTONS; j++)
		{
			m_buttonState[i][j] &= 0x01;
		}
	}

#if DIRECT_INPUT_ACTIVE
	for (i = 0; i < MAX_DI_PADS; i++)
	{
		for (j = 0; j < MAX_DI_ANALOGIN; j++)
		{
			m_diAnalogState[i][j] = 0;
		}

		// 前フレームのPUSHを4bit左にシフト（前の4bitに前フレームの押下状態が残る。）
		m_diDirection[i][0] <<= 4;
		m_diDirection[i][1] <<= 4;
		m_diDirection[i][2] <<= 4;
		m_diDirection[i][3] <<= 4;

		for (j = 0; j < MAX_DI_BUTTONS; j++)
		{
			m_diButtonState[i][j] &= 0x01;
		}
	}
#endif
}

#if !DIRECT_INPUT_ACTIVE | !DI_KEY_MOUSE

void InputManager::MouseWheel(short delta, short x, short y)
{
	// マウスホイールメッセージからの呼び出し用
	// deltaが+で奥、-で手前へ回転。
	m_wheelPos.x = x;
	m_wheelPos.y = y;
	m_wheelDelta = delta;
}
#endif

bool InputManager::CheckKeyboard(UINT keycode, BUTTON_STATE state)
{
	if (keycode >= MAX_KEYS)
		return false;

	return CheckState(m_keyState[keycode], state);
}

bool InputManager::CheckGamePad(UINT padId, UINT btnId, BUTTON_STATE state)
{
	if (padId < MAX_PADS)
	{
		if (m_xPadActive[padId] && btnId < MAX_PAD_BUTTONS)
			return CheckState(m_buttonState[padId][btnId], state);

		return false;
	}
	else
	{
#if DIRECT_INPUT_ACTIVE
		padId -= MAX_PADS;

		if (padId < MAX_DI_PADS)
		{
			if (g_pDIGamePad[padId] && btnId < MAX_DI_BUTTONS)
				return CheckState(m_diButtonState[padId][btnId], state);

			return false;

		}
#endif
		return false;
	}
}

#if DIRECT_INPUT_ACTIVE & DI_KEY_MOUSE
bool InputManager::CheckMouseButton(UINT btnId, BUTTON_STATE state)
{
	if (btnId >= MAX_MOUSE_BUTTONS)
		return false;

	return checkState(m_mouseState[btnId], state);
}
#endif

XMINT2 InputManager::GetMousePosition()
{
	return m_mousePos;
}

XMINT2 InputManager::GetMouseVector()
{
	return m_mouseVect;
}

XMINT2 InputManager::GetWheelPosition()
{
	return m_wheelPos;
}

INT InputManager::GetWheelDelta()
{
	return m_wheelDelta;
}

INT InputManager::GetAnalogValue(UINT padId, int analogId)
{
	INT res = 0;

	if (padId < MAX_PADS && analogId < MAX_PAD_ANALOGIN && m_xPadActive[padId])
	{
		return m_analogState[padId][analogId];
	}

	return res;
}

void InputManager::SetInputEnable(bool flg)
{
	// 
	if (flg)
	{
		// コントローラの制御ON
#if DIRECT_INPUT_ACTIVE
		
#endif
	}
	else
	{
		// 入力制御OFFなのでコントローラの振動等も全て止める。

#if DIRECT_INPUT_ACTIVE

#endif
		// xInputの振動オフは簡単
		XINPUT_VIBRATION vib = {};

		for (int i = 0; i < MAX_PADS; i++)
			XInputSetState(i, &vib);
	}
}

#if DIRECT_INPUT_ACTIVE
// xInputに初期化処理はない、けどDirectInputにはある。
HRESULT InputManager::InitDirectInput(HINSTANCE hInst)
{

	HRESULT hr;

	if (!g_pDInput)
	{
		// DirectInputオブジェクトの作成
		hr = DirectInput8Create(hInst, DIRECTINPUT_VERSION,
			IID_IDirectInput8, (void**)&g_pDInput, NULL);

		if (FAILED(hr))
		{
			return hr;
		}
	}

#if DI_KEY_MOUSE
	// キーボードデバイス作成
	hr = g_pDInput->CreateDevice(GUID_SysKeyboard, &g_pDIKeyboard, NULL);
	if (FAILED(hr)) return hr;	// 失敗したらここで終了

	// デバイスタイプ設定
	hr = g_pDIKeyboard->SetDataFormat(&c_dfDIKeyboard);	// このデバイスはキーボード
	if (FAILED(hr))	return hr;

	// 協調レベルの設定
	hr = g_pDIKeyboard->SetCooperativeLevel(g_hWnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	// ゲームの場合はFOREGROUNDでウインドウが一番上にある時のみ、NONEXCLUSIVEで他のアプリケーションから制御を奪わない
	if (FAILED(hr))	return hr;


	// マウスデバイス作成
	hr = g_pDInput->CreateDevice(GUID_SysMouse, &g_pDIMouse, NULL);
	if (FAILED(hr)) return hr;	// 失敗したらここで終了

	// デバイスタイプ設定
	hr = g_pDIMouse->SetDataFormat(&c_dfDIMouse2);	// このデバイスはマウスの色々多い方
	if (FAILED(hr))	return hr;

	// 協調レベルの設定
	hr = g_pDIMouse->SetCooperativeLevel(g_hWnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	// ゲームの場合はFOREGROUNDでウインドウが一番上にある時のみ、NONEXCLUSIVEで他のアプリケーションから制御を奪わない
	if (FAILED(hr))	return hr;
#endif

	// DIは起動前にコントローラを繋げてないと自動ではコントローラをONに出来ない。
	hr = g_pDInput->EnumDevices(DI8DEVCLASS_GAMECTRL, (LPDIENUMDEVICESCALLBACKW)EnumDIJoysticksCallback,
		NULL, DIEDFL_ATTACHEDONLY);
	
	if (FAILED(hr))	return hr;

	return hr;
}

// 削除もある。
void InputManager::ReleaseDirectInput()
{
	ReleaseAllDIControllers();
	if (g_pDInput)
	{
		g_pDInput->Release();
		g_pDInput = nullptr;
	}
}

bool InputManager::CheckDirectionButton(UINT padId, UINT povId, X_BUTTONS btnId, BUTTON_STATE state)
{
	if (padId < MAX_PADS && m_xPadActive[padId])
	{
		switch (btnId)
		{
		case X_BUTTONS::DPAD_UP:
		case X_BUTTONS::DPAD_RIGHT:
		case X_BUTTONS::DPAD_DOWN:
		case X_BUTTONS::DPAD_LEFT:
			return CheckState(m_buttonState[padId][(UINT)btnId], state);
		}
	}
	else
	{
		if (povId < 4)
		{
			padId -= MAX_PADS;

			if (padId < MAX_DI_PADS && g_pDIGamePad[padId])
			{
				BYTE flg = m_diDirection[padId][povId];
				BYTE mask = 0;

				switch (btnId)
				{
				case X_BUTTONS::DPAD_UP:
					mask = (BYTE)DI_POV::DPOV_UP;
					break;
				case X_BUTTONS::DPAD_RIGHT:
					mask = (BYTE)DI_POV::DPOV_RIGHT;
					break;
				case X_BUTTONS::DPAD_DOWN:
					mask = (BYTE)DI_POV::DPOV_DOWN;
					break;
				case X_BUTTONS::DPAD_LEFT:
					mask = (BYTE)DI_POV::DPOV_LEFT;
					break;
				}

				switch (state)
				{
				case BUTTON_STATE::BUTTON_UP:
					mask = mask | (mask << 4);
					return (flg & mask) == (0xf0 & mask);	// 下のビットだけが0

				case BUTTON_STATE::BUTTON_DOWN:
					mask = mask | (mask << 4);
					return (flg & mask) == (0x0f & mask);	// 下のビットだけが1

				case BUTTON_STATE::BUTTON_PRESS:			// 下のビットが１
					return (flg & mask);
				}
			}
		}
	}

	return false;
}

LONG InputManager::GetDiAnalogValue(UINT padId, int analogId)
{
	LONG res = 0;

	if (padId > MAX_PADS)
	{
		padId -= MAX_PADS;

		if (padId < MAX_DI_PADS)
		{
			if (analogId < MAX_DI_ANALOGIN)
			{
				return m_diAnalogState[padId][analogId];
			}
		}
	}

	return res;
}

// これはprotected
void InputManager::ReleaseDIGameController(int conId)
{
	if (conId < MAX_DI_PADS)
	{
		if (g_pDIGamePad[conId])
		{
			g_pDIGamePad[conId]->Unacquire();
			g_pDIGamePad[conId]->Release();
			g_pDIGamePad[conId] = nullptr;
		}
	}
}

void InputManager::ReleaseAllDIControllers()
{
#if DIRECT_INPUT_ACTIVE & DI_KEY_MOUSE
	if (g_pDIKeyboard != nullptr)
	{
		g_pDIKeyboard->Unacquire();
		g_pDIKeyboard->Release();
		g_pDIKeyboard = nullptr;
	}

	if (g_pDIMouse != nullptr)
	{
		g_pDIMouse->Unacquire();
		g_pDIMouse->Release();
		g_pDIMouse = nullptr;
	}
#endif

	for (int i = 0; i < MAX_DI_PADS; i++)
	{
		ReleaseDIGameController(i);
	}

	g_DICount = 0;
}

#endif

void InputManager::SetXButton(UINT padId, UINT btnId, bool push)
{
	BYTE* targetState = nullptr;
	if (padId < MAX_PADS)
	{
		targetState = m_buttonState[padId];
	}
	else
	{
#if DIRECT_INPUT_ACTIVE
		padId -= MAX_PADS;
		if (padId < MAX_DI_PADS)
		{
			targetState = m_diButtonState[padId];
		}
		else
		{
			return;
		}
#else
		return;
#endif
	}

	targetState[btnId] <<= 1;		// 前フレームのボタン状態

	if (push)
	{
		targetState[btnId] |= 0x01;	// press	押されている
	}
}

bool InputManager::CheckState(UINT flg, BUTTON_STATE state)
{
	switch (state)
	{
	case BUTTON_STATE::BUTTON_PRESS:
		return ((flg & 0x01) == 0x01);
	case BUTTON_STATE::BUTTON_DOWN:
		return (flg == 0x01);
	case BUTTON_STATE::BUTTON_UP:
		return (flg == 0x02);
	}
	return false;
}
