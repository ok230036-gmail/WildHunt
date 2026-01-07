#pragma once
#include "PipeLineManager.h"
class ShadowMapPipeline :
    public GraphicsPipeLineObjectBase
{
private:
    bool m_staticMesh = false;                  // 頂点レイアウトとシェーダが変わるので切り替えフラグ
    ComPtr<ID3D12Resource> m_cbLightViewMtx;    // コンスタントバッファにアップする為のリソース
    ComPtr<ID3D12Resource> m_cbLightProjMtx;    // コンスタントバッファにアップする為のリソース
	std::list<DepthTextureContainer*> m_depthTextures; // シャドウマップ用デプステクスチャリスト

    //  GraphicsPipeLineObjectBase を介して継承されました
    HRESULT CreateDescriptorHeaps() override;

public:
    void SetStaticMeshMode(bool flg)
    {
        m_staticMesh = flg;
    }

    void SetDepthTexture(DepthTextureContainer* depthTex)
    {
        m_depthTextures.push_back(depthTex);
	}

    void RemoveDepthTexture(DepthTextureContainer* depthTex)
    {
        if (!m_depthTextures.empty())
        {
            m_depthTextures.remove(depthTex);
        }
	}

    HRESULT InitPipeLineStateObject(ID3D12Device2* d3dDev) override;
    ID3D12GraphicsCommandList* ExecuteRender() override;
};

