#include <Windows.h>

#include <mmsystem.h>

// for_eachが欲しい
#include <algorithm>

#include "MyGameEngine.h"

#include "MyAccessHub.h"

#include <D3D12Helper.h>
#include <DXSampleHelper.h>

#include "d3dx12.h"

// マルチメディアタイマー
#pragma comment(lib, "winmm.lib")

// DirectX12ライブラリ
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

// FBX SDK
#pragma comment(lib, "libfbxsdk-md.lib")
#pragma comment(lib, "libxml2-md.lib")
#pragma comment(lib, "zlib-md.lib")

using namespace DirectX;

HRESULT MyGameEngine::InitMyGameEngine(HINSTANCE hInst, HWND hwnd)
{
    // AccessHubに登録
    MyAccessHub::SetMyGameEnegine(this);

    HRESULT hr = S_OK;                  // DirectXの初期化処理関数の戻り値

    RECT rc;
    GetClientRect(hwnd, &rc);           // Window全体ではなくて描画領域だけの範囲
    UINT width = rc.right - rc.left;    // 幅と高さを取得
    UINT height = rc.bottom - rc.top;

    UINT dxgiFactoryFlags = 0;          // DX12だとファクトリーからなのでファクトリーフラグに使うので、名前変更。

#ifdef _DEBUG
    // まずデバッグコントローラを起動してアクティブにして、createDeviceFlagsもあわせて変更する。
    ComPtr<ID3D12Debug> debugController;
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(debugController.GetAddressOf()))))
    {
        debugController->EnableDebugLayer();

        //  Enable additional debug layers.
        dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;

        ComPtr<ID3D12Debug1> spDebugController1;
        debugController->QueryInterface(IID_PPV_ARGS(spDebugController1.GetAddressOf()));
        spDebugController1->SetEnableGPUBasedValidation(true);
    }
