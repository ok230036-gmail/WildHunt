#include <MyAccessHub.h>
#include <MyGameEngine.h>

#include "FBXDataContainerSystem.h"

// FbxAMatrixをXMFLOAT4x4に
void convertFbxAMatrixToXMFLOAT4x4(const FbxAMatrix& fbxamatrix, DirectX::XMFLOAT4X4& xmfloat4x4)
{
	for (int row = 0; row < 4; row++)
	{
		for (int column = 0; column < 4; column++)
		{
			xmfloat4x4.m[row][column] = static_cast<float>(fbxamatrix[row][column]);
		}
	}
}

FBX_TEXTURE_TYPE FBXDataContainer::GetTextureType(const fbxsdk::FbxBindingTableEntry& entryTable)
{
	std::string texStr = entryTable.GetSource();

	if (texStr == "Maya|DiffuseTexture")
	{
		return FBX_TEXTURE_TYPE::FBX_DIFFUSE;
	}
	else if (texStr == "Maya|NormalTexture")
	{
		return FBX_TEXTURE_TYPE::FBX_NORMAL;
	}
	else if (texStr == "Maya|SpecularTexture")
	{
		return FBX_TEXTURE_TYPE::FBX_SPECUAR;
	}
	else if (texStr == "Maya|FalloffTexture")
	{
		return FBX_TEXTURE_TYPE::FBX_FALLOFF;
	}
	else if (texStr == "Maya|ReflectionMapTexture")
	{
		return FBX_TEXTURE_TYPE::FBX_REFLECTIONMAP;
	}

	return FBX_TEXTURE_TYPE::FBX_UNKNOWN;
}

void FinishFBXLoad(fbxsdk::FbxManager** man, fbxsdk::FbxImporter** imp, fbxsdk::FbxScene** sc)
{

	if ((*imp) != nullptr)
	{
		(*imp)->Destroy();
		*imp = nullptr;
	}

	if ((*sc) != nullptr)
	{
		(*sc)->Destroy();
		*sc = nullptr;
	}

	if ((*man) != nullptr)
	{
		(*man)->Destroy();
		*man = nullptr;
	}

}

