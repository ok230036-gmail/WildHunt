#include <MyAccessHub.h>	// MyGameEngineを使うから

#include "LightSettingManager.h"

DirectionalLightContainer::DirectionalLightContainer()
{
	auto myEngine = MyAccessHub::GetMyGameEngine();		// CreateConstantBufferを使うよ。

	m_lightData.Color = XMVectorSet(0, 0, 0, 0);		// 初期状態データは全部０
	m_lightData.Direction = XMVectorSet(0, 0, 0, 0);

	// シェーダリソース作成
	myEngine->CreateConstantBuffer(m_cBuff.GetAddressOf(), &m_lightData, 
		sizeof(DirectionalLightContainer::DirectionalLightData));

	m_lightViewMtx = {};
	m_lightProjectionMtx = {};
}

void DirectionalLightContainer::SetDirectionalLight(XMFLOAT3 color, XMFLOAT3 direction)
{
	// 引数の値を反映
	m_lightData.Color = XMVectorSet(color.x, color.y, color.z, 0.0f);

	// ConstantBufferはXMFLOAT3だとシェーダ側でずれるから、XMVECTORに変換
	m_lightData.Direction = XMVectorSet(direction.x, direction.y, direction.z, 0.0f);
	
	UpdateCBuffer(); // リソース更新
}

DirectionalLightContainer::DirectionalLightData& DirectionalLightContainer::GetLightData()
{
	return m_lightData;	// 光源データ取得
}

ID3D12Resource* DirectionalLightContainer::GetConstantBuffer()
{
	return m_cBuff.Get();	// シェーダリソース取得。普段使われるのはこちら。
}

void DirectionalLightContainer::UpdateLightBaseMatrix(XMFLOAT3& eyePos, XMFLOAT3& focusPos)
{
	XMVECTOR camPosVect = XMLoadFloat3(&eyePos);		// カメラ位置
	XMVECTOR focusPosVect = XMLoadFloat3(&focusPos);	// フォーカスする（カメラが向く）座標
	XMVECTOR lightDir = m_lightData.Direction;

	// フォーカス位置から光の方向の逆に現在のカメラ距離の２０倍移動した位置を光源位置としている。
	XMVECTOR lightPos = focusPosVect -
		XMVector3Normalize(lightDir) * XMVector3Length(XMVectorSubtract(focusPosVect, camPosVect)).m128_f32[0] * 20.0f;

	// ここから先はCameraComponentと同じ処理。
	XMVECTOR Up = XMVectorSet(0, 1, 0, 0.0f); // カメラの上方向単位ベクトル（カメラのロール軸）
	m_lightViewMtx = XMMatrixLookAtLH(lightPos, focusPosVect, Up); // View行列作成
	XMMATRIX trMat = XMMatrixTranspose(m_lightViewMtx);
}

void DirectionalLightContainer::CreateLightProjectionMtx(UINT width, UINT height)
{
	// とりあえず1/100で。
	m_lightProjectionMtx = XMMatrixOrthographicLH(width * 0.01f,
		height * 0.01f, 0.01f, 100.0f);
}

XMMATRIX DirectionalLightContainer::GetLightViewMtx()
{
	return m_lightViewMtx;
}

XMMATRIX DirectionalLightContainer::GetLightProjectionMtx()
{
	return m_lightProjectionMtx;
}

void DirectionalLightContainer::UpdateCBuffer()
{
	// UpdateShaderResourceを使う。
	auto myEngine = MyAccessHub::GetMyGameEngine();

	// 更新処理
	myEngine->UpdateShaderResourceOnGPU(m_cBuff.Get(), &m_lightData, 
		sizeof(DirectionalLightContainer::DirectionalLightData));
}

AmbientLightContainer::AmbientLightContainer()
{
	auto myEngine = MyAccessHub::GetMyGameEngine();
	m_color = XMVectorZero(); // 初期値は0

	// シェーダリソース作成
	myEngine->CreateConstantBuffer(m_cBuff.GetAddressOf(), &m_color, sizeof(XMVECTOR));
}

