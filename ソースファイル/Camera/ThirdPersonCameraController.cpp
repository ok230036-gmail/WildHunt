#include <MyAccessHub.h>
#include "CameraComponent.h"
#include "ThirdPersonCameraController.h"

#include "WildHuntScene.h"

// コンポーネント初期化時に呼ばれる処理
void ThirdPersonCameraController::InitAction()
{
	WildHuntScene* scene = dynamic_cast<WildHuntScene*>(MyAccessHub::GetMyGameEngine()->GetSceneController());
	m_keyBind = dynamic_cast<KeyBindComponent*>(scene->GetKeyComponent());

	auto components = GetGameObject()->GetComponents();

	m_camera = nullptr;
	for (auto comp : components)
	{
		m_camera = dynamic_cast<CameraComponent*>(comp);
		if (m_camera != nullptr)	// dynamic_castは失敗するとnullptr
		{
			break;
		}
	}

	m_hDeg = 0.0f;
	m_vDeg = 0.0f;
	m_distance = 3.5f;

	m_cameraShakeFlag = false;
	m_cameraShakeCnt = 0;
	m_cameraShakeMove = 0.05f;
}

// 毎フレーム呼ばれる処理　falseを返すとこのコンポーネントは終了し削除される
bool ThirdPersonCameraController::FrameAction()
{
	int move_x, move_y;
	CharacterData* chData = GetGameObject()->GetCharacterData();
	float height = m_camera->GetSetHeight();
	float width = m_camera->GetSetWidth();

	if (m_keyBind->GetCurrentInputType() == KeyBindComponent::INPUT_TYPES::KEYBOARD)
	{
		// マウス座標から角度を生成
		XMINT2 mousePos = MyAccessHub::GetMyGameEngine()->GetInputManager()->GetMousePosition();

		float half_h = height * 0.5f;	// 高さの半分
		float quat_w = width * 0.125f;	// 幅は1/8

		move_x = mousePos.x - quat_w;	// 左右はWindow幅全体で１回転出来るように
		move_y = mousePos.y - half_h;	// 上下は１８０度で止まるようにしている。

		m_vDeg = move_y / half_h * 90.0f;	// MAX+-90度（特に上下は90度を越えないように。越えると急に見ている向きが変わる）
		m_hDeg = move_x / quat_w * 90.0f;
	}
	else
	{
		// パッドはアナログ値の加算になる。
		float f_temp = 0.0f;

		move_x = m_keyBind->GetCurrentAnalogValue(KeyBindComponent::ANALOG_IDS::CAMERA_H);
		move_y = m_keyBind->GetCurrentAnalogValue(KeyBindComponent::ANALOG_IDS::CAMERA_V);

		f_temp = move_y / (height * 0.5f) * 90.0f;	// MAX90度
		m_vDeg += f_temp;
		m_vDeg = m_vDeg < -90.0f ? -90.0f : m_vDeg > 90.0f ? 89.9999f : m_vDeg;	// ジャスト90.0fになると計算がバグって回転が０になるので89.9999。

		f_temp = move_x / (width * 0.25f) * 90.0f;	// 横は180度回ってもそれほど問題にはならないよ。
		m_hDeg += f_temp;
		m_hDeg = m_hDeg < -180.0f ? m_hDeg + 360.0f : m_hDeg > 180.0f ? m_hDeg - 360.0f : m_hDeg;
	}

	// マウス入力からカメラの位置をフォーカス点を中心に移動
	// 角度ゼロ、原点中心のカメラ座標を作ってX軸とY軸で回転
	XMVECTOR pos = { 0.0f, 0.0f, m_distance, 0.0f };
	XMMATRIX mat = XMMatrixRotationX(XMConvertToRadians(m_vDeg))* XMMatrixRotationY(XMConvertToRadians(m_hDeg));

	XMFLOAT3 focus = m_camera->GetCameraFocus();
	mat = mat * XMMatrixTranslation(focus.x, focus.y, focus.z);	// 回転マトリクスに平行移動を掛ける（中心の移動）

	pos = XMVector3Transform(pos, mat);	// 座標に完成したマトリクスを掛けてアフィン変換を座標に反映する。

	WildHuntScene* p_scene = static_cast<WildHuntScene*>(MyAccessHub::GetMyGameEngine()->GetSceneController());
	TerrainComponent* terCom = p_scene->GetTerrainComponent(0);
	XMFLOAT3 hitPos = {};		// 判定接触点（結果用）
	XMFLOAT3 hitNormal = {};	// 接触ポリゴンの法線（結果用）
	XMFLOAT3 rayStart;			// 線分開始点
	XMFLOAT3 rayEnd;			// 線分終了点
	HitRayLine ray;				// 線分判定（レイ）

	rayStart = chData->GetPosition();
	rayStart.y += 5.0f;
	XMStoreFloat3(&rayEnd, pos);
	rayEnd.y -= 1.0f;

	ray.SetLine(rayStart, rayEnd, 0.0f);

	if (terCom->RayCastHit(ray, hitPos, hitNormal))
	{
		// X,Zをヒット位置に補正
		pos.m128_f32[0] = hitPos.x;
		pos.m128_f32[1] = hitPos.y + 0.5f;
		pos.m128_f32[2] = hitPos.z;
	}

	if (m_cameraShakeFlag)
	{
		m_cameraShakeCnt++;
		pos.m128_f32[0] += m_cameraShakeMove;
		pos.m128_f32[1] += m_cameraShakeMove;

		m_cameraShakeMove *= -1;

		if (m_cameraShakeCnt >= 20)
		{
			m_cameraShakeFlag = false;
		}
	}

	// 出力を座標に反映する
	m_camera->ChangeCameraPosition(pos.m128_f32[0], pos.m128_f32[1], pos.m128_f32[2]);

	return true;
}

// 終了時に呼ばれる処理
void ThirdPersonCameraController::FinishAction()
{
}