HRESULT FBXDataContainer::ReadFbxToMeshContainer(const std::wstring id, FbxMesh* pMesh)
{
	HRESULT hr = S_OK;

	unique_ptr<MeshContainer> meshCont = make_unique<MeshContainer>();

	// ノード数
	int nodecount = pMesh->GetNodeCount();

	// MeshContainerに元データポインタ保存
	meshCont->SetFbxMesh(pMesh);

	// VertexBuffer
	// これが頂点データ本体
	FbxVector4* controllPoints = nullptr;

	// IndexBuffer
	// インデックスデータ
	int* indices = nullptr;

	// 頂点数
	int vertexCount;
	
	// コントロールポイント数
	int contCount;

	// VertexBuffer
	controllPoints = pMesh->GetControlPoints();

	// IndexBuffer
	// コントロールポイントの中からポリゴン頂点のインデックスリスト
	indices = pMesh->GetPolygonVertices();

	// 頂点数
	vertexCount = pMesh->GetPolygonVertexCount();

	// コントロールポイント数(これを使うのはスキンアニメの時)
	contCount = pMesh->GetControlPointsCount();

	meshCont->m_vertexData.clear();
	meshCont->m_vertexData.resize(vertexCount);

	// メッシュごとの当たり判定エリア値初期化
	meshCont->m_vtxMin.x = FLT_MAX;
	meshCont->m_vtxMin.y = FLT_MAX;
	meshCont->m_vtxMin.z = FLT_MAX;

	meshCont->m_vtxMax.x = FLT_MIN;
	meshCont->m_vtxMax.y = FLT_MIN;
	meshCont->m_vtxMax.z = FLT_MIN;


	for (int i = 0; i < vertexCount; i++)
	{
		// Vertexデータを初期化
		ZeroMemory(&meshCont->m_vertexData[i], sizeof(FbxVertex));

		// 頂点データをFBXから自前の頂点バッファへ 
		// インデックスバッファから頂点番号を取得
		int index = indices[i];

		// 頂点座標リストから座標を取得する
		meshCont->m_vertexData[i].position.x = (float)controllPoints[index][0];
		meshCont->m_vertexData[i].position.y = (float)controllPoints[index][1];
		meshCont->m_vertexData[i].position.z = (float)controllPoints[index][2];
		
		// max & min check
		if (meshCont->m_vertexData[i].position.x < meshCont->m_vtxMin.x)
		{
			meshCont->m_vtxMin.x = meshCont->m_vertexData[i].position.x;
		}
		else if (meshCont->m_vertexData[i].position.x > meshCont->m_vtxMax.x)
		{
			meshCont->m_vtxMax.x = meshCont->m_vertexData[i].position.x;
		}

		if (meshCont->m_vertexData[i].position.y < meshCont->m_vtxMin.y)
		{
			meshCont->m_vtxMin.y = meshCont->m_vertexData[i].position.y;
		}
		else if (meshCont->m_vertexData[i].position.y > meshCont->m_vtxMax.y)
		{
			meshCont->m_vtxMax.y = meshCont->m_vertexData[i].position.y;
		}

		if (meshCont->m_vertexData[i].position.z < meshCont->m_vtxMin.z)
		{
			meshCont->m_vtxMin.z = meshCont->m_vertexData[i].position.z;
		}
		else if (meshCont->m_vertexData[i].position.z > meshCont->m_vtxMax.z)
		{
			meshCont->m_vtxMax.z = meshCont->m_vertexData[i].position.z;
		}
	}

	// 保存したminとmaxを全体のmin、maxと比較して更新
	if (m_vtxTotalMax.x < meshCont->m_vtxMax.x) m_vtxTotalMax.x = meshCont->m_vtxMax.x;
	if (m_vtxTotalMax.y < meshCont->m_vtxMax.y) m_vtxTotalMax.y = meshCont->m_vtxMax.y;
	if (m_vtxTotalMax.z < meshCont->m_vtxMax.z) m_vtxTotalMax.z = meshCont->m_vtxMax.z;

	if (m_vtxTotalMin.x > meshCont->m_vtxMin.x) m_vtxTotalMin.x = meshCont->m_vtxMin.x;
	if (m_vtxTotalMin.y > meshCont->m_vtxMin.y) m_vtxTotalMin.y = meshCont->m_vtxMin.y;
	if (m_vtxTotalMin.z > meshCont->m_vtxMin.z) m_vtxTotalMin.z = meshCont->m_vtxMin.z;

	FbxStringList uvset_names;

	// UVSetの名前リストを取得。(マルチテクスチャの場合、UVも複数あり)
	pMesh->GetUVSetNames(uvset_names);

	FbxArray<FbxVector2> uv_buffer;

	// UVSetの名前からUVSetを取得する
	pMesh->GetPolygonVertexUVs(uvset_names.GetStringAt(0), uv_buffer);

	for (int i = 0; i < uv_buffer.Size(); i++)
	{
		FbxVector2& uv = uv_buffer[i];

		// 基本的にバッファの値をそのまま使用する
		meshCont->m_vertexData[i].uv.x = (float)uv[0];
		meshCont->m_vertexData[i].uv.y = (float)(1.0 - uv[1]);	// FBXのVはひっくり返っているので 1.0 - uv[1]
	}

	// indexの作成
	meshCont->m_indexData.clear();
	meshCont->m_indexData.resize(vertexCount);

	// ポリゴン数を取得
	int length = pMesh->GetPolygonCount();

	// ポリゴン数でループ
	for (int i = 0; i < length; i++)
	{
		int baseIndex = i * 3;	// ポリゴンカウント＊３がインデックスの先頭
		
		meshCont->m_indexData[baseIndex] = static_cast<ULONG>(baseIndex);
		meshCont->m_indexData[baseIndex + 1] = static_cast<ULONG>(baseIndex + 1);
		meshCont->m_indexData[baseIndex + 2] = static_cast<ULONG>(baseIndex + 2);
	}

	// 作成したインデックスデータでシェーダリソースを作成
	MyAccessHub::GetMyGameEngine()->GetMeshManager()->AddIndexBuffer(id, meshCont->m_indexData.data(), sizeof(ULONG), vertexCount);

	// 法線
	FbxArray<FbxVector4> normals;
	
	// 法線リストの取得
	pMesh->GetPolygonVertexNormals(normals);

	// 法線登録
	for (int i = 0; i < normals.Size(); i++)
	{
		meshCont->m_vertexData[i].normal.x = normals[i][0];
		meshCont->m_vertexData[i].normal.y = normals[i][1];
		meshCont->m_vertexData[i].normal.z = normals[i][2];

		// normal と 上方向ベクトル(0,1,0)の外積でTangentベクトルを取得する。
		XMFLOAT3 normal = meshCont->m_vertexData[i].normal;
		XMFLOAT3 tanVect = { 1.0f, 0.0f, 0.0f };	// normalが0,1,0だった場合のTangent
		
		// normalがUPベクトルではなかった場合は外積計算
		if (normal.y < 0.99f)
		{
			tanVect = { -normal.z, 0.0f, normal.x };
		}
		meshCont->m_vertexData[i].tangent = tanVect;
	}

	// 頂点カラー
	{
		//  頂点カラーデータの数を確認
		int color_count = pMesh->GetElementVertexColorCount();

		// 頂点カラーデータの取得
		FbxGeometryElementVertexColor* vertex_colors = pMesh->GetElementVertexColor(0);

		if (color_count == 0 || vertex_colors == nullptr)
		{
			// この頂点データには色設定がないので全て白で補正
			for (int i = 0; i < vertexCount; i++)
			{
				meshCont->m_vertexData[i].color.x = 1.0f;
				meshCont->m_vertexData[i].color.y = 1.0f;
				meshCont->m_vertexData[i].color.z = 1.0f;
				meshCont->m_vertexData[i].color.w = 1.0f;
			}
		}
		else
		{
			FbxLayerElement::EMappingMode mapping_mode = vertex_colors->GetMappingMode();
			FbxLayerElement::EReferenceMode reference_mode = vertex_colors->GetReferenceMode();

			// 頂点カラーはカラー設定モードで値の取り方が変わる。
			if (mapping_mode == FbxLayerElement::eByPolygonVertex)
			{
				if (reference_mode == FbxLayerElement::eIndexToDirect)
				{
					//  頂点カラーバッファ取得
					FbxLayerElementArrayTemplate<FbxColor>& colors = vertex_colors->GetDirectArray();

					//  頂点カラーインデックスバッファ取得
					FbxLayerElementArrayTemplate<int>& indeces = vertex_colors->GetIndexArray();

					for (int i = 0; i < indeces.GetCount(); i++)
					{
						int id = indeces.GetAt(i);
						FbxColor color = colors.GetAt(id);
						meshCont->m_vertexData[i].color.x = (float)color.mAlpha;
						meshCont->m_vertexData[i].color.y = (float)color.mRed;
						meshCont->m_vertexData[i].color.z = (float)color.mGreen;
						meshCont->m_vertexData[i].color.w = (float)color.mBlue;
					}
				}
			}
		}

	}

	meshCont->m_MeshId = id;

	FbxLayerElementMaterial* elMat = pMesh->GetElementMaterial(0);
	int matIndex = elMat->GetIndexArray().GetAt(0);
	FbxSurfaceMaterial* srfMat = pMesh->GetNode()->GetSrcObject<FbxSurfaceMaterial>(matIndex);
	if (srfMat != nullptr)
	{
		wchar_t namebuff[64] = {};
		auto mtname = srfMat->GetName();
		size_t conv = 0;
		mbstowcs_s(&conv, namebuff, mtname, strlen(mtname));
		meshCont->m_MaterialId = std::wstring(namebuff);
	}
	else
	{
		meshCont->m_MaterialId = L"";
	}

	// 頂点数保存
	meshCont->m_vertexCount = vertexCount;

	// ボーン、スキンの読み込み
	// まず、メッシュに設定されている「スキン（表面構造）」の数を取得
	int skinCount = pMesh->GetDeformerCount(FbxDeformer::eSkin);

	if (skinCount > 0)
	{
		// スキンアニメがあるのでMeshMatrixとBoneMatrixが必要になる。
		// 次にbone。こっちは工数でかい
		meshCont->InitSkinList(skinCount);
		std::vector< std::vector< std::pair<UINT, float> > > tempWeightVect;

		std::vector< std::vector<FbxSkinAnimeParams> >	skinWeights;
		skinWeights.resize(skinCount);

		// スキンの数だけループ
		for (int skinloop = 0; skinloop < skinCount; skinloop++)
		{
			// スキンのウェイトデータ保存配列初期化
			tempWeightVect.clear();
			tempWeightVect.resize(contCount);

			// スキンを取得
			FbxSkin* pSkin = (FbxSkin*)pMesh->GetDeformer(skinloop, FbxDeformer::eSkin);

			skinWeights[skinloop].clear();
			skinWeights[skinloop].resize(contCount);	// WeightデータはVertexではなくControllPoint用

			// スキンの中に幾つのボーン構造があるのかを取得。
			int clusterCount = pSkin->GetClusterCount();

			// クラスターの数だけループ
			for (int clusterloop = 0; clusterloop < clusterCount; clusterloop++)
			{
				FbxCluster* pCluster = pSkin->GetCluster(clusterloop);

				// ボーンのID番号を取得。getClusterId内で新しいボーンだった場合は初期状態の逆行列も保存しているぞ。
				int clIndex = GetClusterId(pCluster);

				int pointCnt = pCluster->GetControlPointIndicesCount();		// このボーンが影響するコントロールポイントの数
				int* pPointArray = pCluster->GetControlPointIndices();		// このボーンが影響するコントロールポイントのインデックス配列
				double* pPointWeights = pCluster->GetControlPointWeights();	// このボーンが影響するコントロールポイントのウェイト配列、インデックスと組。

				for (int pointloop = 0; pointloop < pointCnt; pointloop++)
				{
					int cpIndex = pPointArray[pointloop];				// コントロールポイントのインデックス
					double boneWeight = pPointWeights[pointloop];		// そのコントロールポイントに与えるウェイト値

					// コントロールポイントのインデックスごとにWeightデータを収集
					tempWeightVect[cpIndex].push_back({ clIndex, static_cast<float>(boneWeight) });
				}

			}

			// 収集したControllPointごとのWeightデータを頂点用データに成形
			for (int cntLoop = 0; cntLoop < contCount; cntLoop++)
			{
				int wCnt = tempWeightVect[cntLoop].size();
				FbxSkinAnimeParams* skinparam = &skinWeights[skinloop][cntLoop];

				// 最大の8に足らない分は0で埋めておく
				for (; wCnt < 8; wCnt++)
				{
					tempWeightVect[cntLoop].push_back({ 0, 0.0f });
				}

				skinparam->indices0 = { tempWeightVect[cntLoop][0].first, tempWeightVect[cntLoop][1].first, tempWeightVect[cntLoop][2].first, tempWeightVect[cntLoop][3].first };
				skinparam->indices1 = { tempWeightVect[cntLoop][4].first, tempWeightVect[cntLoop][5].first, tempWeightVect[cntLoop][6].first, tempWeightVect[cntLoop][7].first };
				skinparam->weight0 = { tempWeightVect[cntLoop][0].second, tempWeightVect[cntLoop][1].second, tempWeightVect[cntLoop][2].second, tempWeightVect[cntLoop][3].second };
				skinparam->weight1 = { tempWeightVect[cntLoop][4].second, tempWeightVect[cntLoop][5].second, tempWeightVect[cntLoop][6].second, tempWeightVect[cntLoop][7].second };
			}
		}

		// 結合したvertexデータを作成
		std::vector<FbxSkinAnimeVertex> skinVertex;
		skinVertex.clear();
		skinVertex.resize(vertexCount);

		for (int i = 0; i < vertexCount; i++)
		{
			int index = indices[i];								// インデックスから頂点番号を取り出す
			skinVertex[i].vertex = meshCont->m_vertexData[i];
			skinVertex[i].skinvalues = skinWeights[0][index];	// シェーダ側の対応スキンは１つだけ
		}

		MyAccessHub::GetMyGameEngine()->GetMeshManager()->AddVertexBuffer(id, skinVertex.data(), sizeof(FbxSkinAnimeVertex), vertexCount);
	}
	else
	{
		// スキンメッシュなし
		if (pMesh->GetScene()->GetSrcObjectCount<FbxMesh>() > 1)
		{
			// ボーンはないけど、メッシュの基底ノードをボーンとして使う
			std::vector<FbxSkinAnimeVertex> skinVertex;
			skinVertex.clear();
			skinVertex.resize(vertexCount);

			UINT boneId = GetClusterId(pMesh->GetNode());

			for (int i = 0; i < vertexCount; i++)
			{
				skinVertex[i].vertex = meshCont->m_vertexData[i];
				skinVertex[i].skinvalues.indices0 = { boneId, 0, 0, 0 };	// 基底NodeのIDをセットして・・・
				skinVertex[i].skinvalues.indices1 = { 0, 0, 0, 0 };
				skinVertex[i].skinvalues.weight0 = { 1.0f, 0, 0, 0 };		// Weight（影響度）を1.0f（100%)にする。
				skinVertex[i].skinvalues.weight1 = { 0, 0, 0, 0 };
			}
			MyAccessHub::GetMyGameEngine()->GetMeshManager()->AddVertexBuffer(id, skinVertex.data(), sizeof(FbxSkinAnimeVertex), vertexCount);
		}
		else
		{
			// シンプルな頂点シェーダ
			MyAccessHub::GetMyGameEngine()->GetMeshManager()->AddVertexBuffer(id, meshCont->m_vertexData.data(), sizeof(FbxVertex), vertexCount);
		}
	}

	
	// MeshContainerオブジェクトをm_pMeshContainerに追加。
	m_pMeshContainer.push_back(move(meshCont));

	return hr;
}

