#include <d3d12.h>
#include <MyAccessHub.h>
#include "MeshManager.h"
#include "DXSampleHelper.h" // ThrowIfFailed等

void MeshManager::CreatePresetMeshData()
{
    HRESULT hr;
    // IndexBuffer
    ULONG indices[] =
    {
        1,0,2,
        0,3,2,
    };

    hr = AddIndexBuffer(L"Sprite", indices, sizeof(ULONG), 6);

    // VertexBuffer
    UINT stride = sizeof(SpriteVertex);
    UINT offset = 0;
    SpriteVertex vertices[] =                               // Sprite用頂点データ作成
    {
        { XMFLOAT3(-0.5f, -0.5f, 1.0f), XMFLOAT2(0, 1) },   // 左上
        { XMFLOAT3(0.5f, -0.5f, 1.0f), XMFLOAT2(1, 1) },    // 右下
        { XMFLOAT3(0.5f, 0.5f, 1.0f), XMFLOAT2(1, 0) },     // 右上
        { XMFLOAT3(-0.5f, 0.5f, 1.0f), XMFLOAT2(0, 0) },    // 左下
    };

    hr = AddVertexBuffer(L"Sprite", vertices, sizeof(SpriteVertex), 4);

    // DecalCubeにはUV値不要。
    SpriteVertex decalV[] =
    {
        { XMFLOAT3(-0.5f, 0.5f, -0.5f), XMFLOAT2(0, 0) },   // 左下　前
        { XMFLOAT3(0.5f, 0.5f, -0.5f), XMFLOAT2(1, 0) },    // 右上　前
        { XMFLOAT3(-0.5f, -0.5f, -0.5f), XMFLOAT2(0, 1) },  // 左上　前
        { XMFLOAT3(0.5f, -0.5f, -0.5f), XMFLOAT2(1, 1) },   // 右下　前

        { XMFLOAT3(-0.5f, 0.5f, 0.5f), XMFLOAT2(0, 0) },    // 左下　奥
        { XMFLOAT3(0.5f, 0.5f, 0.5f), XMFLOAT2(1, 0) },     // 右上　奥
        { XMFLOAT3(-0.5f, -0.5f, 0.5f), XMFLOAT2(0, 1) },   // 左上　奥
        { XMFLOAT3(0.5f, -0.5f, 0.5f), XMFLOAT2(1, 1) },    // 右下　奥
    };

    UINT decalI[] =
    {
        0, 1, 2,    //  side 1
        2, 1, 3,
        4, 0, 6,    //  side 2
        6, 0, 2,
        7, 5, 6,    //  side 3
        6, 5, 4,
        3, 1, 7,    //  side 4
        7, 1, 5,
        4, 5, 0,    //  side 5
        0, 5, 1,
        3, 7, 2,    //  side 6
        2, 7, 6,
    };

    hr = AddVertexBuffer(L"Decal", decalV, sizeof(SpriteVertex), 8);
    hr = AddIndexBuffer(L"Decal", decalI, sizeof(ULONG), 36);
}

void MeshManager::SetVertexBuffer(ID3D12GraphicsCommandList* m_cmdList, const std::wstring idName)
{
    if (m_crVertex == idName)
    {
        return; // 現在設定中のVertexBufferと同じなので更新しない。
    }

    // IDを保存
    m_crVertex = idName;

    // 一度に複数個（マルチメッシュ）つっこめる。
    m_cmdList->IASetVertexBuffers(0, 1, &m_ViewContainers[idName]->vbView);
}

void MeshManager::SetIndexBuffer(ID3D12GraphicsCommandList* m_cmdList, const std::wstring idName)
{
    if (m_crIndex == idName)
    {
        return; // 現在設定中のVertexBufferと同じなので更新しない。
    }

    // IDを保存
    m_crIndex = idName;

    m_cmdList->IASetIndexBuffer(&m_ViewContainers[idName]->ibView);
}

HRESULT MeshManager::AddVertexBuffer(const std::wstring idName, const void* initBuff, UINT vertexSize, UINT vertexCount)
{

    m_VertexBuffers[idName].reset();
    m_VertexBuffers[idName] = make_unique<BufferContainer>();
    m_VertexBuffers[idName]->dataCount = vertexCount;
    m_VertexBuffers[idName]->dataSize = vertexSize;
    m_VertexBuffers[idName]->pBuffer.Reset();

	HRESULT hr = MyAccessHub::GetMyGameEngine()->CreateVertexBuffer(m_VertexBuffers[idName]->pBuffer.GetAddressOf(), initBuff, vertexSize, vertexCount);

    if (FAILED(hr))
        return hr;

    m_VertexBuffers[idName]->pBuffer->SetName(idName.c_str());

    if (m_ViewContainers.find(idName) == m_ViewContainers.end())
    {
        m_ViewContainers[idName].reset();
        m_ViewContainers[idName] = make_unique<ViewContainer>();
    }

    m_ViewContainers[idName]->vbView.BufferLocation = m_VertexBuffers[idName]->pBuffer->GetGPUVirtualAddress();
    m_ViewContainers[idName]->vbView.SizeInBytes = vertexSize * vertexCount;
    m_ViewContainers[idName]->vbView.StrideInBytes = vertexSize;

    return hr;

}

HRESULT MeshManager::AddIndexBuffer(const std::wstring idName, const void* initBuff, UINT valueSize, UINT indexCount)
{
    m_IndexBuffers[idName].reset();
    m_IndexBuffers[idName] = make_unique<BufferContainer>();
    m_IndexBuffers[idName]->dataSize = valueSize;
    m_IndexBuffers[idName]->dataCount = indexCount;
    m_IndexBuffers[idName]->pBuffer.Reset();

    HRESULT hr = MyAccessHub::GetMyGameEngine()->CreateIndexBuffer(m_IndexBuffers[idName]->pBuffer.GetAddressOf(), initBuff, valueSize, indexCount);
    if (FAILED(hr))
        return hr;

    m_IndexBuffers[idName]->pBuffer->SetName(idName.c_str());

    if (m_ViewContainers.find(idName) == m_ViewContainers.end())
    {
        m_ViewContainers[idName].reset();
        m_ViewContainers[idName] = make_unique<ViewContainer>();
    }
    m_ViewContainers[idName]->ibView.BufferLocation = m_IndexBuffers[idName]->pBuffer->GetGPUVirtualAddress();
    m_ViewContainers[idName]->ibView.SizeInBytes = valueSize * indexCount;

    switch (valueSize)
    {
    case 4:
        m_ViewContainers[idName]->ibView.Format = DXGI_FORMAT_R32_UINT;
        break;

    default:
        m_ViewContainers[idName]->ibView.Format = DXGI_FORMAT_R16_UINT;
        break;
    }

    return hr;
}

D3D12_VERTEX_BUFFER_VIEW* MeshManager::GetVertexBufferView(const std::wstring idName)
{
	return &m_ViewContainers[idName]->vbView;
}

D3D12_INDEX_BUFFER_VIEW* MeshManager::GetIndexBufferView(const std::wstring idName)
{
	return &m_ViewContainers[idName]->ibView;
}

void MeshManager::ResetMesh()
{
    m_crIndex = L"";
    m_crVertex = L"";
}

void MeshManager::RemoveVertexBuffer(const std::wstring id, bool withIndex)
{
    m_VertexBuffers.erase(id);

    if (withIndex)
        RemoveIndexBuffer(id);
}

void MeshManager::RemoveIndexBuffer(const std::wstring id)
{
    m_IndexBuffers.erase(id);

    m_ViewContainers.erase(id);
}
