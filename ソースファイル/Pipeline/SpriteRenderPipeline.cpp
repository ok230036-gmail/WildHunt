#include "SpriteRenderPipeline.h"
#include "DXSampleHelper.h" // ThrowIfFailed等
#include "d3dx12.h"

#include "SpriteCharacter.h"

#include <MyAccessHub.h>
#include "WildHuntScene.h"


HRESULT SpriteRenderPipeline::InitPipeLineStateObject(ID3D12Device2* d3dDev)
{
    HRESULT hr = E_FAIL;

    D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};

    // バージョンテスト
    featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

    if (FAILED(d3dDev->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData)))) // 1.1をサポートしているかチェック
    {
        featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;    // 失敗した場合は最大バージョンを1.0に
    }

    // RootSignatureはCommandQueueごとに作成
    CD3DX12_DESCRIPTOR_RANGE1 ranges[1];
    CD3DX12_ROOT_PARAMETER1 rootParameters[8];

    ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC); // Texture

    rootParameters[0].InitAsConstantBufferView(2, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_VERTEX);
    rootParameters[1].InitAsConstantBufferView(3, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_VERTEX);
    rootParameters[2].InitAsConstantBufferView(4, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_VERTEX);
    rootParameters[3].InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_PIXEL);
    rootParameters[4].InitAsConstantBufferView(1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_PIXEL);
    rootParameters[5].InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_VERTEX);
    rootParameters[6].InitAsConstantBufferView(1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_VERTEX);
    rootParameters[7].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);
    // rootParameters[8].InitAsDescriptorTable(1, &ranges[3], D3D12_SHADER_VISIBILITY_PIXEL);

    // SamplerStateの設定
    D3D12_STATIC_SAMPLER_DESC sampler = {};
    sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    sampler.MipLODBias = 0;
    sampler.MaxAnisotropy = 0;
    sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
    sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
    sampler.MinLOD = 0.0f;
    sampler.MaxLOD = D3D12_FLOAT32_MAX;
    sampler.ShaderRegister = 0;
    sampler.RegisterSpace = 0;
    sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

    switch (samplerMode)
    {
    case 0: // No Filter
        sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
        break;

    default:
        sampler.Filter = D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;
        break;
    }

    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
    rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 1, &sampler, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    ComPtr<ID3DBlob> signature;
    ComPtr<ID3DBlob> error;
    ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, signature.GetAddressOf(), error.GetAddressOf()));
    ThrowIfFailed(d3dDev->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(m_rootSignature.GetAddressOf())));

    struct
    {
        byte* data;
        uint32_t size;
    } meshShader, pixelShader;

    ReadDataFromFile(L"Resources/shaders/SpriteVertexShader.cso", &meshShader.data, &meshShader.size);
    ReadDataFromFile(L"Resources/shaders/ColorMixPixelShader.cso", &pixelShader.data, &pixelShader.size);

    // Define the vertex input layout.
    D3D12_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };

    // Describe and create the graphics pipeline state object (PSO).
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout = { layout, _countof(layout) };
    psoDesc.pRootSignature = m_rootSignature.Get();

    // cso (Compiled Shader Object) 読み込み
    psoDesc.VS.BytecodeLength = meshShader.size;
    psoDesc.PS.BytecodeLength = pixelShader.size;
    psoDesc.VS.pShaderBytecode = meshShader.data;
    psoDesc.PS.pShaderBytecode = pixelShader.data;

    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);   // CW front; cull back

    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);             // Opaque

    D3D12_RENDER_TARGET_BLEND_DESC defaultRenderTargetBlendDesc = psoDesc.BlendState.RenderTarget[0];
    defaultRenderTargetBlendDesc.BlendEnable = FALSE;

    switch (blendMode)
    {
    case 0:
        break;

    case 1: // AlphaBlend
        defaultRenderTargetBlendDesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
        defaultRenderTargetBlendDesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
        defaultRenderTargetBlendDesc.BlendOp = D3D12_BLEND_OP_ADD;
        defaultRenderTargetBlendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
        defaultRenderTargetBlendDesc.DestBlendAlpha = D3D12_BLEND_ZERO;
        defaultRenderTargetBlendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
        defaultRenderTargetBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
        break;

    default:
        break;
    }

    for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
        psoDesc.BlendState.RenderTarget[i] = defaultRenderTargetBlendDesc;

    psoDesc.BlendState.RenderTarget[0].BlendEnable = true;


    //  CD3DX12_BLEND_DESCはd3dx12.hにあるヘルパー

    psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
    psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
    psoDesc.DepthStencilState.DepthEnable = TRUE;
    psoDesc.DepthStencilState.StencilEnable = TRUE;
    psoDesc.DepthStencilState.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
    psoDesc.DepthStencilState.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
    const D3D12_DEPTH_STENCILOP_DESC defaultStencilOp =
    { D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_COMPARISON_FUNC_ALWAYS };
    psoDesc.DepthStencilState.FrontFace = defaultStencilOp;
    psoDesc.DepthStencilState.BackFace = defaultStencilOp;

    psoDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.SampleDesc.Count = 1;

    ThrowIfFailed(d3dDev->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(m_pipeLineState.GetAddressOf())));

    UINT frameOffset = FRAME_COUNT * MAX_SPRITES;
    MyGameEngine* engine = MyAccessHub::GetMyGameEngine();

    UINT fixedSize = (sizeof(XMFLOAT4) + 255) & ~255;

    // UV
    engine->CreateConstantBuffer(m_pUvBuffer.GetAddressOf(), nullptr, fixedSize * frameOffset);
    NAME_D3D12_OBJECT(m_pUvBuffer);     // Debugでエラー表示する時に名前が出るように。

    // Size
    engine->CreateConstantBuffer(m_pSizeBuffer.GetAddressOf(), nullptr, fixedSize * frameOffset);
    NAME_D3D12_OBJECT(m_pSizeBuffer);   // Debugでエラー表示する時に名前が出るように。

    // Color Mix
    engine->CreateConstantBuffer(m_pColorBuffer.GetAddressOf(), nullptr, fixedSize * frameOffset);

    NAME_D3D12_OBJECT(m_pColorBuffer);  // Debugでエラー表示する時に名前が出るように。

    fixedSize = (sizeof(MixSettings) + 255) & ~255;
    // Mix Mode
    engine->CreateConstantBuffer(m_pMixBuffer.GetAddressOf(), nullptr, fixedSize * frameOffset);

    NAME_D3D12_OBJECT(m_pMixBuffer);    // Debugでエラー表示する時に名前が出るように。

    fixedSize = (sizeof(XMMATRIX) + 255) & ~255;

    // Model Matrix(Affine Transform)
    engine->CreateConstantBuffer(m_pAffineMatrix.GetAddressOf(), nullptr, fixedSize * frameOffset);

    NAME_D3D12_OBJECT(m_pAffineMatrix); // Debugでエラー表示する時に名前が出るように。

    // コマンドリスト用vector枠確保
    {
        m_cmdLists.resize(FRAME_COUNT);
        MyGameEngine* engine = MyAccessHub::GetMyGameEngine();
        for (int i = 0; i < FRAME_COUNT; i++)
        {
            // Create the command list.
            ID3D12CommandAllocator* cmdAL = engine->GetCommandAllocator(i);
            ThrowIfFailed(engine->GetDirect3DDevice()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, cmdAL, m_pipeLineState.Get(), IID_PPV_ARGS(m_cmdLists[i].GetAddressOf())));
            m_cmdLists[i]->Close();
        }
        engine->WaitForGpu();
    }
    hr = CreateDescriptorHeaps();

    return hr;
}