HRESULT FBXDataContainer::LoadMaterial(const std::wstring id, FbxSurfaceMaterial* material)
{
	HRESULT hr = S_OK;

	enum class MaterialOrder
	{
		Ambient,
		Diffuse,
		Specular,
		MaxOrder,
	};

	const fbxsdk::FbxImplementation* implementation = GetImplementation(material, FBXSDK_IMPLEMENTATION_CGFX);

	wchar_t namebuff[64] = {};
	auto mtname = material->GetName();
	size_t conv = 0;
	mbstowcs_s(&conv, namebuff, mtname, strlen(mtname));
	std::wstring matName = namebuff;
	m_pMaterialContainer[matName] = make_unique<MaterialContainer>();

	FbxDouble3 colors[(int)MaterialOrder::MaxOrder];
	FbxDouble factors[(int)MaterialOrder::MaxOrder];
	FbxProperty fbxProp = material->FindProperty(FbxSurfaceMaterial::sAmbient);
	if (material->GetClassId().Is(FbxSurfaceLambert::ClassId))
	{
		const char* element_check_list[] =
		{
			FbxSurfaceMaterial::sAmbient,
			FbxSurfaceMaterial::sDiffuse,

			FbxSurfaceMaterial::sSpecular,
		};

		const char* factor_check_list[] =
		{
			FbxSurfaceMaterial::sAmbientFactor,
			FbxSurfaceMaterial::sDiffuseFactor,

			FbxSurfaceMaterial::sSpecularFactor,
		};

		// スペキュラを追加したので3に
		for (int i = 0; i < 3; i++)
		{
			fbxProp = material->FindProperty(element_check_list[i]);
			if (fbxProp.IsValid())
			{
				colors[i] = fbxProp.Get<FbxDouble3>();

			}
			else
			{
				colors[i] = FbxDouble3(1.0, 1.0, 1.0);
			}

			fbxProp = material->FindProperty(factor_check_list[i]);
			if (fbxProp.IsValid())
			{
				factors[i] = fbxProp.Get<FbxDouble>();
			}
			else
			{
				factors[i] = 1.0;
			}
		}
	}
	else
	{
		for (int i = 0; i < 3; i++)
		{
			colors[i] = FbxDouble3(1.0, 1.0, 1.0);
			factors[i] = 1.0;
		}
	}

	FbxDouble3 color = colors[(int)MaterialOrder::Ambient];
	FbxDouble factor = factors[(int)MaterialOrder::Ambient];
	m_pMaterialContainer[matName]->SetAmbient((float)color[0], (float)color[1], (float)color[2], (float)factor);

	color = colors[(int)MaterialOrder::Diffuse];
	factor = factors[(int)MaterialOrder::Diffuse];
	m_pMaterialContainer[matName]->SetDiffuse((float)color[0], (float)color[1], (float)color[2], (float)factor);

	color = colors[(int)MaterialOrder::Specular];
	factor = factors[(int)MaterialOrder::Specular];
	m_pMaterialContainer[matName]->SetSpecular((float)color[0], (float)color[1], (float)color[2], (float)factor);

	//  テクスチャ読み込み
	if (implementation == nullptr)
	{
		fbxProp = material->FindProperty(FbxSurfaceMaterial::sDiffuse);
		LoadTextureFromMaterial(matName, id, FBX_TEXTURE_TYPE::FBX_DIFFUSE, &fbxProp);
	}
	else
	{
		const FbxBindingTable* rootTable = implementation->GetRootTable();
		size_t entryCount = rootTable->GetEntryCount();

		for (int ent = 0; ent < entryCount; ent++)
		{
			const FbxBindingTableEntry entry = rootTable->GetEntry(ent);

			fbxProp = material->FindPropertyHierarchical(entry.GetSource());
			if (!fbxProp.IsValid())
			{
				fbxProp = material->RootProperty.FindHierarchical(entry.GetSource());
			}

			LoadTextureFromMaterial(matName, id, GetTextureType(entry), &fbxProp);
		}
	}

	m_pMaterialContainer[matName]->UpdateD3DResource();

	return hr;
}

