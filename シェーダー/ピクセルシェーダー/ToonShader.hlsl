#include "BaseConstantBuffers.hlsli"    // 基本定数バッファ
#include "LightingFuncs.hlsli"          // 追加ライト処理用関数
#include "PSConstantBufferHeader.hlsli" // PSの基本バッファ 今回色々増えて行くぞ。

struct TONE_OUT // ピクセルシェーダ出力に使う構造体
{
    float4 outColor : SV_TARGET0;
    
    // 輪郭線抽出
    float4 outNormal : SV_TARGET1; // ここに法線を書けばOK
};

Texture2D ToneTex : register(t3, space0);   // Textureをスロット0の3番目のテクスチャレジスタに設定

TONE_OUT main(VS_OUT input)
{
    TONE_OUT res;
    float2 uv = input.uv;
    float4 normal = input.wnml; // NORMALが来ると変換

    // テクスチャカラーの取得
    float4 tex_color = Texture.Sample(Sampler, uv);
    
    // Alpha Test 半透明のメッシュを書きこまないため
    if (tex_color.w < 0.1f)
        discard;
    
    // Normal Factor Check
    if (TextureFlag & TEX_NORMAL) // BIT計算はShader5以上じゃないと動かないらしい、エラー吐いたら注意
    {
        normal = GetNormalVect(input.wnml.xyz, input.wtan.xyz, input.wbnml.xyz, NormalTex.Sample(Sampler, uv).xyz);
    }
    
    float p;
    
    // 影のエッジぼかす処理
    float4 shadowColor = GetShadowColor(input.lightPos, ShadowMapTex, ShadowSampler);
    
    tex_color *= shadowColor;   // テクスチャ自体の色に補正値をかける。

    if (shadowColor.x < 1.0)
    {
        p = 0.2;    // 最も暗く。ここでトーンテクスチャに影の時だけ選べる暗さの色があると・・・
    }
    else
    {
        p = dot(input.wnml.xyz, -dLightVector.xyz);
        p = p * 0.5f + 0.5; // HalfLambert公式
        p = p * p;          // もう一回、掛ける事で0.25f～1.0fに
    }
    
    // このハーフランバートで作った拡散光量値でToneMapを取得
    // ハーフにする事で、値の光量の分布を平均化
    float4 toneCL = ToneTex.Sample(Sampler, float2(p, 0.0f)); 
    
    res.outColor.xyz = tex_color.xyz * toneCL.xyz; // ToneMapの色が拡散光（Diffuse）のかわり。
    
    res.outColor.w = tex_color.w * input.color.w;
    
    // res.outNormal = float4(input.wnml.x, input.wnml.y, input.wnml.z, 1.0f); // メッシュの輪郭線のみを抽出したい場合
    res.outNormal = float4(normal.x, normal.y, normal.z, 1.0f); // ノーマルマップ上の輪郭線も抽出したい場合
    
    return res;
}