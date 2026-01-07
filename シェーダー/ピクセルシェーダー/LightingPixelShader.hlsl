#include "BaseConstantBuffers.hlsli"
#include "PSConstantBufferHeader.hlsli"
#include "LightingFuncs.hlsli" // 今回の主題処理本体

float4 main(VS_OUT input) : SV_Target
{
    // テクスチャカラーの取得
    float4 tex_color = Texture.Sample(Sampler, input.uv);
    
    // Diffuse * Albedo カラー計算。引数は(法線ベクトル、頂点カラー、光源カラー、光ベクトル)
    float4 diffuse = MakeDiffuseColor(input.wnml, input.color, dLightColor, dLightVector);
    
    // ディフューズカラー(Light + Ambient) * テクスチャカラー
    float4 finalColor = tex_color * (diffuse + AmbientColor);
    
    return finalColor;
}