ID3D12GraphicsCommandList* SpriteRenderPipeline::ExecuteRender()
{
    MyGameEngine* myEngine = MyAccessHub::GetMyGameEngine();
    MeshManager* mshMng = myEngine->GetMeshManager();

    ID3D12CommandAllocator* cmdAl = myEngine->GetCurrentCommandAllocator();

    WildHuntScene* scene = static_cast<WildHuntScene*>(myEngine->GetSceneController());
    std::wstring cameraLabel = L""; // カメラ未設定状態

    UINT frameIndex = myEngine->GetCurrentFrameIndex();

    // 描画対象が無い場合は、終了
    if (m_renderList.size() < 1) return nullptr;

    ID3D12GraphicsCommandList* cmdList = m_cmdLists[frameIndex].Get();

    ThrowIfFailed(cmdList->Reset(cmdAl, m_pipeLineState.Get()));

    myEngine->SetMainRenderTarget(cmdList);

    // Indicate that the back buffer will be used as a render target.
    CD3DX12_RESOURCE_BARRIER tra = CD3DX12_RESOURCE_BARRIER::Transition(myEngine->GetRenderTarget(frameIndex), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
    cmdList->ResourceBarrier(1, &tra);

    cmdList->SetGraphicsRootSignature(m_rootSignature.Get());

    // SetDescriptorHeaps作成
    ID3D12DescriptorHeap* ppHeaps[] = { m_srvHeap.Get() };
    cmdList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

    UINT stride = sizeof(SpriteVertex);
    UINT offset = 0;

    cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    cmdList->IASetVertexBuffers(0, 1, mshMng->GetVertexBufferView(L"Sprite"));
    cmdList->IASetIndexBuffer(mshMng->GetIndexBufferView(L"Sprite"));

    TextureManager* texMng = myEngine->GetTextureManager();

    int drawCount = 0;
    int frmIndex = myEngine->GetCurrentFrameIndex() * MAX_SPRITES;

    const wchar_t* lastTexId = nullptr;
    int textureIndex = -1;

    // 256bytes allignment
    UINT affinBuffSize = (sizeof(XMMATRIX) + 255) & ~255;
    D3D12_GPU_VIRTUAL_ADDRESS affineAddr = m_pAffineMatrix->GetGPUVirtualAddress() + affinBuffSize * frmIndex;

    UINT f4BuffSize = (sizeof(XMFLOAT4) + 255) & ~255;
    D3D12_GPU_VIRTUAL_ADDRESS uvAddr = m_pUvBuffer->GetGPUVirtualAddress() + f4BuffSize * frmIndex;
    D3D12_GPU_VIRTUAL_ADDRESS colorAddr = m_pColorBuffer->GetGPUVirtualAddress() + f4BuffSize * frmIndex;

    UINT f2BuffSize = (sizeof(XMFLOAT2) + 255) & ~255;
    D3D12_GPU_VIRTUAL_ADDRESS sizeAddr = m_pSizeBuffer->GetGPUVirtualAddress() + f2BuffSize * frmIndex;

    UINT mixBuffSize = (sizeof(MixSettings) + 255) & ~255;
    D3D12_GPU_VIRTUAL_ADDRESS mixAddr = m_pMixBuffer->GetGPUVirtualAddress() + mixBuffSize * frmIndex;

    for (auto charaData : m_renderList)
    {
        SpriteCharacter* p_sprite = static_cast<SpriteCharacter*>(charaData);

        if (p_sprite->GetCameraLabel() != cameraLabel)
        {
            auto comp = scene->GetCameraComponent(p_sprite->GetCameraLabel());
            if (comp != nullptr)
            {
                cameraLabel = p_sprite->GetCameraLabel();
                CharacterData* camChar = comp->GetGameObject()->GetCharacterData();
                ID3D12Resource* p_viewMtx = camChar->GetConstantBuffer(0);
                ID3D12Resource* p_prjMtx = camChar->GetConstantBuffer(1);
                cmdList->SetGraphicsRootConstantBufferView(5, p_viewMtx->GetGPUVirtualAddress());// View　ここはDESCのアドレス値だけ。中身はまだ。
                cmdList->SetGraphicsRootConstantBufferView(6, p_prjMtx->GetGPUVirtualAddress());// Projection　ここはDESCのアドレス値だけ。中身はまだ。
            }
        }

        // 定数バッファのGPU上先頭アドレスをセット。
        cmdList->SetGraphicsRootConstantBufferView(0, affineAddr + affinBuffSize * drawCount);
        cmdList->SetGraphicsRootConstantBufferView(1, uvAddr + f4BuffSize * drawCount);
        cmdList->SetGraphicsRootConstantBufferView(2, sizeAddr + f2BuffSize * drawCount);
        cmdList->SetGraphicsRootConstantBufferView(3, colorAddr + f4BuffSize * drawCount);
        cmdList->SetGraphicsRootConstantBufferView(4, mixAddr + mixBuffSize * drawCount);

        const XMFLOAT4* p_rect = p_sprite->GetTextureUV();

        // マップ用メモリに書き込み
        m_aUv[drawCount] = *p_rect;

        const XMUINT2* p_size = p_sprite->GetSpriteSize();

        // マップ用メモリに書き込み
        m_aSize[drawCount].x = (float)p_size->x;
        m_aSize[drawCount].y = (float)p_size->y;

        // Color Mix
        const XMFLOAT4* p_color = p_sprite->GetColor();

        // マップ用メモリに書き込み
        m_aColor[drawCount] = *p_color;
        m_aMix[drawCount] = { p_sprite->GetColorMix(), p_sprite->GetAlphaMix() };

        XMMATRIX AffineMat = XMMatrixTranspose(p_sprite->GetWorldMatrix()); // 転置
        
        // マップ用メモリに書き込み
        m_aAffin[drawCount] = AffineMat;

        const wchar_t* texId = p_sprite->GetTextureId();
        if (lastTexId != texId)
        {
            if (m_srvTexList.find(texId) == m_srvTexList.end())
            {
                texMng->CreateTextureSRV(myEngine->GetDirect3DDevice(), m_srvHeap.Get(), 7 + m_numOfTex, texId);
                m_srvTexList[texId] = m_numOfTex;
                m_numOfTex++;
            }

            lastTexId = texId;
            textureIndex = m_srvTexList[lastTexId];
        }

        auto gpuHeap = m_srvHeap->GetGPUDescriptorHandleForHeapStart();
        gpuHeap.ptr += myEngine->GetDirect3DDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * (7 + textureIndex);
        CD3DX12_GPU_DESCRIPTOR_HANDLE cbvSrvGpuHandle(gpuHeap);
        cmdList->SetGraphicsRootDescriptorTable(7, cbvSrvGpuHandle);    // Texture　ここはDESCのアドレス値だけ。中身はまだ。

        // 描画
        cmdList->DrawIndexedInstanced(6, 1, 0, 0, 0);

        drawCount++;
    }


    // Indicate that the back buffer will now be used to present.
    tra = CD3DX12_RESOURCE_BARRIER::Transition(myEngine->GetRenderTarget(frameIndex), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
    cmdList->ResourceBarrier(1, &tra);

    m_renderList.clear();
    cmdList->Close();

    // Mapはここで一括
    {
        // Size
        UINT8* size_mapBuff;
        ThrowIfFailed(m_pSizeBuffer->Map(0, nullptr, reinterpret_cast<void**>(&size_mapBuff)));

        // UV
        UINT8* uv_mapBuff;
        ThrowIfFailed(m_pUvBuffer->Map(0, nullptr, reinterpret_cast<void**>(&uv_mapBuff)));

        // Color
        UINT8* color_mapBuff;
        ThrowIfFailed(m_pColorBuffer->Map(0, nullptr, reinterpret_cast<void**>(&color_mapBuff)));

        // Mix
        UINT8* mix_mapBuff;
        ThrowIfFailed(m_pMixBuffer->Map(0, nullptr, reinterpret_cast<void**>(&mix_mapBuff)));

        // Affin
        UINT8* affin_mapBuff;
        ThrowIfFailed(m_pAffineMatrix->Map(0, nullptr, reinterpret_cast<void**>(&affin_mapBuff)));

        for (int dc = 0; dc < drawCount; dc++)
        {
            memcpy(size_mapBuff + (frmIndex + dc) * f2BuffSize, &m_aSize[dc], sizeof(XMFLOAT2));
            memcpy(uv_mapBuff + (frmIndex + dc) * f4BuffSize, &m_aUv[dc], sizeof(XMFLOAT4));
            memcpy(color_mapBuff + (frmIndex + dc) * f4BuffSize, &m_aColor[dc], sizeof(XMFLOAT4));
            memcpy(mix_mapBuff + (frmIndex + dc) * mixBuffSize, &m_aMix[dc], sizeof(MixSettings));
            memcpy(affin_mapBuff + (frmIndex + dc) * affinBuffSize, &m_aAffin[dc], sizeof(XMMATRIX));
        }

        m_pSizeBuffer->Unmap(0, nullptr);
        m_pUvBuffer->Unmap(0, nullptr);
        m_pColorBuffer->Unmap(0, nullptr);
        m_pMixBuffer->Unmap(0, nullptr);
        m_pAffineMatrix->Unmap(0, nullptr);
    }

    return cmdList;
}

HRESULT SpriteRenderPipeline::CreateDescriptorHeaps()
{
    // シェーダリソース用のヒープを確保
    D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc = {};
    cbvHeapDesc.NumDescriptors = 12; // 最大テクスチャ数
    cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    return MyAccessHub::GetMyGameEngine()->GetDirect3DDevice()->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(m_srvHeap.ReleaseAndGetAddressOf()));
}