#endif

    // このエンジンが対応するDirectX仕様レベルを列挙
    D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_12_2, // Shader Model 6.5 + Direct3D 12.2
        D3D_FEATURE_LEVEL_12_1, // Shader Model 5.0 + Direct3D 12.1
        D3D_FEATURE_LEVEL_12_0, // Shader Model 5.0 + Direct3D 12.0

        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
    };
    UINT numFeatureLevels = ARRAYSIZE(featureLevels);

    // まずFactoryを作る
    ComPtr<IDXGIFactory4> factory;
    ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(factory.GetAddressOf())));

    // ハードウェアアダプタを取得してDeviceを作成
    ComPtr<IDXGIAdapter1> adapter;
    ComPtr<IDXGIFactory6> factory6;

    if (SUCCEEDED(factory->QueryInterface(IID_PPV_ARGS(factory6.GetAddressOf()))))
    {
        // Factory6に対応しているのでハイスペックアダプタを探す
        for (
            UINT adapterIndex = 0;
            SUCCEEDED(factory6->EnumAdapterByGpuPreference( // ファクトリーからGPU機能の乗るアダプタを一つずつ取得
                adapterIndex,
                DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,       // HIGH_PERFORMANCEはD3D完全対応ハード用らしい
                IID_PPV_ARGS(adapter.GetAddressOf())));
            ++adapterIndex)
        {
            DXGI_ADAPTER_DESC1 desc;
            adapter->GetDesc1(&desc);

            if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)    // 一応ソフトウェア版（WARPアダプタ）はスキップ
            {
                continue;
            }

            hr = E_FAIL;

            for (UINT featureLevel = 0; featureLevel < numFeatureLevels; featureLevel++)
            {
                m_featureLevel = featureLevels[featureLevel];
                if (SUCCEEDED(hr = D3D12CreateDevice(adapter.Get(), m_featureLevel, _uuidof(ID3D12Device), &m_pd3dDevice)))
                {
                    break;
                }

            }

            if (SUCCEEDED(hr))
                break;
        }
    }

    if (m_pd3dDevice.Get() == nullptr)
    {
        // Factory6に対応出来なかったり、HIGH_PERFORMANCEで取得出来なかった（ハードがちょっと古かった）場合
        // 全アダプタで使える物を探す
        for (UINT adapterIndex = 0; SUCCEEDED(factory->EnumAdapters1(adapterIndex, &adapter)); ++adapterIndex)
        {
            DXGI_ADAPTER_DESC1 desc;
            adapter->GetDesc1(&desc);

            if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)    // 一応ソフトウェア版（WARPアダプタ）はスキップ
            {
                continue;
            }

            hr = E_FAIL;

            for (UINT featureLevel = 0; featureLevel < numFeatureLevels; featureLevel++)
            {
                m_featureLevel = featureLevels[featureLevel];
                if (SUCCEEDED(hr = D3D12CreateDevice(adapter.Get(), m_featureLevel, _uuidof(ID3D12Device), &m_pd3dDevice)))
                {
                    break;
                }

            }

            if (SUCCEEDED(hr))
                break;
        }

    }

    // deviceが出来てなかったら終了
    ThrowIfFailed(hr);

    // CommandQueueはGPUごとに一つ。
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

    ThrowIfFailed(m_pd3dDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(m_pCommandQueue.GetAddressOf())));


    ComPtr<IDXGISwapChain1> swapChain;

    //  Describe and create the swap chain.
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.BufferCount = FRAME_COUNT;
    swapChainDesc.Width = width;
    swapChainDesc.Height = height;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;   // D3D12のフレームバッファはFLIP_SEQUENTIALかFLIP_DISCARD

    // DISCARDはバックバッファをクリアするけどSEQUENTIALはバックバッファが保持される。
    swapChainDesc.SampleDesc.Count = 1;

    ThrowIfFailed(factory->CreateSwapChainForHwnd(
        m_pCommandQueue.Get(),        //  Swap chain needs the queue so that it can force a flush on it.
        hwnd,
        &swapChainDesc,
        nullptr,
        nullptr,
        &swapChain
    ));

    ThrowIfFailed(swapChain.As(&m_pSwapChain)); // SwapChain3へ。
    m_frameIndex = m_pSwapChain->GetCurrentBackBufferIndex();   // バックバッファのインデックス番号を保持

    //  Note this tutorial doesn't handle full-screen swapchains so we block the ALT+ENTER shortcut
    ThrowIfFailed(factory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER));


    //  まずはRenderTarget用のRenderTargetView Heapを作成する。
    //  Describe and create a render target view (RTV) descriptor heap.
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.NumDescriptors = FRAME_COUNT;
    rtvHeapDesc.NodeMask = 0;                               // どれ用のGPUの値か
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;      // HEAPは色々同じやり方で作るので注意。これはRenderTargetView用と言うフラグ
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;    // RTV用はフラグNONEで。
    ThrowIfFailed(m_pd3dDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(m_rtvHeap.GetAddressOf())));

    // RTVのバッファ本体を作る時に必要になるのでRTV用ヒープの連結サイズを取得しておく。
    m_rtvDescriptorSize = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    // RTVHeapにRenderTargetViewの実体を作成していく。CreateRenderTargetViewね.
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());   // まずRTVHeapのハンドルを先頭にセット。

    // バックバッファ数分ループ
    for (UINT n = 0; n < FRAME_COUNT; n++)
    {
        ThrowIfFailed(m_pSwapChain->GetBuffer(n, IID_PPV_ARGS(m_renderTargets[n].GetAddressOf())));     // SwapChainからバッファを取得して・・・
        m_pd3dDevice->CreateRenderTargetView(m_renderTargets[n].Get(), nullptr, rtvHandle);             // バッファに描画ターゲットとしてのViewを作成

        // この処理自体はD3D11もほぼ同じ。書き込み先がRTVHeapのハンドルな所に注意。
        rtvHandle.Offset(1, m_rtvDescriptorSize);       // 終わったらHeapハンドルをHeapサイズ分ずらす。
        NAME_D3D12_OBJECT_INDEXED(m_renderTargets, n);  // Debugでエラー表示する時に名前が出るように。

        // CommandAllocator の作成。実行コマンドリストである「CommandList」を作るためのメモリ領域を確保するオブジェクト。
        ThrowIfFailed(m_pd3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(m_commandAllocators[n].GetAddressOf())));
    }

    // DepthStencilView Heapの作成
    // Descriptor Heaps
    // Depth Stencil
    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};    // rtvやsrvと同じような感じ。

    dsvHeapDesc.NumDescriptors = 1;

    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

    ThrowIfFailed(m_pd3dDevice->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(m_dsvHeap.GetAddressOf())));

    //  Create the depth stencil view.
    D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
    depthOptimizedClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
    depthOptimizedClearValue.DepthStencil.Stencil = 0;

    const CD3DX12_HEAP_PROPERTIES depthStencilHeapProps(D3D12_HEAP_TYPE_DEFAULT);
    const CD3DX12_RESOURCE_DESC depthStencilTextureDesc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D24_UNORM_S8_UINT, width, height, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);
 
    ThrowIfFailed(m_pd3dDevice->CreateCommittedResource(
        &depthStencilHeapProps,
        D3D12_HEAP_FLAG_NONE,
        &depthStencilTextureDesc,
        D3D12_RESOURCE_STATE_DEPTH_WRITE,
        &depthOptimizedClearValue,
        IID_PPV_ARGS(m_pDepthStencil.GetAddressOf())
    ));

    NAME_D3D12_OBJECT(m_pDepthStencil); // Debugでエラー表示する時に名前が出るように。

    D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
    depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;

    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = m_dsvHeap->GetCPUDescriptorHandleForHeapStart();

    m_pd3dDevice->CreateDepthStencilView(m_pDepthStencil.Get(), &depthStencilDesc, dsvHandle);


    // CommandListにセットする必要があるのでメンバ化。
    m_viewport.Width = (FLOAT)width;
    m_viewport.Height = (FLOAT)height;
    m_viewport.MinDepth = 0.0f;
    m_viewport.MaxDepth = 1.0f;
    m_viewport.TopLeftX = 0;
    m_viewport.TopLeftY = 0;

    // m_scissorRectはスクリーン座標系
    m_scissorRect.top = 0;
    m_scissorRect.bottom = height;
    m_scissorRect.left = 0;
    m_scissorRect.right = width;

    // 画面初期化用の空のCommandListを作成しておく
    for (int i = 0; i < FRAME_COUNT; i++)
    {
        ThrowIfFailed(m_pd3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocators[i].Get(), nullptr, IID_PPV_ARGS(m_initCommand.GetAddressOf())));
        ThrowIfFailed(m_initCommand->Close());
    }

    // リソースバリアのFenceを作成。これでマルチスレッドでリソースのクリティカルセクションを避ける
    {
        ThrowIfFailed(m_pd3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_fence.GetAddressOf())));
        for (int i = 0; i < FRAME_COUNT; i++)
        {
            m_fenceValues[i] = 1;
        }

        // フェンスによるアップ待ちは、スレッドのロック待ち。ここはWin32のCreateEventを使う。
        m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        if (m_fenceEvent == nullptr)
        {
            ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
        }
    }

    // Comの初期化。WICTextureLoaderを使うため。
    hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (FAILED(hr))
        return hr;

    // WICTexture
    m_pTextureMng = make_unique<TextureManager>();
    hr = m_pTextureMng->InitTextureManager();
    if (FAILED(hr))
        return hr;

    m_meshMng = make_unique<MeshManager>();
    m_pipelineMng = make_unique<PipeLineManager>();

    // 輪郭線実装
    m_PrePipelineMng = make_unique<PipeLineManager>();
    m_PEPipelineMng = make_unique<PipeLineManager>();

    // WICTextureLoader使うため、Comの初期化。
    hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (FAILED(hr))
    {
        return hr;
    }

    // Sound
    m_soundMng = make_unique<SoundManager>();
    hr = m_soundMng->InitSoundManager();
    if (FAILED(hr))
    {
        DWORD r = GetLastError();
        return hr;
    }

    // タイマー初期設定
    if (!QueryPerformanceFrequency(&m_timerFreq))
    {
        return S_FALSE;
    }

    // インスタンスはシングルトン
    m_inputMng = InputManager::GetInstance();
    m_inputMng->InitInputManager(hInst, hwnd);

    if (FAILED(hr))
    {
        DWORD r = GetLastError();
        return hr;
    }

    m_hitMng = make_unique<HitManager>();

    // Scene
    // インスタンスは別作成
    hr = m_sceneCont->InitSceneController();
    if (FAILED(hr))
    {
        DWORD r = GetLastError();
        return hr;
    }

    return S_OK;
}

