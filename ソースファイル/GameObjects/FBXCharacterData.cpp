#include <algorithm>

#include <MyAccessHub.h>
#include <MyGameEngine.h>
#include "FBXCharacterData.h"

using namespace fbxsdk;
using namespace DirectX;

void FBXCharacterData::ResetBoneMatrix(int clCount)
{
	for (int i = 0; i < clCount; i++)
	{
		memset(&m_F4X4Matrix[i], 0, sizeof(XMFLOAT4X4));

		m_F4X4Matrix[i]._11 = 1.0f;
		m_F4X4Matrix[i]._22 = 1.0f;
		m_F4X4Matrix[i]._33 = 1.0f;
		m_F4X4Matrix[i]._44 = 1.0f;
	}
}

FBXCharacterData::FBXCharacterData()
{
	AddConstantBuffer(sizeof(XMMATRIX), nullptr);	// For ModelMatrix

	// このラベル名でLightSettingManagerに登録
	m_ambientLight = L"SCENE_AMBIENT";			
	m_directionalLight = L"SCENE_DIRECTIONAL";
}

HRESULT FBXCharacterData::SetMainFBX(const std::wstring fbxId)
{
	FBXDataContainerSystem* fbxContSys = FBXDataContainerSystem::GetInstance();
	m_mainFbx = fbxContSys->GetModelFbx(fbxId);

	if (m_mainFbx == nullptr)
		return E_FAIL;

	int clCount = m_mainFbx->GetClusterCount();

	m_boneConvertIdList.clear();
	m_F4X4Matrix.clear();

	if (clCount > 0)
	{
		int curCbuff = m_cbuffCount;
		AddConstantBuffer(sizeof(XMFLOAT4X4) * clCount, nullptr);
		m_mainFbx->SetCBuffIndex(curCbuff);

		m_boneConvertIdList.resize(clCount);
		m_F4X4Matrix.resize(clCount);

		ResetBoneMatrix(clCount);
	}

	return S_OK;
}

void FBXCharacterData::SetAnime(std::wstring animeLabel)
{
	if (animeLabel != m_currentAnimeLabel)
	{
		FBXDataContainerSystem* fbxContSys = FBXDataContainerSystem::GetInstance();
		FBXDataContainer* animeCont = fbxContSys->GetAnimeFbx(animeLabel);

		if (animeCont != nullptr)
		{
			MeshContainer* meshCont = nullptr;

			m_currentAnimeLabel = animeLabel;
			m_animeTime = 0;

			animeCont->GetFbxScene()->SetCurrentAnimationStack(animeCont->GetAnimeStack());

			m_animeFbx = animeCont;

			// IDリスト更新
			int clCount = m_mainFbx->GetClusterCount();
			for (int i = 0; i < clCount; i++)
			{
				m_boneConvertIdList[i] = m_animeFbx->GetNodeId(m_mainFbx->GetBoneName(i));
			}

			ResetBoneMatrix(clCount);	// ボーン構成が異なる場合に前のアニメのデータが残っちゃうから、リセット
			UpdateAnimation(0);

			m_animeEnd = false;
			m_extraAction = false;
		}
	}
	else
	{
		int frames = m_animeFbx->GetAnimeFrames();	// 再生位置のフレーム取得
		if (m_animeTime >= frames)					// まだ終了位置を超えているなら
		{
			m_animeEnd = true;
		}
		else
		{
			// m_animeEnd = false;
		}
	}
}

void FBXCharacterData::UpdateAnimation()
{
	// いったん、全部自動ループ
	assert(m_animeFbx);
	int frames = m_animeFbx->GetAnimeFrames();
	m_animeTime++;
	if (m_animeTime >= frames)
	{
		m_animeTime -= frames;
		m_animeEnd = true;
	}
	UpdateAnimation(m_animeTime);
}

void FBXCharacterData::UpdateAnimation(int frameCount)
{
	double nowTime = m_animeFbx->GetPeriodTime() * frameCount;

	FbxTime currentTime;
	currentTime.SetSecondDouble(nowTime);

	m_currentAnimeTime = currentTime;

	// ボーンデータ更新
	m_mainFbx->GetAnimatedMatrix(currentTime, m_animeFbx->GetFbxScene(), m_boneConvertIdList, m_F4X4Matrix);

}

// 当たり判定関係
bool FBXCharacterData::MakeAABB(UINT index, HitAABB& aabb)
{
	MeshContainer* mesh = m_mainFbx->GetMeshContainer(index);// m_pMeshContainer[index].get();

	if (mesh == nullptr || mesh->m_vertexCount < 2) return false;

	aabb.SetAABBMinMax(mesh->m_vtxMin, mesh->m_vtxMax);

	return true;
}

bool FBXCharacterData::MakeSphere(UINT index, HitSphere& sphere)
{
	MeshContainer* mesh = m_mainFbx->GetMeshContainer(index);// m_pMeshContainer[index].get();

	if (mesh == nullptr || mesh->m_vertexCount < 2) return false;

	sphere.SetSphereMinMax(mesh->m_vtxMin, mesh->m_vtxMax);

	return true;
}

XMFLOAT4X4* FBXCharacterData::GetAnimatedMatrixData()
{
	return m_F4X4Matrix.data();
}

// FbxAMatrixをXMMATRIXに変換、転置はしない
void convertFbxAMatrixToXMMATRIX(const FbxAMatrix& fbxamatrix, DirectX::XMMATRIX& xmmat)
{
	xmmat = XMMATRIX(
		static_cast<float>(fbxamatrix[0][0]), static_cast<float>(fbxamatrix[0][1]), static_cast<float>(fbxamatrix[0][2]), static_cast<float>(fbxamatrix[0][3]),
		static_cast<float>(fbxamatrix[1][0]), static_cast<float>(fbxamatrix[1][1]), static_cast<float>(fbxamatrix[1][2]), static_cast<float>(fbxamatrix[1][3]),
		static_cast<float>(fbxamatrix[2][0]), static_cast<float>(fbxamatrix[2][1]), static_cast<float>(fbxamatrix[2][2]), static_cast<float>(fbxamatrix[2][3]),
		static_cast<float>(fbxamatrix[3][0]), static_cast<float>(fbxamatrix[3][1]), static_cast<float>(fbxamatrix[3][2]), static_cast<float>(fbxamatrix[3][3])
	);

	// xmmat = XMMatrixTranspose(xmmat);	// 転置
}

// BoneNameから現在のアニメデータを取得
XMMATRIX FBXCharacterData::GetAnimatedMatrixByBoneName(const char* boneName)
{
	FbxNode* node;
	FbxScene* animeScene = m_animeFbx->GetFbxScene();

	vector<const char*> boneNameList = m_animeFbx->GetBoneNameList();

	int len = boneNameList.size();

	for (int i = 0; i < len; i++)
	{
		if (strcmp(boneName, boneNameList[i]) == 0)
		{
			node = animeScene->GetNode(i);		// ボーンのIDからFbxNodeを取得
			FbxAMatrix matrix = node->EvaluateGlobalTransform(m_currentAnimeTime);

			XMMATRIX xmat;
			convertFbxAMatrixToXMMATRIX(matrix, xmat);

			return xmat;
			break;
		}
	}
}