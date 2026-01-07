#pragma once
#include "PipeLineManager.h"
#include "FBXCharacterData.h"

#include "ShadowMapPipeline.h"


//  ライト処理をいじるためのパイプライン。
typedef int (*CBuffFunction)(int parameterIndex, std::wstring& currentLabel, MyGameEngine* engine, ID3D12GraphicsCommandList* cmdList, FBXCharacterData* fbxChara);

class StandardLightingPipeline :
    public GraphicsPipeLineObjectBase
{
private:
    ComPtr<ID3D12DescriptorHeap> m_srvHeap;

    ComPtr<ID3D12DescriptorHeap> m_rtvHeap; // RenderTargetView Heap
    
    // Managerから取得するので最初はnullptr
    GraphicsPipeLineObjectBase* m_postEffect = nullptr;

    // Depth ShadowのためのShadowMapを作成するパイプライン、型固定で保持。
    ShadowMapPipeline* m_shadowMapPL = nullptr;

    UINT32  m_pipelineFlg;
    CBuffFunction m_BoneFunc;
    CBuffFunction m_AmbientFunc;
    CBuffFunction m_DirectionalFunc;

    int m_worldMtxIndex;
    int m_textureIndex;
    int m_lightIndex;

    int m_toneIndex;

    int m_shadowDepthIndex;
    ComPtr<ID3D12Resource> m_cbLightViewProjection;


    void SetTextureToCommandLine(MyGameEngine* engine, TextureManager* pTextureMng, ID3D12GraphicsCommandList* cmdList, int prmIndex, std::wstring texId);

    //  GraphicsPipeLineObjectBase を介して継承されました
    virtual HRESULT CreateDescriptorHeaps() override;

public:

    enum PIPELINE_FLAGS
    {
        SKELTAL =   0x00000001,
        Lambert =   0x00000002,
        Phong =     0x00000004,
        Blinn =     0x00000008,
        Tone =      0x0000010C, 	// BlinnとPhong用のカメラ、テクスチャ設定をあわせもつので 10C

        LIGHTING_MASK = 0x0000000E,
    };

    void SetPipelineFlags(UINT32 flg);

    //  GraphicsPipeLineObjectBase を介して継承されました
    virtual HRESULT InitPipeLineStateObject(ID3D12Device2* d3dDev) override;
    virtual ID3D12GraphicsCommandList* ExecuteRender() override;

    virtual void AddRenerObject(CharacterData* obj) override;
};
