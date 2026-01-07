#pragma once

#include <Windows.h>
#include <d3d12.h>
#include <unordered_map>
#include <memory>
#include <wrl/client.h>

#include <DirectXMath.h>

using namespace std;
using namespace DirectX;
using Microsoft::WRL::ComPtr;

struct SpriteVertex
{
	XMFLOAT3 Pos;
	XMFLOAT2 Tex;
};

class MeshManager
{
private:
	struct BufferContainer
	{
		UINT	dataSize = 0;
		UINT	dataCount = 0;
		ComPtr<ID3D12Resource> pBuffer = nullptr;
	};

	struct ViewContainer
	{
		D3D12_VERTEX_BUFFER_VIEW vbView;
		D3D12_INDEX_BUFFER_VIEW	ibView;
	};
	unordered_map<std::wstring, unique_ptr<BufferContainer>> m_VertexBuffers;
	unordered_map<std::wstring, unique_ptr<BufferContainer>> m_IndexBuffers;
	unordered_map<std::wstring, unique_ptr<ViewContainer>> m_ViewContainers;

	std::wstring m_crVertex;
	std::wstring m_crIndex;

public:
	void CreatePresetMeshData();

	void SetVertexBuffer(ID3D12GraphicsCommandList* m_cmdList, const std::wstring idName);
	void SetIndexBuffer(ID3D12GraphicsCommandList* m_cmdList, const std::wstring idName);

	HRESULT AddVertexBuffer(const std::wstring idName, const void* initBuff, UINT vertexSize, UINT vertexCount);
	HRESULT AddIndexBuffer(const std::wstring idName, const void* initBuff, UINT valueSize, UINT indexCount);

	D3D12_VERTEX_BUFFER_VIEW* GetVertexBufferView(const std::wstring idName);
	D3D12_INDEX_BUFFER_VIEW* GetIndexBufferView(const std::wstring idName);

	void RemoveVertexBuffer(const std::wstring id, bool withIndex);
	void RemoveIndexBuffer(const std::wstring id);

	void ResetMesh();
};