void MyGameEngine::SetSceneController(SceneController* pSceneCont)
{
    m_sceneCont.reset(pSceneCont);
}


HitManager* MyGameEngine::GetHitManager()
{
    return m_hitMng.get();
}

void MyGameEngine::AddGameObject(GameObject* obj)
{
    m_gameObjects.push_back(obj);
    obj->Init();                                            // 追加後に初期化メソッド呼び出し
}

void MyGameEngine::RemoveGameObject(GameObject* obj)
{
    obj->CleanupGameObject();                               // 削除前に終了メソッド呼び出し
    m_gameObjects.remove(obj);
}

void MyGameEngine::SetGameObjectToAddQueue(GameObject* obj) // 追加キューに新規オブジェクトを追加
{
    m_addGameObjects.push_back(obj);
}

void MyGameEngine::SetMainRenderTarget(ID3D12GraphicsCommandList* cmdList)
{
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize);
    CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(m_dsvHeap->GetCPUDescriptorHandleForHeapStart());

    cmdList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

    SetDefaultViewportAndRect(cmdList);
}

void MyGameEngine::SetDefaultViewportAndRect(ID3D12GraphicsCommandList* cmdList)
{
    cmdList->RSSetViewports(1, &m_viewport);
    cmdList->RSSetScissorRects(1, &m_scissorRect);
}


