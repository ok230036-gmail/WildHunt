#include "BaseConstantBuffers.hlsli"
struct VS_IN
{
    float3 pos : POSITION0;
    float3 nor : NORMAL0;
    
    float3 tan : TANGENT0;

    float4 color : COLOR0; // Albedoのようなもの
    float2 uv : TEXTURE0;
};

VS_OUT main(VS_IN input)
{
    VS_OUT output;
    
    // 行列変換用に４要素にふやして末尾（w）に1.0fを追加
    output.pos = float4(input.pos, 1.0f);
    
    // ローカル座標 * ワールド座標変換行列
    output.pos = mul(output.pos, World);
    
    // 重要ポイント、ビュー座標にする前の座標値をoutputに保存
    output.wpos = output.pos;
    
    // ワールド座標 * ビュー座標変換行列
    output.pos = mul(output.pos, View);
    
    // ビュー座標 * プロジェクション座標変換行列
    output.pos = mul(output.pos, Projection);

    // 法線をワールド座標系に変換。
    float4 normal = float4(input.nor, 0.0f);
    normal = mul(normal, World);
    
    // 正規化 もしかしたらWorldでscale変わっているかもしれないため
    normal = normalize(normal); 

    output.wnml = normal; // PSに渡す用
    
    float4 tangent = float4(input.tan, 0.0f);
    tangent = mul(tangent, World);
    tangent = normalize(tangent); // 正規化

    output.wtan = tangent; // PSに渡す値
    output.wbnml = float4(normalize(cross(normal.xyz, tangent.xyz)), 0.0);
    
    // ライトから見たプロジェクション座標を保存
    output.lightPos = mul(output.wpos, lightViewProjection);
    
    // ピクセルシェーダに渡す
    output.color = input.color;
    
    // Texture座標指定
    output.uv = input.uv;
    
    return output;
}