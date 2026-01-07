#include "SceneController.h"
#include <MyAccessHub.h>

void SceneController::CheckSceneOrder()
{
	if (m_scene != m_orderSceneID)
	{
		MyGameEngine* engine = MyAccessHub::GetMyGameEngine();
		
		int frameId = engine->GetCurrentFrameIndex();
		
		frameId = --frameId < 0 ? 1 : frameId;	// Swap後の処理なので前フレームのIndex。
		
		engine->WaitForGpu(frameId);			// リセット待機。Releaseだとぶっ飛ばすので落ちる。
		ID3D12CommandAllocator* cmdAlloc = engine->GetCommandAllocator(frameId);
		cmdAlloc->Reset();						// 前フレームのCommandAllocatorをリセット。
		engine->GetPipelineManager()->ResetPipeline(); // SRV本体は消さなくても大丈夫。

		if (SUCCEEDED(ChangeGameScene(m_orderSceneID)))
		{
			m_scene = m_orderSceneID;
			engine->WaitForGpu();				// リセット待機。Releaseだとぶっ飛ばすので落ちる。
		}
	}
}
