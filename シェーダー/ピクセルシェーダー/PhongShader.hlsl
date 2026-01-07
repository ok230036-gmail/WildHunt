#include "BaseConstantBuffers.hlsli"    // 基本定数バッファ
#include "LightingFuncs.hlsli"          // 追加ライト処理用関数
#include "PSConstantBufferHeader.hlsli" // PSの追加バッファ 今回色々増えて行くぞ。

float4 main(VS_OUT input) : SV_Target
{
    float2 uv = input.uv;
    float4 normal = input.wnml;
    
    // Normal Textureを持っている場合のみノーマルマップによる法線修正を計算
    if (TextureFlag & TEX_NORMAL)   // 何気にBIT計算はShader5以上じゃないと動かないので注意
    {
        float3 nmlTex = NormalTex.Sample(Sampler, uv);
        normal = GetNormalVect(normal.xyz, input.wtan.xyz, input.wbnml.xyz, nmlTex.xyz);
    }

    // テクスチャカラーの取得
    float4 tex_color = Texture.Sample(Sampler, uv);
    
    // Diffuse * Albedo カラー計算。中身はLambert。
    float4 diffuse = MakeDiffuseColor(normal, input.color, dLightColor, dLightVector);
        float4 specular = 0;        // Phong反射モデルの実装
    if (TextureFlag & TEX_SPECULAR) // テクスチャ所持チェック
    {
        float4 sptex;   // スペキュラーマップの色
        sptex = SpecularTex.Sample(Sampler, uv);
        
        // Phong反射の反射光計算式そのまま。
        float3 refVect = dLightVector + 2.0f * dot(-dLightVector, normal.xyz) * normal.xyz;
    
        // 視線ベクトルの作成
        // 視線ベクトルはカメラから反射点へのベクトルの逆方向
        float3 viewVect = normalize(CameraPos - input.wpos.xyz);

        specular.xyz = dLightColor * MaterialSpecular.xyz * sptex.xyz * pow(saturate(dot(refVect, viewVect)), MaterialSpecular.w);
    }

    // ディフューズカラー(Light + Ambient) * テクスチャカラー + スペキュラカラー
    float4 finalColor = tex_color * (diffuse + AmbientColor) + specular;
    finalColor.w = tex_color.w * input.color.w;
    
    return finalColor;}