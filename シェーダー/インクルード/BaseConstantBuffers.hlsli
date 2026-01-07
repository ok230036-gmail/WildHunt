struct VS_OUT
{
    float4 pos : SV_POSITION;
    float4 wnml : NORMAL0; // ワールド座標系での法線 今回ついに使う時が来る
    
    float4 wpos : POSITION0; // ワールド座標系での座標
    
    float4 wtan : TANGENT0; // ワールド座標系での法線に対するTangentベクトル
    float4 wbnml : NORMAL1; // BINORMAL
    
    // Depth Shadow用に追加
    float4 lightPos : POSITION1; // シャドウマップ軸上でのプロジェクション座標
    
    float4 color : COLOR0;
    float2 uv : TEXCOORD0;
};

cbuffer ViewBuffer : register(b0) // これはカメラの位置で決まる
{
    float4x4 View; // ビュー変換行列
}

cbuffer ProjectionBuffer : register(b1) // これはカメラの設定で決まる
{
    float4x4 Projection; // 透視射影変換行列
}

cbuffer WorldBuffer : register(b2) // メッシュが持つ自身のマトリクス
{
    float4x4 World; // ワールド変換行列
}

cbuffer DirectionalLight : register(b3) // 今回の新規実装 平行光源
{
    float3 dLightColor;     // 色
    float3 dLightVector;    // 向き
}

// DepthShadow用に追加
cbuffer cbShadowMap : register(b7)
{
    float4x4 lightViewProjection;
}