void MyGameEngine::CleanupDevice()
{
    // スマートポインタ化しているので個別のReleaseは不要。

    std::for_each(m_gameObjects.begin(), m_gameObjects.end(), [this](GameObject* obj)
        {
            obj->CleanupGameObject();
        }
    );

    m_gameObjects.clear();
    m_pTextureMng->ReleaseAllTextures();

    WaitForGpu();   // GPU待機

    CoUninitialize();
}


void MyGameEngine::FrameUpdate()
{

    LARGE_INTEGER   nowTimer;
    float           frame;
    const float     FRAME_TIME = 1.0f / 60.0f;
    
    QueryPerformanceCounter(&nowTimer);
    frame = static_cast<float>(nowTimer.QuadPart - m_preTimer.QuadPart) / static_cast<float>(m_timerFreq.QuadPart);

    // 1/60秒に一回、処理を行う
    if (frame >= FRAME_TIME)
    {
        list<GameObject*> deleteObjects;    // 削除リスト
        deleteObjects.clear();              // 初期化

        m_meshMng->ResetMesh();

        m_preTimer = nowTimer;              // タイマー更新。次回は現在の時間から1/60秒後
        m_inputMng->Update();

        // ヒットバッファフラッシュ
        m_hitMng->RefreshHitSystem();

        std::for_each(m_gameObjects.begin(), m_gameObjects.end(),   // gameObjectsの中身全てで
            [this, &deleteObjects](GameObject* obj) {
                if (!obj->Action()) {                               // actionメソッドを実行
                    deleteObjects.push_back(obj);                   // falseの場合はGameObjectの削除へ
                }
            }
        );

        m_hitMng->HitFrameAction();

        m_inputMng->RefreshBuffer();

        // オブジェクト削除モード
        if (!deleteObjects.empty())
        {
            std::for_each(deleteObjects.begin(), deleteObjects.end(),   // gameObjectsの中身全てで
                [this](GameObject* obj) {
                    RemoveGameObject(obj);                              // MyGameEngineのメソッドで安全に削除
                    delete(obj);                                        // メモリ解放
                }
            );

            deleteObjects.clear();                                      // 削除リストクリア
        }

        // オブジェクト追加モード
        if (!m_addGameObjects.empty())
        {
            for (auto addObj : m_addGameObjects)
            {
                AddGameObject(addObj);
            }

            // 追加リストクリア
            m_addGameObjects.clear();
        }

        // サウンド処理
        m_soundMng->AudioUpdate();

        Render();

        MoveToNextFrame();  // GPU待機　次フレーム切り替え

        m_sceneCont->CheckSceneOrder();
    }

}

void MyGameEngine::WaitForGpu()
{
    WaitForGpu(m_frameIndex);
}

void MyGameEngine::WaitForGpu(int frame)
{
    // GPU終了確認。これは初期化、または終了処理前用

    // コマンドキューにフェンスの検知シグナルを追加
    ThrowIfFailed(m_pCommandQueue->Signal(m_fence.Get(), m_fenceValues[frame]));

    // フェンス処理終了確認イベントをセット
    ThrowIfFailed(m_fence->SetEventOnCompletion(m_fenceValues[frame], m_fenceEvent));

    // イベント実行待ち 時間制限なし
    WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);

    // 該当フレームのフェンスカウント加算
    m_fenceValues[frame]++;
}

