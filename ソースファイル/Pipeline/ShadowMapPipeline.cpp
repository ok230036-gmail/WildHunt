#include "ShadowMapPipeline.h"

#include "FBXCharacterData.h"	    // レンダリング時に必要
#include "LightSettingManager.h"    // ライト方向が欲しい
#include "WildHuntScene.h"          // カメラが欲しい
#include "CameraComponent.h"        // カメラ位置と焦点位置が欲しい

#include <DXSampleHelper.h>
#include <d3dx12.h>

HRESULT ShadowMapPipeline::CreateDescriptorHeaps()
{
    return S_OK;    // Descriptor Heapなし
}

HRESULT ShadowMapPipeline::InitPipeLineStateObject(ID3D12Device2* d3dDev)
{
    D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};

    // バージョンテスト。
    featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

    if (FAILED(d3dDev->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
    {
        featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;    // 失敗した場合は最大バージョンを1.0に
    }

    struct
    {
        byte* data;
        uint32_t size;
    } vertexShader, pixelShader;

    // このPipelineはStatic用とSkeltal用の2つを同時に持つので注意
    CD3DX12_ROOT_PARAMETER1 rootParameters[4] = {};

    // リソース用テクスチャは不要
    rootParameters[0].InitAsConstantBufferView(3, 0,
        D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_VERTEX);   // Bone
    rootParameters[1].InitAsConstantBufferView(2, 0,
        D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_VERTEX);   // World
    rootParameters[2].InitAsConstantBufferView(0, 0,
        D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_VERTEX);   // View
    rootParameters[3].InitAsConstantBufferView(1, 0,
        D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_VERTEX);   // Projection

    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
    ComPtr<ID3DBlob> signature;
    ComPtr<ID3DBlob> error;

    // スタティックメッシュのインプットレイアウト。入力はそのままのメッシュデータを取り込むのでこうなる。
    D3D12_INPUT_ELEMENT_DESC staticLayout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 36, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXTURE", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };

    // スケルタルメッシュのインプットレイアウト
    D3D12_INPUT_ELEMENT_DESC skeltalLayout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 36, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXTURE", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },

        { "BLENDINDICES", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, D3D12_APPEND_ALIGNED_ELEMENT,
        D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "BLENDINDICES", 1, DXGI_FORMAT_R32G32B32A32_UINT, 0, D3D12_APPEND_ALIGNED_ELEMENT,
        D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "BLENDWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT,
        D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "BLENDWEIGHT", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT,
        D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };

    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 0, nullptr,
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
    ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion,
        signature.GetAddressOf(), error.GetAddressOf()));
    ThrowIfFailed(d3dDev->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(),
        IID_PPV_ARGS(m_rootSignature.GetAddressOf())));

    if (m_staticMesh)
    {
        // 頂点シェーダのみ スタティックメッシュ用
        ReadDataFromFile(L"Resources/shaders/StaticMeshShadowMapVS.cso", &vertexShader.data, &vertexShader.size);
        psoDesc.InputLayout = { staticLayout, _countof(staticLayout) };
        psoDesc.pRootSignature = m_rootSignature.Get();
        psoDesc.VS.BytecodeLength = vertexShader.size;
        psoDesc.PS.BytecodeLength = 0;			// ピクセルシェーダの容量0
        psoDesc.VS.pShaderBytecode = vertexShader.data;
        psoDesc.PS.pShaderBytecode = nullptr;		// バイナリもnullptr
    }
    else
    {
        // 頂点シェーダのみ　こちらはスケルタルメッシュ用
        ReadDataFromFile(L"Resources/shaders/SkeltalMeshShadowMapVS.cso", &vertexShader.data, &vertexShader.size);

        //  Describe and create the graphics pipeline state object (PSO).
        psoDesc.InputLayout = { skeltalLayout, _countof(skeltalLayout) };
        psoDesc.pRootSignature = m_rootSignature.Get();
        psoDesc.VS.BytecodeLength = vertexShader.size;
        psoDesc.PS.BytecodeLength = 0;
        psoDesc.VS.pShaderBytecode = vertexShader.data;
        psoDesc.PS.pShaderBytecode = nullptr;
    }

    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);//  CW front; cull back
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);         //  Opaque

    for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
        psoDesc.BlendState.RenderTarget[i].BlendEnable = FALSE;     // BlendState全てOFF

    psoDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 0;	                // Render Targetがない。
    psoDesc.RTVFormats[0] = DXGI_FORMAT_UNKNOWN;    // RenderTargetのFormatもUNKNOWN
    psoDesc.SampleDesc.Count = 1;                   // マルチサンプリング適用数。１だと「使わない」

    //  Depth Stencilの設定
    psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;  // この設定が実際の深度バッファとずれていると描画で落ちる。
    psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
    psoDesc.DepthStencilState.DepthEnable = TRUE;
    psoDesc.DepthStencilState.StencilEnable = FALSE;     //  OFF
    // ステンシルがないのでここまででOK。

    ThrowIfFailed(d3dDev->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(m_pipeLineState.GetAddressOf())));

    MyGameEngine* engine = MyAccessHub::GetMyGameEngine();

	// 定数バッファ作成 ライトのViewとProjection用
    engine->CreateConstantBuffer(m_cbLightViewMtx.GetAddressOf(), nullptr, sizeof(XMMATRIX));
    engine->CreateConstantBuffer(m_cbLightProjMtx.GetAddressOf(), nullptr, sizeof(XMMATRIX));

    // コマンドリスト用vector枠確保
    m_cmdLists.resize(FRAME_COUNT);
    for (int i = 0; i < FRAME_COUNT; i++)
    {
        // Create the command list.
        ID3D12CommandAllocator* cmdAL = engine->GetCommandAllocator(i);
        ThrowIfFailed(engine->GetDirect3DDevice()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
            cmdAL, m_pipeLineState.Get(), IID_PPV_ARGS(m_cmdLists[i].GetAddressOf())));
        m_cmdLists[i]->Close();

        NAME_D3D12_OBJECT_INDEXED(m_cmdLists, i);
    }

    engine->WaitForGpu();

    return S_OK;
}

