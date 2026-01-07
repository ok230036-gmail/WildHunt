#pragma once

#include <Windows.h>		// xinputのinclude前にWindows.hのincludeがないとビルド失敗するケース有り。コンパイルの順番問題
#include <directxmath.h>
#include <Xinput.h>

#pragma comment(lib, "xinput.lib")

// DirectInput（XBOXではないUSBコントローラ）設定。
#define DIRECT_INPUT_ACTIVE 1	// XBOXやWindowsStoreAppだと不要。Steamだとほぼ必須。
#define DI_KEY_MOUSE 0			// キーボードとマウスをDirectInputで取るモード

#if DIRECT_INPUT_ACTIVE
#include <dinput.h>
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "dinput8.lib")

#define MAX_DI_PADS 4
#define MAX_DI_BUTTONS 32		// DirectInputのボタンは最大128
#define MAX_DI_ANALOGIN 32
#endif


#define MAX_KEYS 256			// キーボード用最大キー数 DirectInputのキーボード固定値が256
#if DIRECT_INPUT_ACTIVE & DI_KEY_MOUSE
#define MAX_MOUSE_BUTTONS 5		// こっちはマウス用
#endif
#define MAX_PAD_BUTTONS 16		// 上下左右＋10ボタンまで対応
#define MAX_PAD_ANALOGIN 8		// アナログ入力の数	XYをバラで取るので８
#define MAX_PADS 4				// xInputのパッドは４つまで

#define INPUT_DEADZONE_L  ( 0.24f * FLOAT(0x7FFF) )  //  Default to 24% of the +/- 32767 range.   This is a reasonable default value but can be altered if needed.
#define INPUT_DEADZONE_R  ( 0.24f * FLOAT(0x7FFF) )  //  Default to 24% of the +/- 32767 range.   This is a reasonable default value but can be altered if needed.

using namespace DirectX;

class InputManager
{
public:

	enum class BUTTON_STATE
	{
		BUTTON_DOWN,
		BUTTON_PRESS,
		BUTTON_UP
	};

	enum class X_BUTTONS
	{
		DPAD_UP = 0,
		DPAD_DOWN,
		DPAD_LEFT,
		DPAD_RIGHT,
		BTN_START,
		BTN_BACK,
		BTN_L3,
		BTN_R3,
		BTN_L,
		BTN_R,

		BTN_GUIDE,
		BTN_UNKNOWN,		// 多分未使用

		BTN_A,
		BTN_B,
		BTN_X,
		BTN_Y,
	};

	enum class X_ANALOGS
	{
		L_X = 0,
		L_Y,
		L_Z,
		R_X,
		R_Y,
		R_Z,
		L_TRIGGER,
		R_TRIGGER,
	};

	enum class DI_ANALOGS
	{
		L_X = 0,
		L_Y,
		L_Z,
		R_X,
		R_Y,
		R_Z,

		SLIDER_X,
		SLIDER_Y,
		V_SLIDER_X,
		V_SLIDER_Y,
		A_SLIDER_X,
		A_SLIDER_Y,
		F_SLIDER_X,
		F_SLIDER_Y,

		LA_X,
		LA_Y,
		LA_Z,
		LAR_X,
		LAR_Y,
		LAR_Z,

		LF_X,
		LF_Y,
		LF_Z,
		LFR_X,
		LFR_Y,
		LFR_Z,

		LV_X,
		LV_Y,
		LV_Z,
		LVR_X,
		LVR_Y,
		LVR_Z,
	};

	static InputManager* GetInstance();	// シングルトンインスタンス取得staticメソッド
	~InputManager();					// デストラクタ

	HRESULT InitInputManager(HINSTANCE hInst, HWND hwnd);	// InputManager初期化
	void Update();						// 入力データ更新処理
	void RefreshBuffer();				// 入力データリセット処理

#if !DIRECT_INPUT_ACTIVE | !DI_KEY_MOUSE
	void MouseWheel(short delta, short x, short y);
#endif

	bool CheckKeyboard(UINT keycode, BUTTON_STATE state);
	bool CheckGamePad(UINT padId, UINT btnId, BUTTON_STATE state);
#if DIRECT_INPUT_ACTIVE & DI_KEY_MOUSE
	bool CheckMouseButton(UINT btnId, BUTTON_STATE state);
#endif

	XMINT2	GetMousePosition();
	XMINT2	GetMouseVector();
	XMINT2	GetWheelPosition();
	int		GetWheelDelta();
	INT		GetAnalogValue(UINT padId, int analogId);

	void SetInputEnable(bool flg);
#if DIRECT_INPUT_ACTIVE
	// DirectInputの終了
	void ReleaseDirectInput();

	// POVチェック
	bool CheckDirectionButton(UINT padId, UINT povId, X_BUTTONS btnId, BUTTON_STATE state);
	// アナログチェック
	LONG GetDiAnalogValue(UINT padId, int analogId);
#endif

protected:

#if DIRECT_INPUT_ACTIVE
	// DirectInput初期化
	HRESULT InitDirectInput(HINSTANCE hInst);

	enum class DI_POV
	{
		DPOV_UP = 0x01,
		DPOV_RIGHT = 0x02,
		DPOV_DOWN = 0x04,
		DPOV_LEFT = 0x08,
	};
	BYTE m_diDirection[MAX_DI_PADS][4];
	BYTE m_diButtonState[MAX_DI_PADS][MAX_DI_BUTTONS];
	LONG m_diAnalogState[MAX_DI_PADS][MAX_DI_ANALOGIN];
#endif

	BYTE m_keyState[MAX_KEYS];				// キーボードとマウスのボタン
#if DIRECT_INPUT_ACTIVE & DI_KEY_MOUSE
	BYTE m_mouseState[MAX_MOUSE_BUTTONS];	// DIモード　マウスボタン
#endif
	BYTE m_buttonState[MAX_PADS][MAX_PAD_BUTTONS];		// コントローラボタン
	INT m_analogState[MAX_PADS][MAX_PAD_ANALOGIN];		// コントローラアナログスティック

	bool m_xPadActive[MAX_PADS];			// コントローラON/OFF

	XMINT2	m_mousePos = { 0, 0 };			// マウス座標
	XMINT2	m_mouseVect = { 0, 0 };			// マウス移動ベクトル
	XMINT2	m_wheelPos = { 0, 0 };			// マウスホイール発生位置
	INT		m_wheelDelta;					// マウスホイール移動値

	void SetXButton(UINT padId, UINT btnId, bool push);
	bool CheckState(UINT flg, BUTTON_STATE state);
#if DIRECT_INPUT_ACTIVE
	void ReleaseDIGameController(int conId);
	void ReleaseAllDIControllers();
#endif

	InputManager();
};