void MyGameEngine::MoveToNextFrame()
{
    // 通常フレーム時のGPU終了確認

    const UINT64 currentFenceVal = m_fenceValues[m_frameIndex];

    // コマンドキューにフェンスのシグナル追加
    ThrowIfFailed(m_pCommandQueue->Signal(m_fence.Get(), currentFenceVal));

    // フレームインデックス更新
    m_frameIndex = m_pSwapChain->GetCurrentBackBufferIndex();

    // 次のフレームが終了していない（前回のレンダリングが続いている）場合、処理を待つ
    if (m_fence->GetCompletedValue() < m_fenceValues[m_frameIndex])
    {
        ThrowIfFailed(m_fence->SetEventOnCompletion(m_fenceValues[m_frameIndex], m_fenceEvent));

        // イベント実行待ち
        WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);
    }

    // フェンスカウント更新。次フレームの値の更新なので元値が前フレーム
    m_fenceValues[m_frameIndex] = currentFenceVal + 1;

}

HRESULT MyGameEngine::CreateVertexBuffer(ID3D12Resource** pVertexBuffer, const void* initBuff, UINT vertexSize, UINT vertexCount)
{
    HRESULT hr;

    size_t totalSize = vertexSize * vertexCount;

    // 頂点バッファ
    CD3DX12_HEAP_PROPERTIES upHeapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

    CD3DX12_RESOURCE_DESC sourceDesc = CD3DX12_RESOURCE_DESC::Buffer(totalSize);

    if (SUCCEEDED( hr = m_pd3dDevice->CreateCommittedResource(
        &upHeapProp,    // CD3DX12_HEAP_PROPERTIESはD3D12のMSサンプルが用意しているラッパー構造体
        D3D12_HEAP_FLAG_NONE,
        &sourceDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(pVertexBuffer)) ) )

    {
        uint8_t* memory = nullptr;
        (*pVertexBuffer)->Map(0, nullptr, reinterpret_cast<void**>(&memory));
        std::memcpy(memory, initBuff, totalSize);
        (*pVertexBuffer)->Unmap(0, nullptr);
    }

    return hr;
}

HRESULT MyGameEngine::CreateIndexBuffer(ID3D12Resource** pIndexBuffer, const void* indexBuff, UINT valueSize, UINT indexCount)
{
    HRESULT hr;

    size_t indexSize = valueSize * indexCount;

    // インデックスバッファ
    CD3DX12_HEAP_PROPERTIES upHeapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(indexSize);

    if (SUCCEEDED(hr = m_pd3dDevice->CreateCommittedResource(
        &upHeapProp,
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(pIndexBuffer))))
    {
        uint8_t* memory = nullptr;
        (*pIndexBuffer)->Map(0, nullptr, reinterpret_cast<void**>(&memory));
        std::memcpy(memory, indexBuff, indexSize);
        (*pIndexBuffer)->Unmap(0, nullptr);
    }

    return hr;
}

HRESULT MyGameEngine::CreateConstantBuffer(ID3D12Resource** pConstBuffer, const void* initBuff, UINT buffSize)
{
    CD3DX12_HEAP_PROPERTIES upheapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

    UINT fixedSize = (buffSize + 255) & ~255;

    CD3DX12_RESOURCE_DESC sourceDesc = CD3DX12_RESOURCE_DESC::Buffer(fixedSize);

    HRESULT hr = m_pd3dDevice->CreateCommittedResource(
        &upheapProp,   // CD3DX12_HEAP_PROPERTIESはD3D12のMSサンプルが用意しているラッパー構造体
        D3D12_HEAP_FLAG_NONE,
        &sourceDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(pConstBuffer));

    if (SUCCEEDED(hr) && initBuff != nullptr)
    {
        uint8_t* memory = nullptr;
        (*pConstBuffer)->Map(0, nullptr, reinterpret_cast<void**>(&memory));
        std::memcpy(memory, initBuff, sizeof(buffSize));
        (*pConstBuffer)->Unmap(0, nullptr);
    }

    return hr;
}

void MyGameEngine::InitCameraConstantBuffer(CharacterData* chData)
{
    XMMATRIX mtr = {};

    chData->AddConstantBuffer(sizeof(XMMATRIX), &mtr);  // for View
    chData->AddConstantBuffer(sizeof(XMMATRIX), &mtr);  // for Projection
}