HRESULT FBXDataContainer::LoadTextureFromMaterial(const std::wstring matName, const std::wstring id, FBX_TEXTURE_TYPE texType, const fbxsdk::FbxProperty* fbxProp)
{
	HRESULT hr = S_OK;
	MyGameEngine* engine = MyAccessHub::GetMyGameEngine();

	fbxsdk::FbxFileTexture* texture = nullptr;
	std::string keyword;
	int numOfTex = fbxProp->GetSrcObjectCount<FbxFileTexture>();
	if (numOfTex > 0)
	{
		numOfTex = 1;
	}
	else
	{
		numOfTex = fbxProp->GetSrcObjectCount<FbxLayeredTexture>();
	}

	TextureManager* texMng = MyAccessHub::GetMyGameEngine()->GetTextureManager();
	for (int i = 0; i < numOfTex; i++)
	{
		texture = fbxProp->GetSrcObject<FbxFileTexture>(i);
		if (texture != nullptr)
		{
			std::string filePath = texture->GetRelativeFileName();

			//  ファイルパス分解
			//  まず区切り文字を/に統一
			std::string::size_type position(filePath.find('\\'));
			std::string fileName;

			while (position != std::string::npos)
			{
				filePath.replace(position, 1, "/");
				position = filePath.find('\\', position + 1);
			}

			//  最後の/を見つけて「ファイル名」だけを取り出す。
			std::string::size_type offset = std::string::size_type(0);
			position = filePath.find('/');

			while (position != std::string::npos)
			{
				offset = position + 1;
				position = filePath.find('/', offset);
			}

			fileName = filePath.substr(offset);

			// FBXに埋め込まれているテクスチャファイル名はPSDが多いから、PSDはTGAに名前を変更する。
			position = fileName.find(".psd");
			if (position != std::string::npos)
			{
				// 最後の.psdを取得
				while (position != std::string::npos)
				{
					offset = position;
					position = fileName.find(".psd", position + 4);
				}
				fileName.replace(offset, 4, ".tga");
			}

			// fileNameをワイド文字列に変換
			wchar_t namebuff[64] = {};
			wchar_t texturePath[128];
			size_t conv = 0;
			mbstowcs_s(&conv, namebuff, fileName.c_str(), fileName.length());

			// ゲームシステムとしてのパスに変換 ID名/テクスチャファイル名になる。
			wsprintfW(texturePath, L"Resources/textures/%ls/%ls", id.c_str(), namebuff);

			// テクスチャの登録IDはid_ファイル名
			wchar_t idName[128];
			wsprintfW(idName, L"%ls_%ls", id.c_str(), namebuff);
			hr = texMng->CreateTextureFromFile(engine->GetDirect3DDevice(), idName, texturePath);

			if (FAILED(hr))
			{
				break;
			}

			switch (texType)
			{
			case FBX_TEXTURE_TYPE::FBX_DIFFUSE:
				m_pMaterialContainer[matName]->m_diffuseTextures.push_back(idName);
				break;

			case FBX_TEXTURE_TYPE::FBX_NORMAL:
				m_pMaterialContainer[matName]->m_normalTextures.push_back(idName);
				break;

			case FBX_TEXTURE_TYPE::FBX_SPECUAR:
				m_pMaterialContainer[matName]->m_specularTextures.push_back(idName);
				break;

			case FBX_TEXTURE_TYPE::FBX_FALLOFF:
				m_pMaterialContainer[matName]->m_falloffTextures.push_back(idName);
				break;

			case FBX_TEXTURE_TYPE::FBX_REFLECTIONMAP:
				m_pMaterialContainer[matName]->m_reflectionMapTextures.push_back(idName);
				break;

			default:
				break;
			}

			m_pMaterialContainer[matName]->materialInfo.TextureFlag |= (unsigned int)texType;
		}
	}

	return hr;
}

