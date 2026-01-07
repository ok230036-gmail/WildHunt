cbuffer cbAmbientColor : register(b4)
{
    float4 AmbientColor;
}

Texture2D Texture : register(t0, space0);       // Textureをスロット0の0番目のテクスチャレジスタに設定
SamplerState Sampler : register(s0, space0);    // Samplerをスロット0の0番目のサンプラレジスタに設定

// テクスチャ追加
Texture2D SpecularTex : register(t1, space0);   // Textureをスロット0の1番目のテクスチャレジスタに設定
Texture2D NormalTex : register(t2, space0);     // Textureをスロット0の2番目のテクスチャレジスタに設定

cbuffer cbCameraPosition : register(b5)
{
    float3 CameraPos; // カメラ座標
}

// normal & specular texture + Material color
cbuffer cbMaterialInfo : register(b6)
{
    float4 MaterialDiffuse;
    float4 MaterialAmbient;
    float4 MaterialSpecular;
    
    uint TextureFlag;
}

// Shadowの処理
Texture2D ShadowMapTex : register(t4, space0); // ShadowMapTextureをスロット0の４番目に設定。

// 影のエッジぼかす
SamplerComparisonState ShadowSampler : register(s1, space0); // ComparisonSamplerをスロット0の1番目のサンプラレジスタに設定


// どのテクスチャを持っているかフラグ
#define TEX_DIFFUSE 0x01
#define TEX_NORMAL 0x02
#define TEX_SPECULAR 0x04
#define TEX_FALLOFF 0x08
#define TEX_REFLECTION 0x10
#define PSMODE_TOON 0x01
