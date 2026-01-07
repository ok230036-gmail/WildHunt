#pragma once
#include <d3d12.h>
#include <DirectXMath.h>
#include <vector>

#include <wrl/client.h>

#include <PipeLineManager.h>

using namespace DirectX;
using Microsoft::WRL::ComPtr;

class GraphicsPipeLineObjectBase;

class CharacterData
{
protected:
	XMFLOAT3	position;			// キャラクタの中心位置
	XMFLOAT3	rotation;			// キャラクタの角度
	XMFLOAT3	scale;				// キャラクタのスケール

	XMMATRIX	m_worldMtx;			// ワールドマトリクス
	XMMATRIX	m_worldMtxInv;		// ワールドマトリクスの逆行列
	bool		m_mtxChange;		// マトリクスの変更フラグ

	std::vector<ComPtr<ID3D12Resource>> m_constantBuffers;

	GraphicsPipeLineObjectBase* m_pPipeLine = nullptr;

	UINT m_cbuffCount;

	std::wstring m_camera;

public:
	CharacterData()
	{
		SetPosition(0.0f, 0.0f, 0.0f);
		SetRotation(0.0f, 0.0f, 0.0f);
		SetScale(1.0f, 1.0f, 1.0f);

		m_constantBuffers.clear();
		m_cbuffCount = 0;

		m_camera = L"MainCamera";
	}

	virtual ~CharacterData() {}

	void AddConstantBuffer(UINT buffSize, const void* initData);

	ID3D12Resource* GetConstantBuffer(UINT index)
	{
		if (m_cbuffCount > index)
		{
			return m_constantBuffers[index].Get();
		}

		return nullptr;
	}

	void SetPosition(float x, float y, float z)
	{
		position.x = x;
		position.y = y;
		position.z = z;

		m_mtxChange = true;
	}

	void SetRotation(float x, float y, float z)
	{
		rotation.x = x;
		rotation.y = y;
		rotation.z = z;

		m_mtxChange = true;
	}

	void SetScale(float x, float y, float z)
	{
		scale.x = x;
		scale.y = y;
		scale.z = z;

		m_mtxChange = true;
	}

	// 現在位置を取得
	const XMFLOAT3 GetPosition()
	{
		return position;
	}

	// 現在角度を取得
	const XMFLOAT3 GetRotation()
	{
		return rotation;
	}

	// 現在スケールを取得
	const XMFLOAT3 GetScale()
	{
		return scale;
	}

	void SetGraphicsPipeLine(std::wstring pipelineName);

	GraphicsPipeLineObjectBase* GetPipeline()
	{
		return m_pPipeLine;
	}

	void SetCameraLabel(std::wstring label)
	{
		m_camera = label;
	}

	std::wstring GetCameraLabel()
	{
		return m_camera;
	}

	XMMATRIX& GetWorldMatrix();			// そのままでワールドマトリクスを取得
	XMMATRIX& GetInverseWorldMatrix();	// そのままでワールドマトリクスの逆行列を取得

	void SetWorldMatrix(XMMATRIX mat);	// アフィン変換行列設定
};

