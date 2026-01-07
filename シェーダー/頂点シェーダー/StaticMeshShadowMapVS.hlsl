struct VS_IN
{
    float3 pos : POSITION0;
    float3 nor : NORMAL0;
    float3 tan : TANGENT0;
    float4 color : COLOR0;
    float2 uv : TEXTURE0;
};

cbuffer ViewBuffer : register(b0)       // これはカメラの位置で決まる
{
    float4x4 View;                      // ビュー変換行列
}

cbuffer ProjectionBuffer : register(b1) // これはカメラの位置で決まる
{
    float4x4 Projection;                // 透視射影変換行列
}

cbuffer WorldBuffer : register(b2)      // メッシュが持つ自身のマトリクス
{
    float4x4 World;                     // ワールド変換行列
}
float4 main(VS_IN input) : SV_Position
{
    float4 pos;
    
    // 行列変換用に４要素にふやして末尾（w）に1.0fを追加
    pos = float4(input.pos, 1.0f);
    
    // ローカル座標 * ワールド座標変換行列
    pos = mul(pos, World);
    
    // ワールド座標 * ビュー座標変換行列
    pos = mul(pos, View);
    
    // ビュー座標 * プロジェクション座標変換行列
    pos = mul(pos, Projection);
    
    return pos;
}