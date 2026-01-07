#include "EdgeDrawPipeline.h"
#include "DXSampleHelper.h" // ThrowIfFailed等
#include <d3dx12.h>
#include <MyAccessHub.h>

HRESULT EdgeDrawPipeline::CreateDescriptorHeaps()
{
    MyGameEngine* engine = MyAccessHub::GetMyGameEngine();
    ID3D12Device* d3dDev = engine->GetDirect3DDevice();

    // シェーダリソース用のヒープを確保
    D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc = {};
    cbvHeapDesc.NumDescriptors = 2;                 // 最大テクスチャ数
    cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    HRESULT hr = d3dDev->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(m_srvHeap.ReleaseAndGetAddressOf()));
    if (FAILED(hr)) return hr;

    TextureManager* texMng = engine->GetTextureManager();
    texMng->CreateTextureSRV(d3dDev, m_srvHeap.Get(), m_numOfTex, L"NormalBuffer");
    m_srvTexList[L"NormalBuffer"] = m_numOfTex;     // 利用テクスチャリストに登録
    m_numOfTex++;


    // DepthBufferのSRVを作成する。
    D3D12_CPU_DESCRIPTOR_HANDLE heapHandle;

    ID3D12Resource* depthBuff = engine->GetDepthStencil();
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;     // 今回Depth別管理していなかったので決め打ちで
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;

    UINT offset = d3dDev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    size_t startAddr = m_srvHeap->GetCPUDescriptorHandleForHeapStart().ptr;
    heapHandle.ptr = startAddr + offset * m_numOfTex;
    d3dDev->CreateShaderResourceView(depthBuff, &srvDesc, heapHandle);
    m_srvTexList[L"DepthBuffer"] = m_numOfTex;              // 利用テクスチャリストに登録
    m_numOfTex++;

    return hr;
}

HRESULT EdgeDrawPipeline::InitPipeLineStateObject(ID3D12Device2* d3dDev)
{
    HRESULT hr = E_FAIL;
    D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};

    featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
    if (FAILED(d3dDev->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
    {
        featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;    // 失敗した場合は最大バージョンを1.0に
    }

    // POST EFFECTで一回だけの物なので、定数バッファなし。
    CD3DX12_DESCRIPTOR_RANGE1 ranges[2] = {};
    CD3DX12_ROOT_PARAMETER1 rootParameters[2] = {};
    ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_NONE); // Texture
    ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1, 0, D3D12_DESCRIPTOR_RANGE_FLAG_NONE); // Texture
    rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_ALL);
    rootParameters[1].InitAsDescriptorTable(1, &ranges[1], D3D12_SHADER_VISIBILITY_PIXEL);

    // SamplerStateの設定。RootSignatureにここらへんの設定が組み込まれる。
    D3D12_STATIC_SAMPLER_DESC sampler = {};
    sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;    // テクスチャの端で値が大きく変わると変な線が出るのでMIRROR
    sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
    sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
    sampler.MipLODBias = 0;
    sampler.MaxAnisotropy = 1;
    sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
    sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
    sampler.MinLOD = 0.0f;
    sampler.MaxLOD = D3D12_FLOAT32_MAX;
    sampler.ShaderRegister = 0;
    sampler.RegisterSpace = 0;
    sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;	// ANISOTROPICでも良いけど重いので

    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
    rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 1, &sampler,
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    ComPtr<ID3DBlob> signature;
    ComPtr<ID3DBlob> error;
    ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion,
        signature.GetAddressOf(), error.GetAddressOf()));
    ThrowIfFailed(d3dDev->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(),
        IID_PPV_ARGS(m_rootSignature.GetAddressOf())));

    struct
    {
        byte* data;
        uint32_t size;
    } meshShader, pixelShader;

    // シェーダファイル名
    ReadDataFromFile(L"Resources/shaders/PEEdgeDrawVS.cso", &meshShader.data, &meshShader.size);
    ReadDataFromFile(L"Resources/shaders/PEEdgeDrawPS.cso", &pixelShader.data, &pixelShader.size);

    //  Define the vertex input layout.
    D3D12_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };

    //  Describe and create the graphics pipeline state object (PSO).
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout = { layout, _countof(layout) };
    psoDesc.pRootSignature = m_rootSignature.Get();

    psoDesc.VS.BytecodeLength = meshShader.size;
    psoDesc.PS.BytecodeLength = pixelShader.size;
    psoDesc.VS.pShaderBytecode = meshShader.data;
    psoDesc.PS.pShaderBytecode = pixelShader.data;
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);//  CW front; cull back
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);          //  Opaque

    D3D12_RENDER_TARGET_BLEND_DESC defaultRenderTargetBlendDesc = psoDesc.BlendState.RenderTarget[0];
    defaultRenderTargetBlendDesc.BlendEnable = FALSE;
    defaultRenderTargetBlendDesc.SrcBlend = D3D12_BLEND_ZERO;       // SRC:ZERO DEST;SRC_COLOR　これは乗算合成設定
    defaultRenderTargetBlendDesc.DestBlend = D3D12_BLEND_SRC_COLOR;
    defaultRenderTargetBlendDesc.BlendOp = D3D12_BLEND_OP_ADD;
    defaultRenderTargetBlendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
    defaultRenderTargetBlendDesc.DestBlendAlpha = D3D12_BLEND_ZERO;
    defaultRenderTargetBlendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
    defaultRenderTargetBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

    for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
        psoDesc.BlendState.RenderTarget[i] = defaultRenderTargetBlendDesc;

    psoDesc.BlendState.RenderTarget[0].BlendEnable = true;	// MRTではない。
    psoDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.SampleDesc.Count = 1;

    ThrowIfFailed(d3dDev->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(m_pipeLineState.GetAddressOf())));

    // コマンドリスト用vector枠確保
    MyGameEngine* engine = MyAccessHub::GetMyGameEngine();
    m_cmdLists.resize(FRAME_COUNT);
    for (int i = 0; i < FRAME_COUNT; i++)
    {
        //  Create the command list.
        ID3D12CommandAllocator* cmdAL = engine->GetCommandAllocator(i);
        ThrowIfFailed(engine->GetDirect3DDevice()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
            cmdAL, m_pipeLineState.Get(), IID_PPV_ARGS(m_cmdLists[i].GetAddressOf())));
        m_cmdLists[i]->Close();
    }

    engine->WaitForGpu();

    hr = CreateDescriptorHeaps();

    return hr;
}