HRESULT FBXDataContainer::LoadFBX(const std::wstring fileName, const std::wstring id, bool noMaterial = false)
{
	HRESULT hr = S_OK;

	fbxsdk::FbxManager* fbx_manager = nullptr;
	FbxScene* fbx_scene = nullptr;
	FbxImporter* fbx_importer = nullptr;
	FbxNode* fbx_node = nullptr;

	char* c_filename = nullptr;
	int wcSize = sizeof(wchar_t) * wcslen(fileName.c_str()) + 1;

	c_filename = new char[wcSize];
	size_t retVal = 0;
	wcstombs_s(&retVal, c_filename, wcSize, fileName.c_str(), wcSize);

	fbx_manager = FBXDataContainer::GetFbxManager();

	// FbxImporterの作成
	fbx_importer = FbxImporter::Create(fbx_manager, c_filename);
	if (fbx_importer == nullptr)
	{
		hr = E_FAIL;
		FinishFBXLoad(&fbx_manager, &fbx_importer, &fbx_scene);
		return hr;
	}

	// FbxSceneの生成
	fbx_scene = FbxScene::Create(fbx_manager, c_filename);
	if (fbx_scene == nullptr)
	{
		hr = E_FAIL;
		FinishFBXLoad(&fbx_manager, &fbx_importer, &fbx_scene);
		return hr;
	}

	// Importerのファイル読み込み準備
	bool res = fbx_importer->Initialize(c_filename);
	delete[] c_filename;

	if (res == false)
	{
		// 初期化失敗
		hr = E_FAIL;
		FinishFBXLoad(&fbx_manager, &fbx_importer, &fbx_scene);
		return hr;
	}

	// sceneにインポート
	if (fbx_importer->Import(fbx_scene) == false)
	{
		// インポート失敗
		hr = E_FAIL;
		FinishFBXLoad(&fbx_manager, &fbx_importer, &fbx_scene);
		return hr;
	}

	FbxAxisSystem dx = FbxAxisSystem::DirectX;
	if (dx != fbx_scene->GetGlobalSettings().GetAxisSystem())
	{
		dx.DeepConvertScene(fbx_scene);
	}

	// 頂点にあるFbxNodeを取得。
	fbx_node = fbx_scene->GetRootNode();

	if (fbx_node != nullptr)	// ROOTがnullptrの時は読み取り失敗。もしくは中身が空なのでそのまま終了。
	{
		// ノード検索用リスト作成
		int nodes = fbx_scene->GetNodeCount();
		m_nodeNameList.clear();
		m_nodeNameList.resize(nodes);
		for (int i = 0; i < nodes; i++)
		{
			m_nodeNameList[i] = fbx_scene->GetNode(i)->GetName();
		}

		// FBXのモデルデータを三角形ポリゴンデータに変換
		FbxGeometryConverter converter(fbx_manager);

		converter.SplitMeshesPerMaterial(fbx_scene, true);	// マテリアルごとにメッシュを分解
		converter.Triangulate(fbx_scene, true);				// FBXを全部三角形だけに変換。

		// アニメデータチェック
		if (fbx_importer->GetAnimStackCount() > 0)
		{
			auto stack = fbx_scene->GetCurrentAnimationStack();

			m_startTime = stack->GetLocalTimeSpan().GetStart().GetSecondDouble();
			m_endTime = stack->GetLocalTimeSpan().GetStop().GetSecondDouble();

			m_timePeriod = 1.0 / 60.0;	// 16.66666ms
			m_animeFrames = floorl((m_endTime - m_startTime) / m_timePeriod);

			m_animeStack = stack;
			fbx_scene->SetCurrentAnimationStack(m_animeStack);
		}
		else
		{
			m_animeStack = nullptr;
			m_animeFrames = 0;
		}

		// 総数を取得して、ループ。
		m_pMeshContainer.clear();		// まずはMeshContainerをクリア

		int materialCnt;
		if (noMaterial)
		{
			m_pFbxScene = fbx_scene;
			return S_OK;
		}
		else
		{
			materialCnt = fbx_scene->GetMaterialCount();
		}

		for (int i = 0; i < materialCnt; i++)
		{
			LoadMaterial(id, fbx_scene->GetSrcObject<FbxSurfaceMaterial>(i));
		}

		// メッシュ読み込み処理　マテリアルごとに分解しているので元データよりも数が増えている可能性あり
		int meshCount = fbx_scene->GetSrcObjectCount<FbxMesh>();

		m_vtxTotalMax.x = FLT_MIN;
		m_vtxTotalMax.y = FLT_MIN;
		m_vtxTotalMax.z = FLT_MIN;

		m_vtxTotalMin.x = FLT_MAX;
		m_vtxTotalMin.y = FLT_MAX;
		m_vtxTotalMin.z = FLT_MAX;

		wchar_t idName[128];
		for (int i = 0; i < meshCount; i++)
		{
			wsprintfW(idName, L"%ls_%02d", id.c_str(), i);	// メッシュごとに名前をつける。元ファイル名+番号。
			if (FAILED(ReadFbxToMeshContainer(idName, fbx_scene->GetSrcObject<FbxMesh>(i))))
			{
				hr = E_FAIL;

				FinishFBXLoad(&fbx_manager, &fbx_importer, &fbx_scene);
				return hr;
			}
		}

		// clusterの数からボーン用マトリクスのコンスタントバッファ用CPU側メモリを確保
		m_clusterCount = m_boneNameList.size();

		m_pFbxScene = fbx_scene;
	}

	// FbxImporter削除
	fbx_importer->Destroy();

	return hr;
}

