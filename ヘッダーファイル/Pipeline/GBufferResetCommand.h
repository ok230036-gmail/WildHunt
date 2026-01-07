#pragma once
#include "PipeLineManager.h"

class GBufferResetCommand : public GraphicsPipeLineObjectBase
{
private:
	std::list<Texture2DContainer*> m_rtvTextures;
	std::list<DepthTextureContainer*> m_dsvTextures;

public:
	void SetRTVTexture(Texture2DContainer* rtvTex)
	{
		m_rtvTextures.push_back(rtvTex);
	}

	void SetDepthTexture(DepthTextureContainer* depthTex)
	{
		m_dsvTextures.push_back(depthTex);
	}

	void RemoveRTVTexture(Texture2DContainer* rtvTex)
	{
		if (!m_rtvTextures.empty())
		{
			m_rtvTextures.remove(rtvTex);
		}
	}

	void RemoveDepthTexture(DepthTextureContainer* depthTex)
	{
		if (!m_dsvTextures.empty())
		{
			m_dsvTextures.remove(depthTex);
		}
	}

	void ClearRTVTextures()
	{
		m_rtvTextures.clear();
	}

	void ClearDepthTextures()
	{
		m_dsvTextures.clear();
	}

	//  GraphicsPipeLineObjectBase ÇâÓÇµÇƒåpè≥Ç≥ÇÍÇ‹ÇµÇΩ
	HRESULT CreateDescriptorHeaps() override;
	HRESULT InitPipeLineStateObject(ID3D12Device2* d3dDev) override;
	ID3D12GraphicsCommandList* ExecuteRender() override;
};