ID3D12GraphicsCommandList* EdgeDrawPipeline::ExecuteRender()
{
    // 描画対象が無いのでここで終了
    if (m_renderList.size() < 1) return nullptr;

    MyGameEngine* myEngine = MyAccessHub::GetMyGameEngine();
    MeshManager* mshMng = myEngine->GetMeshManager();

    ID3D12CommandAllocator* cmdAl = myEngine->GetCurrentCommandAllocator();

    UINT frameIndex = myEngine->GetCurrentFrameIndex();

    ID3D12GraphicsCommandList* cmdList = m_cmdLists[frameIndex].Get();
    ThrowIfFailed(cmdList->Reset(cmdAl, m_pipeLineState.Get()));

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(myEngine->GetRTVHeap()->GetCPUDescriptorHandleForHeapStart(), frameIndex,
        myEngine->GetDirect3DDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV));

    cmdList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);     // RTVは通常の出力先
    myEngine->SetDefaultViewportAndRect(cmdList);	                // ViewPortとScissorRectはswapchain全域

    //  Indicate that the back buffer will be used as a render target.
    CD3DX12_RESOURCE_BARRIER tra[2];
    tra[0] = CD3DX12_RESOURCE_BARRIER::Transition(myEngine->GetRenderTarget(frameIndex), D3D12_RESOURCE_STATE_PRESENT,
        D3D12_RESOURCE_STATE_RENDER_TARGET);
    tra[1] = CD3DX12_RESOURCE_BARRIER::Transition(myEngine->GetDepthStencil(), D3D12_RESOURCE_STATE_DEPTH_WRITE,
        D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    cmdList->ResourceBarrier(2, tra);

    cmdList->SetGraphicsRootSignature(m_rootSignature.Get());

    ID3D12DescriptorHeap* ppHeaps[] = { m_srvHeap.Get() };
    cmdList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

    UINT stride = sizeof(SpriteVertex);
    UINT offset = 0;

    cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    cmdList->IASetVertexBuffers(0, 1, mshMng->GetVertexBufferView(L"Sprite"));
    cmdList->IASetIndexBuffer(mshMng->GetIndexBufferView(L"Sprite"));

    auto gpuHeap = m_srvHeap->GetGPUDescriptorHandleForHeapStart();
    cmdList->SetGraphicsRootDescriptorTable(0, gpuHeap);    // NormalTex
    gpuHeap.ptr += myEngine->GetDirect3DDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    cmdList->SetGraphicsRootDescriptorTable(1, gpuHeap);    // DepthTex
    
    // 描画
    cmdList->DrawIndexedInstanced(6, 1, 0, 0, 0);

    //  Indicate that the back buffer will now be used to present.
    tra[0] = CD3DX12_RESOURCE_BARRIER::Transition(myEngine->GetRenderTarget(frameIndex), D3D12_RESOURCE_STATE_RENDER_TARGET,
        D3D12_RESOURCE_STATE_PRESENT);
    tra[1] = CD3DX12_RESOURCE_BARRIER::Transition(myEngine->GetDepthStencil(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE |
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_DEPTH_WRITE);
    cmdList->ResourceBarrier(2, tra);

    m_renderList.clear();
    cmdList->Close();

    return cmdList;
}