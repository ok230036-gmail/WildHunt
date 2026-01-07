struct VS_INPUT
{
    float4 pos : POSITION0;
    float2 uv : TEXCOORD0;
};
struct PS_INPUT
{
    float4 pos : SV_Position;
    float2 tex0 : TEXCOORD0;
    float4 tex1 : TEXCOORD1;
    float4 tex2 : TEXCOORD2;
    float4 tex3 : TEXCOORD3;
    float4 tex4 : TEXCOORD4;
    float4 tex5 : TEXCOORD5;
    float4 tex6 : TEXCOORD6;
    float4 tex7 : TEXCOORD7;
    float4 tex8 : TEXCOORD8;
};
Texture2D NormalTex : register(t0, space0);     // Textureをスロット0の0番目のテクスチャレジスタに法線バッファ設定
Texture2D DepthTex : register(t1, space0);      // Textureをスロット0の1番目のテクスチャレジスタに深度バッファ設定
SamplerState Sampler : register(s0, space0);    // Samplerをスロット0の0番目のサンプラレジスタに設定