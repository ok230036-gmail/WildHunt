#include "PipeLineManager.h"
#include "DXSampleHelper.h" // ThrowIfFailed等
#include "d3dx12.h"

#include <MyAccessHub.h>

void PipeLineManager::SetPipeLineOrder(const std::wstring labelName, int orderNo)
{
    auto targetIte = m_pipeLineDB.find(labelName);
    if (targetIte == m_pipeLineDB.end())
        return;

    auto targetObj = targetIte->second.get();

    auto ite = m_pipelineOrder.begin();

    for (; ite != m_pipelineOrder.end(); ite++)
    {
        if (*ite == targetObj)
        {
            break;
        }
    }

    if (ite != m_pipelineOrder.end())
    {
        // 現状の移動させるイテレータを削除
        m_pipelineOrder.erase(ite);
    }

    if (orderNo >= m_pipelineOrder.size())
    {
        m_pipelineOrder.push_back(targetObj);
    }
    else
    {
        int index = 0;
        for (ite = m_pipelineOrder.begin(); ite != m_pipelineOrder.end(); ite++)
        {
            if (index < orderNo)
                index++;
            else
                break;
        }
        
        m_pipelineOrder.insert(ite, targetObj);
    }
}

int PipeLineManager::GetPipeLineOrder(const std::wstring labelName)
{
    int index = 0;
    bool find = false;

    auto targetIte = m_pipeLineDB.find(labelName);
    if (targetIte == m_pipeLineDB.end())
        return -1;

    auto targetObj = targetIte->second.get();

    for (auto ite = m_pipelineOrder.begin(); ite != m_pipelineOrder.end(); ite++)
    {
        if (*ite == targetObj)
        {
            break;
        }
        index++;
    }

    if (find)
        return index;
    
    return -1;
}

void PipeLineManager::AddPipeLineObject(const std::wstring labelName, GraphicsPipeLineObjectBase* obj)
{
    if (SUCCEEDED(obj->InitPipeLineStateObject(MyAccessHub::GetMyGameEngine()->GetDirect3DDevice())))
    {
        if (m_pipeLineDB.find(labelName) != m_pipeLineDB.end())
        {
            int index;
            int length;
            
            auto ite = m_pipelineOrder.begin();
            bool find = false;

            for (;ite != m_pipelineOrder.end(); ite++)
            {
                if (*ite == obj)
                {
                    find = true;
                    break;
                }
            }

            m_pipelineOrder.erase(ite);
        }
        m_pipeLineDB[labelName].reset(obj);
        m_pipelineOrder.push_back(obj);
    }
}

UINT PipeLineManager::Render(bool initSet)
{
    MyGameEngine* engine = MyAccessHub::GetMyGameEngine();
    UINT renderCount = 0;

    m_renderCommandList.clear();
    if (initSet)
    {
        m_renderCommandList.push_back(engine->GetInitCommandList());
        renderCount++;
    }

    // 登録してあるPipeLineを全て実行
    for (auto const& ppObj : m_pipelineOrder)
    {
        ID3D12GraphicsCommandList* resultCmd = ppObj->ExecuteRender();
        if (resultCmd != nullptr)
        {
            m_renderCommandList.push_back(resultCmd);
            renderCount++;
        }

        ppObj->ClearRenderList();
    }

    return renderCount;
}

void PipeLineManager::ResetPipeline()
{
    for (const auto& pair : m_pipeLineDB)
    {
        pair.second->ResetTextureList();
    }
}

void GraphicsPipeLineObjectBase::ResetTextureList()
{
    m_srvTexList.clear();
    m_numOfTex = 0;

    CreateDescriptorHeaps();
}