int FBXDataContainer::GetNodeId(const char* nodeName)
{
	int length = m_nodeNameList.size();
	for (int i = 0; i < length; i++)
	{
		if (strcmp(m_nodeNameList[i], nodeName) == 0)
		{
			return i;
		}
	}

	return -1;
}

int FBXDataContainer::GetMeshId(const char* meshName)
{
	int len = m_pMeshContainer.size();

	for (int i = 0; i < len; i++)
	{
		if (strcmp(m_pMeshContainer[i]->GetMeshNodeName(), meshName) == 0)
		{
			return i;
		}
	}

	return -1;
}

const char* FBXDataContainer::GetBoneName(int id)
{
	if (id < m_clusterCount)
	{
		return m_boneNameList[id];
	}
	return nullptr;
}

int FBXDataContainer::GetBoneId(const char* boneName)
{

	int len = m_boneNameList.size();

	for (int i = 0; i < len; i++)
	{
		if (strcmp(m_boneNameList[i], boneName) == 0)
		{
			return i;
		}
	}


	return -1;
}

FbxNode* FBXDataContainer::GetMeshNode(int id)
{
	if (id < m_pMeshContainer.size())
	{
		return m_pMeshContainer[id]->GetFbxMesh()->GetNode();
	}
	return nullptr;
}

