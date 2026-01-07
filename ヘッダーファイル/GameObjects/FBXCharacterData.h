#pragma once
#include <Windows.h>
#include <MyAccessHub.h>
#include <CharacterData.h>

#include <memory>
#include <vector>
#include <unordered_map>
#include <wrl/client.h>

#include <fbxsdk.h>

#include "HitShapes.h"
#include "FBXDataContainerSystem.h"

using Microsoft::WRL::ComPtr;
using namespace DirectX;

class FBXDataContainer;


class FBXCharacterData : public CharacterData
{
private:

	FBXDataContainer* m_mainFbx;

	// スキンアニメ用メンバ追加
	FBXDataContainer* m_animeFbx;

	std::wstring m_currentAnimeLabel;
	LONG	m_animeTime;

	bool m_animeEnd;
	bool m_extraAction;
	FbxTime m_currentAnimeTime;

	// アニメマトリクスをCharacterData側でもたせる。
	std::vector<XMFLOAT4X4>			m_F4X4Matrix;			// アニメーションのupdateで更新されるDirect3D用クラスタマトリクス
	std::vector<int>				m_boneConvertIdList;	// アニメ側のボーン名から見た本体側のボーンID値の配列

	// Lighting
	std::wstring m_ambientLight;		// LightSettingManagerからひっぱる環境光データのラベル
	std::wstring m_directionalLight;	// LightSettingManagerからひっぱる平行光源データのラベル

	bool m_isCastShadow = true;

	void ResetBoneMatrix(int clCount);

public:

	FBXCharacterData();

	HRESULT SetMainFBX(const std::wstring fbxId);

	FBXDataContainer* GetMainFbx()
	{
		return m_mainFbx;
	}

	// スキンアニメ用FBX読み込みとアニメ実行メソッド関係
	void SetAnime(std::wstring animeLabel);		// 再生アニメ指定
	void UpdateAnimation();						// アニメ１フレーム更新
	void UpdateAnimation(int frameCount);		// アニメフレーム指定更新

	// 当たり判定関係
	bool MakeAABB(UINT index, HitAABB& aabb);
	bool MakeSphere(UINT index, HitSphere& sphere);

	XMFLOAT4X4* GetAnimatedMatrixData();

	XMMATRIX GetAnimatedMatrixByBoneName(const char* boneName);

	// Lighting
	void SetAmbientLight(std::wstring amb)
	{
		m_ambientLight = amb;
	}

	void SetDirectionalLight(std::wstring dir)
	{
		m_directionalLight = dir;
	}

	std::wstring& GetAmbientLight()
	{
		return m_ambientLight;
	}

	std::wstring& GetDirectionalLight()
	{
		return m_directionalLight;
	}

	void SetCastShadow(bool isCast)
	{
		m_isCastShadow = isCast;
	}

	bool CheckCastShadow()
	{
		return m_isCastShadow;
	}

	bool GetAnimeEnd()
	{
		FBXDataContainerSystem* fbxContSys = FBXDataContainerSystem::GetInstance();
		FBXDataContainer* animeCont = fbxContSys->GetAnimeFbx(m_currentAnimeLabel);

		int frames = animeCont->GetAnimeFrames(); // 再生位置のフレーム取得

		if (frames != 0) // まだ終了位置を超えているなら
			return m_animeEnd;
		
		return 0;
	}

	bool CheckExtraAction(int termFrame)
	{
		FBXDataContainerSystem* fbxContSys = FBXDataContainerSystem::GetInstance();
		FBXDataContainer* animeCont = fbxContSys->GetAnimeFbx(m_currentAnimeLabel);

		int frames = animeCont->GetAnimeFrames();	// 再生位置のフレーム取得
		if (termFrame >= frames)					// まだ終了位置を超えているなら
			return true;
		

		return false;
	}
};