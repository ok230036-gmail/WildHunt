#include "SpriteCharacter.h"
#include "TextureManager.h"
#include "MyAccessHub.h"

void SpriteCharacter::SetTextureId(const wchar_t* texId)
{
	if (texture_id != texId)
	{
		texture_id = texId;
	}
}

HRESULT SpriteCharacter::SetSpritePatterns(XMUINT4* patterns, UINT numOfPatterns)
{
    auto texContainer = MyAccessHub::GetMyGameEngine()->GetTextureManager()->GetTexture(texture_id);

    if (texContainer == nullptr)
        return E_FAIL;

    const XMUINT4* tempUint = nullptr;
    unique_ptr<SpritePattern> tempSp;

    m_uvArray.clear();
    m_uvArray.resize(numOfPatterns);

    for (UINT i = 0; i < numOfPatterns; i++)
    {
        tempUint = &patterns[i];

        // unique_ptrはコピー出来ないから、unique_ptr所有権移動
        tempSp = move(m_uvArray.at(i));
        tempSp.reset(new SpritePattern());

        tempSp->size.x = tempUint->z - tempUint->x;
        tempSp->size.y = tempUint->w - tempUint->y;

        tempSp->uv.x = tempUint->x / texContainer->fWidth;
        tempSp->uv.y = tempUint->y / texContainer->fHeight;

        tempSp->uv.z = tempUint->z / texContainer->fWidth;
        tempSp->uv.w = tempUint->w / texContainer->fHeight;

        // unique_ptr所有権移動
        m_uvArray.at(i) = move(tempSp);
    }

    return S_OK;
}

// HUD MODE
HRESULT SpriteCharacter::SetSpritePattern(UINT index, float w, float h, XMFLOAT4& pattern)
{
    if (m_uvArray.size() < index + 1) 
    {
        m_uvArray.resize(index + 1);
    }
    
    if (m_uvArray[index] == nullptr)
    {
        m_uvArray[index] = make_unique<SpritePattern>();
    }

    m_uvArray[index]->size.x = w;
    m_uvArray[index]->size.y = h;
    m_uvArray[index]->uv.x = pattern.x;
    m_uvArray[index]->uv.y = pattern.y;
    m_uvArray[index]->uv.z = pattern.z;
    m_uvArray[index]->uv.w = pattern.w;

    return S_OK;
}

// ColorMix
void SpriteCharacter::SetColor(float r, float g, float b, float a)
{
    m_color.x = r;
    m_color.y = g;
    m_color.z = b;
    m_color.w = a;
}

void SpriteCharacter::SetColorMix(COLOR_MIX_OP op)
{
    m_colorMix = (UINT)op;
}

void SpriteCharacter::SetAlphaMix(COLOR_MIX_OP op)
{
    m_alphaMix = (UINT)op;
}

const XMFLOAT4* SpriteCharacter::GetColor()
{
    return &m_color;
}

UINT SpriteCharacter::GetColorMix()
{
    return m_colorMix;
}

UINT SpriteCharacter::GetAlphaMix()
{
    return m_alphaMix;
}