MeshContainer::~MeshContainer()
{
	m_vertexData.clear();
	m_indexData.clear();

	// スキンアニメ用データ削除
	m_skinCount = 0;

	auto meshMng = MyAccessHub::GetMyGameEngine()->GetMeshManager();
	meshMng->RemoveVertexBuffer(m_MeshId, true);
}

void MeshContainer::SetFbxMesh(FbxMesh* mesh)
{
	m_mesh = mesh;
	m_meshNodeName = m_mesh->GetNode(0)->GetName();

	// スキンアニメモーションリセット
	FbxTime timeZero;
	timeZero.SetFrame(0, FbxTime::EMode::eFrames60);

	m_meshNodeName = m_mesh->GetNode(0)->GetName();
	m_IBaseMatrix = m_mesh->GetNode(0)->EvaluateGlobalTransform(timeZero).Inverse();
}

void MeshContainer::InitSkinList(int skinCount)
{
	m_skinCount = skinCount;
}

MaterialContainer::~MaterialContainer()
{
	if (m_uniqueTextures)
	{
		TextureManager* texMng = MyAccessHub::GetMyGameEngine()->GetTextureManager();
		for (auto id : m_diffuseTextures)
		{
			texMng->ReleaseTexture(id);
		}

		for (auto id : m_normalTextures)
		{
			texMng->ReleaseTexture(id);
		}

		for (auto id : m_specularTextures)
		{
			texMng->ReleaseTexture(id);
		}

		for (auto id : m_falloffTextures)
		{
			texMng->ReleaseTexture(id);
		}

		for (auto id : m_reflectionMapTextures)
		{
			texMng->ReleaseTexture(id);
		}

	}

	m_diffuseTextures.clear();
	m_normalTextures.clear();
	m_specularTextures.clear();
	m_falloffTextures.clear();
	m_reflectionMapTextures.clear();
}