void AmbientLightContainer::UpdateCBuffer()
{
	// シェーダリソース更新処理
	auto myEngine = MyAccessHub::GetMyGameEngine();
	myEngine->UpdateShaderResourceOnGPU(m_cBuff.Get(), &m_color, sizeof(XMVECTOR));
}

void AmbientLightContainer::SetLight(float r, float g, float b)
{
	m_color = XMVectorSet(r, g, b, 1.0f);	// 設定＋更新処理本体
	UpdateCBuffer();
}

void AmbientLightContainer::SetLight(XMFLOAT3 color)
{
	SetLight(color.x, color.y, color.z);	// 上のオーバーロードメソッドを呼ぶ
}

LightSettingManager::LightSettingManager()
{
	// 各データ保存用unordered_mapのリセット。
	m_AmbientLights.clear();
	m_DirectionalLights.clear();
}

LightSettingManager::~LightSettingManager()
{
	// unique_ptrなのでclearされたら全てデストラクタ。
	m_AmbientLights.clear();
	m_DirectionalLights.clear();
}

LightSettingManager* LightSettingManager::GetInstance()
{
	// staticローカル変数を利用したシングルトン化。
	static LightSettingManager* instance = new LightSettingManager();
	return instance;
}

void LightSettingManager::CreateAmbientLight(std::wstring label, XMFLOAT3 light)
{
	if (m_AmbientLights.find(label) != m_AmbientLights.end())
	{
		// 同じ名前のデータがある場合はデータの書き換え
		m_AmbientLights[label]->SetLight(light);
		return;
	}

	std::unique_ptr<AmbientLightContainer> ambLight = std::make_unique<AmbientLightContainer>();	// ユニークポインタ作成

	// 引数で持ってきた色データを光源にセット
	ambLight->SetLight(light);

	// ユニークポインタな光源をunordered_mapに移動。
	m_AmbientLights[label] = std::move(ambLight);
}

void LightSettingManager::CreateDirectionalLight(std::wstring label, XMFLOAT3 light, XMFLOAT3 direction)
{
	if (m_DirectionalLights.find(label) != m_DirectionalLights.end())
	{
		// 同じ名前のデータがある場合はデータの書き換え
		m_DirectionalLights[label]->SetDirectionalLight(light, direction);
		return;
	}

	std::unique_ptr<DirectionalLightContainer> dirLight = std::make_unique<DirectionalLightContainer>();
	dirLight->SetDirectionalLight(light, direction);	// 作成とデータセット

	m_DirectionalLights[label] = std::move(dirLight);	// unordered_mapへ

	// 平行光源用シャドウマップテクスチャを作成
	auto myEngine = MyAccessHub::GetMyGameEngine();
	myEngine->GetTextureManager()->CreateDepthStencilTexture(
		myEngine->GetDirect3DDevice(),
		label + L"_Depth", // ライト名_Depthでテクスチャ名を統一
		2048,	// 幅
		2048	// 高さ
	);
	m_DirectionalLights[label]->CreateLightProjectionMtx(2048, 2048); // Projection行列作成
}

AmbientLightContainer* LightSettingManager::GetAmbientLight(std::wstring label)
{
	// labelの名前のデータが無いかどうか
	if (m_AmbientLights.find(label) == m_AmbientLights.end())
		return nullptr;

	return m_AmbientLights[label].get();
}

DirectionalLightContainer* LightSettingManager::GetDirectionalLight(std::wstring label)
{
	// labelの名前のデータが無いかどうか
	if (m_DirectionalLights.find(label) == m_DirectionalLights.end())
		return nullptr;

	return m_DirectionalLights[label].get();
}

// 環境光データの削除
void LightSettingManager::DeleteAmbientLight(std::wstring label)
{
	// 該当データがあれば削除
	if (m_AmbientLights.find(label) != m_AmbientLights.end())
		m_AmbientLights.erase(label);
}

// 平行光源データの削除
void LightSettingManager::DeleteDirectionalLight(std::wstring label)
{
	// 該当データがあれば削除
	if (m_DirectionalLights.find(label) != m_DirectionalLights.end())
		m_DirectionalLights.erase(label);
}