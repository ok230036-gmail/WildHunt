#include "GBufferResetCommand.h"
#include "DXSampleHelper.h" // ThrowIfFailedなどを使うため
#include <d3dx12.h>

HRESULT GBufferResetCommand::CreateDescriptorHeaps()
{
	return S_OK;
}

HRESULT GBufferResetCommand::InitPipeLineStateObject(ID3D12Device2* d3dDev)
{
    m_cmdLists.resize(FRAME_COUNT);
    MyGameEngine* engine = MyAccessHub::GetMyGameEngine();
    for (int i = 0; i < FRAME_COUNT; i++)
    {
        // CommandAllocator分のCommandListを作る
        ID3D12CommandAllocator* cmdAL = engine->GetCommandAllocator(i);
        ThrowIfFailed(engine->GetDirect3DDevice()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
            cmdAL, m_pipeLineState.Get(), IID_PPV_ARGS(m_cmdLists[i].GetAddressOf())));
        m_cmdLists[i]->Close();
    }

    ClearRTVTextures();
    ClearDepthTextures();

    return S_OK;
}

ID3D12GraphicsCommandList* GBufferResetCommand::ExecuteRender()
{
    MyGameEngine* myEngine = MyAccessHub::GetMyGameEngine();
    MeshManager* mshMng = myEngine->GetMeshManager();

    ID3D12CommandAllocator* cmdAl = myEngine->GetCurrentCommandAllocator();

    UINT frameIndex = myEngine->GetCurrentFrameIndex();
    ID3D12GraphicsCommandList* cmdList = m_cmdLists[frameIndex].Get();

    ThrowIfFailed(cmdList->Reset(cmdAl, m_pipeLineState.Get()));

    CD3DX12_RESOURCE_BARRIER tra[1];

    for (auto texBuff : m_rtvTextures)
    {
        ID3D12Resource* nmlBuff = texBuff->m_pTexture.Get();

        // NormalバッファをRENDER TARGETに切り替え
        tra[0] = CD3DX12_RESOURCE_BARRIER::Transition(nmlBuff,
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
            D3D12_RESOURCE_STATE_RENDER_TARGET);    // D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCEはピクセルシェーダ以外で読む時用
        cmdList->ResourceBarrier(1, tra);   // セット。第一引数は設定の数。

        // Normalバッファのクリア。
        cmdList->ClearRenderTargetView(texBuff->descHeap->GetCPUDescriptorHandleForHeapStart(), Colors::Transparent, 0, nullptr);

        // バリアを戻す
        tra[0] = CD3DX12_RESOURCE_BARRIER::Transition(nmlBuff,
            D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE |
            D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
        cmdList->ResourceBarrier(1, tra);   // セット。第一引数は設定の数。
    }

    // Depthバッファクリア処理
    for (auto depthTex : m_dsvTextures)
    {
        CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(depthTex->dsvHeap->GetCPUDescriptorHandleForHeapStart());

        D3D12_RECT scissor;	// ScissorRectはテクスチャ上の描画領域設定
        scissor.left = 0;
        scissor.top = 0;
        scissor.right = depthTex->lWidth;	// Depthのサイズにあわせてシザーも変化。
        scissor.bottom = depthTex->lHeight;

        cmdList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 1, &scissor);
	}
    cmdList->Close();

    return cmdList;
}