void MaterialContainer::UpdateD3DResource()
{
	MyGameEngine* engine = MyAccessHub::GetMyGameEngine();
	engine->UpdateShaderResourceOnGPU(m_d3dresource.Get(), &materialInfo, sizeof(FbxMaterialInfo));
}

int FBXDataContainer::GetClusterId(FbxCluster* pCluster)
{
	return GetClusterId(pCluster->GetLink());
}

int FBXDataContainer::GetClusterId(FbxNode* pNode)
{
	int size = m_boneNameList.size();
	const char* nodeName = pNode->GetName();

	for (int id = 0; id < size; id++)
	{
		if (strcmp(nodeName, m_boneNameList[id]) == 0)
			return id;
	}

	m_boneNameList.push_back(nodeName);

	// 逆行列も保存
	m_IboneMatrix.push_back(pNode->EvaluateGlobalTransform().Inverse());

	return size;
}

void FBXDataContainer::GetAnimatedMatrix(const FbxTime& animeTime, FbxScene* animeScene, std::vector<int>& idList, std::vector<XMFLOAT4X4>& resMtxArray)
{
	FbxNode* node;
	int size = idList.size();

	for (int i = 0; i < size; i++)
	{
		int boneId = idList[i];
		if (boneId < 0)
		{
			continue;
		}
		
		node = animeScene->GetNode(boneId);

		FbxAMatrix matrix = node->EvaluateGlobalTransform(animeTime) * m_IboneMatrix[i];

		convertFbxAMatrixToXMFLOAT4x4(matrix, resMtxArray[i]);
	}

}

HRESULT FBXDataContainerSystem::LoadModelFBX(const std::wstring fileName, const std::wstring id)
{

	std::unique_ptr<FBXDataContainer> un_fbx = make_unique<FBXDataContainer>();
	HRESULT hr = un_fbx->LoadFBX(fileName, id);
	if (SUCCEEDED(hr))
	{
		if (m_modelFbxMap[id] != nullptr)
			m_modelFbxMap[id].reset();

		m_modelFbxMap[id] = std::move(un_fbx);
	}

	return hr;
}

FBXDataContainer* FBXDataContainerSystem::GetModelFbx(const std::wstring id)
{
	if (m_modelFbxMap.find(id) == m_modelFbxMap.end())
		return nullptr;

	return m_modelFbxMap[id].get();
}

HRESULT FBXDataContainerSystem::LoadAnimationFBX(const std::wstring fileName, const std::wstring id)
{
	std::unique_ptr<FBXDataContainer> un_fbx = make_unique<FBXDataContainer>();
	HRESULT hr = un_fbx->LoadFBX(fileName, id, true);
	if (SUCCEEDED(hr))
	{
		if (m_animeFbxMap[id] != nullptr)
			m_animeFbxMap[id].reset();

		m_animeFbxMap[id] = std::move(un_fbx);
	}

	return hr;
}

FBXDataContainer* FBXDataContainerSystem::GetAnimeFbx(const std::wstring id)
{
	if (m_animeFbxMap.find(id) == m_animeFbxMap.end())
		return nullptr;

	return m_animeFbxMap[id].get();
}

void FBXDataContainerSystem::ClearModelFBX()
{
	m_modelFbxMap.clear();
}

void FBXDataContainerSystem::ClearAnimeFBX()
{
	m_animeFbxMap.clear();
}

void FBXDataContainerSystem::DeleteModelFBX(std::wstring id)
{
	if (m_modelFbxMap.find(id) != m_modelFbxMap.end())
		m_modelFbxMap.erase(id);
}

void FBXDataContainerSystem::DeleteAnimeFBX(std::wstring id)
{
	if (m_animeFbxMap.find(id) != m_animeFbxMap.end())
		m_animeFbxMap.erase(id);
}
