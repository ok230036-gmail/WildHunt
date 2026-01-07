#include "BaseConstantBuffers.hlsli"
#include "LightingFuncs.hlsli"
#include "PSConstantBufferHeader.hlsli"

float4 main(VS_OUT input) : SV_Target
{
    float4 normal = input.wnml;
    float2 uv = input.uv;
    
    // NormalTextureを持っている場合のみノーマルマップによる法線修正を計算
    if (TextureFlag & TEX_NORMAL)
    {
        normal = GetNormalVect(normal.xyz, input.wtan.xyz, input.wbnml.xyz, NormalTex.Sample(Sampler, input.uv).xyz);
    }
    
    // テクスチャカラーの取得
    float4 tex_color = Texture.Sample(Sampler, uv);
    // Diffuse * Albedoカラー計算。中身はLambert。
    
    float4 diffuse = MakeDiffuseColor(normal, input.color, dLightColor, dLightVector);
    
    float4 specular = 0;
    
    // Blinn-Phongのための反射モデル
    // 視線ベクトルの作成
    float3 viewVect = normalize(CameraPos - input.wpos.xyz); // 正規化も
    
    // ハーフベクトルは反射点基準の視線ベクトルと入射光ベクトルを足して正規化したもの。
    float3 halfVect = normalize(-dLightVector + viewVect);
    
    // Specular計算はhalfVectと法線で
    if (TextureFlag & TEX_SPECULAR) // テクスチャ所持チェック
    {
        float4 sptex; // スペキュラーマップの色
        sptex = SpecularTex.Sample(Sampler, uv);
        specular.xyz = dLightColor * MaterialSpecular.xyz * sptex.xyz *
        pow(saturate(dot(halfVect, normal.xyz)), MaterialSpecular.w);
    }
    else
    {
        // テクスチャがないのでマテリアルカラーだけ追加で反映
        specular.xyz = dLightColor * MaterialSpecular.xyz * pow(saturate(dot(halfVect, normal.xyz)), MaterialSpecular.w);
    }

    // テクスチャカラー * (ディフューズカラー(Light + Ambient)) + スペキュラカラー
    float4 finalColor = tex_color * (diffuse + AmbientColor) + specular;
    
    return finalColor;
}