void MyGameEngine::UpdateCameraMatrixForComponent(float fov, XMVECTOR Eye, XMVECTOR At, XMVECTOR Up, float width, float height, float nearZ, float farZ)
{

    if (m_cameraData == nullptr) return;

    XMMATRIX view = MakeViewMatix(Eye, At, Up);
    XMMATRIX proj = MakePerspectiveProjectionMatrix(fov, width, height, nearZ, farZ);

    XMMATRIX trView = XMMatrixTranspose(view);
    XMMATRIX trProj = XMMatrixTranspose(proj);

    uint8_t* memory = nullptr;
    ID3D12Resource* resource = m_cameraData->GetConstantBuffer(CB_CAM_VIEW_INDEX);  // view
    resource->Map(0, nullptr, reinterpret_cast<void**>(&memory));
    std::memcpy(memory, &trView, sizeof(XMMATRIX));
    resource->Unmap(0, nullptr);

    resource = m_cameraData->GetConstantBuffer(CB_CAM_PROJECTION_INDEX);            // proj
    resource->Map(0, nullptr, reinterpret_cast<void**>(&memory));
    std::memcpy(memory, &trProj, sizeof(XMMATRIX));
    resource->Unmap(0, nullptr);

}

HRESULT MyGameEngine::UpdateShaderResourceOnGPU(ID3D12Resource* resource, const void* res, size_t buffSize)
{
    uint8_t* memory = nullptr;
    resource->Map(0, nullptr, reinterpret_cast<void**>(&memory));
    std::memcpy(memory, res, buffSize);
    resource->Unmap(0, nullptr);

    return S_OK;
}

HRESULT MyGameEngine::UploadCreatedTextures()
{
    m_initCommand->Reset(m_commandAllocators[m_frameIndex].Get(), nullptr);
    return m_pTextureMng->UploadCreatedTextures(m_pd3dDevice.Get(), m_initCommand.Get(), m_pCommandQueue.Get());
}

void MyGameEngine::Render()
{
    ThrowIfFailed(m_commandAllocators[m_frameIndex]->Reset()); // まずコマンドアロケーターを初期化
    ThrowIfFailed(m_initCommand->Reset(m_commandAllocators[m_frameIndex].Get(), nullptr));

    // コマンドを記録していく。
    m_initCommand->RSSetViewports(1, &m_viewport);
    m_initCommand->RSSetScissorRects(1, &m_scissorRect);

    // Indicate that the back buffer will be used as a render target.
    CD3DX12_RESOURCE_BARRIER tra = CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
    m_initCommand->ResourceBarrier(1, &tra);

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize);
    CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(m_dsvHeap->GetCPUDescriptorHandleForHeapStart());

    m_initCommand->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

    // バックバッファのクリア。これがないと前の絵が残るぞ
    m_initCommand->ClearRenderTargetView(rtvHandle, Colors::MidnightBlue, 0, nullptr);
    
    // ステンシルバッファのクリア。
    m_initCommand->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 1, &m_scissorRect);

    //  Indicate that the back buffer will now be used to present.
    tra = CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
    m_initCommand->ResourceBarrier(1, &tra);

    ThrowIfFailed(m_initCommand->Close());

    int preCount = m_PrePipelineMng->Render(true);
    if (preCount > 0)
    {
        m_pCommandQueue->ExecuteCommandLists(preCount, m_PrePipelineMng->GetCommandList());
        WaitForGpu(); //  GPU処理の結果を本番の描画に使うのでGPU待機
    }

    // 蓄積したコマンドリストをまとめて実行
    int listCount = m_pipelineMng->Render(true);
    if (listCount > 0)
        m_pCommandQueue->ExecuteCommandLists(listCount, m_pipelineMng->GetCommandList());

    listCount = m_PEPipelineMng->Render();
    if (listCount > 0)
    {
        WaitForGpu(); // 終了を待つ。
        m_pCommandQueue->ExecuteCommandLists(listCount, m_PEPipelineMng->GetCommandList());
    }

    // 描画したバックバッファと表示中のバッファを入れ替え。(SwapChain)

    ThrowIfFailed(m_pSwapChain->Present(1, 0));
}

MyGameEngine::MyGameEngine(UINT width, UINT height, std::wstring title)
{
    m_title = title;
    m_windowWidth = width;
    m_windowHeight = height;
}
