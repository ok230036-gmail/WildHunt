#pragma once
#include "PipeLineManager.h"
class EdgeDrawPipeline :
    public GraphicsPipeLineObjectBase
{
private:
    ComPtr<ID3D12DescriptorHeap> m_srvHeap; // NormalBuffer‚ÆDepthBuffer‚ğSRV‰»‚·‚é—pB

    // GraphicsPipeLineObjectBase ‚ğ‰î‚µ‚ÄŒp³‚³‚ê‚Ü‚µ‚½
    HRESULT CreateDescriptorHeaps() override;

public:
    HRESULT InitPipeLineStateObject(ID3D12Device2* d3dDev) override;
    ID3D12GraphicsCommandList* ExecuteRender() override;
};

