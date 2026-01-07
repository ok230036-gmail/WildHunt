#pragma once

#include <Windows.h>
#include <MyAccessHub.h>
#include <CharacterData.h>

#include <memory>
#include <vector>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;
using namespace DirectX;

enum class SPRITE_BLEND_MODE
{
	ALPHA_BLEND = 0,
};

enum class SPRITE_FILTER_MODE
{
	NoFilter = 0,
};

class SpriteCharacter : public CharacterData
{
private:
	class SpritePattern
	{
	public:
		XMFLOAT4	uv;
		XMUINT2	size;
	};

	const wchar_t*	index_id = L"Sprite";			// Vertex, IndexバッファID。

	const wchar_t*	texture_id = nullptr;			// テクスチャID（複数のテクスチャをIDで選択できるようにしている）
	std::vector<std::unique_ptr<SpritePattern>>		m_uvArray;	// スプライトパターンのリスト

	UINT		m_patternIndex;						// 使用中のパターンID

	SPRITE_BLEND_MODE		m_blendMode;
	SPRITE_FILTER_MODE		m_filterMode;


	XMFLOAT2 m_textureSize;

	XMFLOAT4 m_color;
	UINT	m_colorMix;
	UINT	m_alphaMix;


public:
	enum class COLOR_MIX_OP
	{
		MIX_MUL = 0,
		MIX_ADD = 1,
		MIX_DEC = 2,
		MIX_SET = 3,
		MIX_NONE = 4,
	};

	SpriteCharacter()
	{
		SetPosition(0.0f, 0.0f, 0.0f);
		m_uvArray.clear();

		m_patternIndex = 0;

		m_blendMode = SPRITE_BLEND_MODE::ALPHA_BLEND;
		m_filterMode = SPRITE_FILTER_MODE::NoFilter;

		m_color = { 1, 1, 1, 1 };
		m_textureSize = { 0, 0 };

		m_colorMix = (UINT)COLOR_MIX_OP::MIX_MUL;
		m_alphaMix = (UINT)COLOR_MIX_OP::MIX_MUL;
	}

	void SetTextureId(const wchar_t* texId);
	HRESULT SetSpritePatterns(XMUINT4* patterns, UINT numOfPatterns);

	HRESULT SetSpritePattern(UINT index, float w, float h, XMFLOAT4& pattern);

	void ChangeBlendMode(SPRITE_BLEND_MODE mode)
	{
		m_blendMode = mode;
	}

	void ChangeFilterMode(SPRITE_FILTER_MODE mode)
	{
		m_filterMode = mode;
	}

	void SetSpriteIndex(UINT idx)
	{
		m_patternIndex = idx;
	}

	const XMFLOAT4* GetTextureUV(UINT index)
	{
		return &m_uvArray.at(index).get()->uv;
	}

	const XMFLOAT4* GetTextureUV()
	{
		return &m_uvArray.at(m_patternIndex).get()->uv;
	}

	const XMUINT2* GetSpriteSize(UINT index)
	{
		return &m_uvArray.at(index).get()->size;
	}

	const XMUINT2* GetSpriteSize()
	{
		return &m_uvArray.at(m_patternIndex).get()->size;
	}

	const wchar_t* GetTextureId()
	{
		return texture_id;
	}

	void SetColor(float r, float g, float b, float a);
	void SetColorMix(COLOR_MIX_OP op);
	void SetAlphaMix(COLOR_MIX_OP op);

	const XMFLOAT4* GetColor();
	UINT	GetColorMix();
	UINT	GetAlphaMix();
};