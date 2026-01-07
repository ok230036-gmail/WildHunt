#include <MyAccessHub.h>
#include <MyGameEngine.h>
#include "CharacterData.h"

void CharacterData::AddConstantBuffer(UINT buffSize, const void* initData)
{
	MyGameEngine* engine = MyAccessHub::GetMyGameEngine();
	if (m_constantBuffers.size() <= m_cbuffCount)
	{
		m_constantBuffers.resize(m_cbuffCount + 5);
	}
	m_constantBuffers[m_cbuffCount].Reset();
	
	engine->CreateConstantBuffer(m_constantBuffers[m_cbuffCount].GetAddressOf(), initData, buffSize);

	m_cbuffCount++;
}

void CharacterData::SetGraphicsPipeLine(std::wstring pipelineName)
{
	PipeLineManager* plMng = MyAccessHub::GetMyGameEngine()->GetPipelineManager();

	m_pPipeLine = plMng->GetPipeLineObject(pipelineName);
}

// ワールドマトリクスを取得
XMMATRIX& CharacterData::GetWorldMatrix()
{
	if (m_mtxChange)
	{
		m_mtxChange = false;

		// Worldマトリクス更新
		XMMATRIX model_matrix;
		XMMATRIX translate = XMMatrixTranslation(position.x, position.y, position.z);
		XMMATRIX rotate_x = XMMatrixRotationX(XMConvertToRadians(rotation.x));
		XMMATRIX rotate_y = XMMatrixRotationY(XMConvertToRadians(rotation.y));
		XMMATRIX rotate_z = XMMatrixRotationZ(XMConvertToRadians(rotation.z));
		XMMATRIX scale_mat = XMMatrixScaling(scale.x, scale.y, scale.z);
		model_matrix = scale_mat * rotate_z * rotate_x * rotate_y * translate;

		m_worldMtx = model_matrix;

		// ここで、逆行列も取得しておく
		m_worldMtxInv = XMMatrixInverse(nullptr, model_matrix);
	}

	return m_worldMtx;
}

// ワールドマトリクスの逆行列を取得するメソッド
XMMATRIX& CharacterData::GetInverseWorldMatrix()
{
	if (m_mtxChange)
		GetWorldMatrix();

	return m_worldMtxInv;
}

// スクリプト側でワールドマトリクスを変更するときの処理(m_mtxChangeをfalseにして、勝手に変更しないように)
void CharacterData::SetWorldMatrix(XMMATRIX xmat)
{
	m_mtxChange = false;
	m_worldMtx = xmat;
	m_worldMtxInv = XMMatrixInverse(nullptr, xmat);
}