ID3D12GraphicsCommandList* ShadowMapPipeline::ExecuteRender()
{
    // 描画対象が無い場合は、ここで終了
    if (m_renderList.size() < 1) return nullptr;

    // ViewPort & Scissor Rect
	D3D12_VIEWPORT viewport = {};
	D3D12_RECT scissorRect = {};

    MyGameEngine* myEngine = MyAccessHub::GetMyGameEngine();
    MeshManager* mshMng = myEngine->GetMeshManager();

    // 光源データを取得したいのでシーンが欲しい。
    WildHuntScene* scene = static_cast<WildHuntScene*>(myEngine->GetSceneController());

    // コマンドラインの取得とリセット
    ID3D12CommandAllocator* cmdAl = myEngine->GetCurrentCommandAllocator();
    UINT frameIndex = myEngine->GetCurrentFrameIndex();
    ID3D12GraphicsCommandList* cmdList = m_cmdLists[frameIndex].Get();
    ThrowIfFailed(cmdList->Reset(cmdAl, m_pipeLineState.Get()));

    for (auto depthTex : m_depthTextures)
    {
        // DepthStecilViewの設定
        CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(depthTex->dsvHeap->GetCPUDescriptorHandleForHeapStart());

        // RTV,DSVの設定。RenderTargetは無し。今回のポイント。
        cmdList->OMSetRenderTargets(0, nullptr, FALSE, &dsvHandle);

        D3D12_VIEWPORT viewport = {};
        viewport.TopLeftX = 0;
        viewport.TopLeftY = 0;
        viewport.Width = depthTex->lWidth;
        viewport.Height = depthTex->lHeight;
        viewport.MinDepth = 0;
        viewport.MaxDepth = 1.0f;

        D3D12_RECT scissorRect = {};
        scissorRect.left = 0;
        scissorRect.top = 0;
        scissorRect.right = depthTex->lWidth;
        scissorRect.bottom = depthTex->lHeight;

        // ViewPort ScissorRect共に専用
        cmdList->RSSetViewports(1, &viewport);
        cmdList->RSSetScissorRects(1, &scissorRect);

        cmdList->SetGraphicsRootSignature(m_rootSignature.Get());               // RootSignatureセット
        cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);   // トポロジセット

        // Lightの位置をカメラとしてマトリクスセット
        LightSettingManager* pLightMng = LightSettingManager::GetInstance();

        auto dLight = pLightMng->GetDirectionalLight(L"SCENE_DIRECTIONAL");	// ライトデータ取得
        assert(dLight);	// 取れなかったら止める

		XMMATRIX trLVMtx = XMMatrixTranspose(dLight->GetLightViewMtx());
        XMMATRIX trLPMtx = XMMatrixTranspose(dLight->GetLightProjectionMtx());
        myEngine->UpdateShaderResourceOnGPU(m_cbLightViewMtx.Get(), &trLVMtx, sizeof(XMMATRIX));
        myEngine->UpdateShaderResourceOnGPU(m_cbLightProjMtx.Get(), &trLPMtx, sizeof(XMMATRIX));

        cmdList->SetGraphicsRootConstantBufferView(2, m_cbLightViewMtx->GetGPUVirtualAddress()); // View
        cmdList->SetGraphicsRootConstantBufferView(3, m_cbLightProjMtx->GetGPUVirtualAddress()); // Projection

        for (auto charaData : m_renderList)
        {
            FBXCharacterData* p_fbxChara = static_cast<FBXCharacterData*>(charaData);

            // FbxCharacterDataのメソッドでmainFbx取り出し
            FBXDataContainer* mainFbx = p_fbxChara->GetMainFbx();

            // Worldマトリクスのセット、0が該当のマトリクス用Resource
            ID3D12Resource* p_WorldMtx = p_fbxChara->GetConstantBuffer(0);

            MeshContainer* mesh = nullptr;

            if (!m_staticMesh)
            {
                // スキン有り ボーンデータ
                ID3D12Resource* resource = p_fbxChara->GetConstantBuffer(mainFbx->GetCBuffIndex());
                cmdList->SetGraphicsRootConstantBufferView(0, resource->GetGPUVirtualAddress());    // Skin
            }
            cmdList->SetGraphicsRootConstantBufferView(1, p_WorldMtx->GetGPUVirtualAddress());      // World

            // MainFbxに格納された分割メッシュを全てレンダリング
            for (int meshIndex = 0; (mesh = mainFbx->GetMeshContainer(meshIndex)) != nullptr; meshIndex++)
            {
                // 頂点データ・インデックスデータ設定。
                mshMng->SetVertexBuffer(cmdList, mesh->m_MeshId);
                mshMng->SetIndexBuffer(cmdList, mesh->m_MeshId);

                //  描画
                cmdList->DrawIndexedInstanced((UINT)mesh->m_indexData.size(), 1, 0, 0, 0);
            }
        }
    }

    m_renderList.clear();
    cmdList->Close();

    return cmdList;
}
