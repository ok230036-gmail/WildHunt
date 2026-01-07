float4 MakeDiffuseColor(float4 normal, float4 inColor, float3 lightColor, float3 lightVect)
{
    // 平行光源の色はlightColor、方向はlightVect。
    // dotで内積計算
    float3 dlight = lightColor * saturate(dot(normal.xyz, -lightVect));
    
    float4 output;
    
    // ポリゴンの頂点カラーとDirectional Lightをかけた新しい頂点カラー
    output.xyz = inColor.xyz * dlight;
    output.w = inColor.w; // アルファは入力値のまま

    return output;
}

// NORMALの処理
float4 GetNormalVect(float3 baseNormal, float3 baseTangent, float3 binormal, float3 nmlTex)
{
    float4 normal; // nmlTexをワールド座標系に
    float3x3 tanMtx = // このTangent空間への変換マトリクス。
    {
        baseTangent,
        binormal,
        baseNormal,
    };
    
    // transposeで逆行列化。(直行正規形行列は転置すると逆行列になる)
    tanMtx = transpose(tanMtx);                 // Tangent空間をWorld空間に変換する行列にする

    nmlTex.xy = nmlTex.xy * 2.0f - 1.0f;        // XYは-1.0から1.0
    nmlTex.z = 0.5 + nmlTex.z * 0.5;            // Zは0.5から1.0
    nmlTex = normalize(nmlTex);                 // 法線マップはTangent空間のベクトル
    normal = float4(mul(nmlTex, tanMtx), 0);    // ワールド座標系に変換
    return normal;
}

// Shadow用の処理
float4 GetShadowColor(float4 pos, Texture2D lightDepth, SamplerComparisonState samp)
{
    float4 color = { 1, 1, 1, 1 };
    float3 screenPos = pos.xyz / pos.w;                             // プロジェクション座標をｗで割って、スクリーン座標になる。
    float2 uv = (screenPos.xy + float2(1, -1)) * float2(0.5, -0.5); // D3Dのスクリーン座標だとXYは-1～1。かつ、Yが上下逆。
    
    if (uv.x < 0.0f || uv.y < 0.0f || uv.x > 1.0f || uv.y > 1.0f)   // UVが0.0～1.0に収まっているかチェック
        return color;
    
    // 影のエッジぼかすための処理
    float4 depth = lightDepth.SampleCmp(samp, uv, screenPos.z - 0.001); // -0.001でシャドウアクネの対策
    float shadowWeight = lerp(0.5, 1.0, depth);                         // 0.5より小さいと流石に黒いので0.5に強制
    
    // シャドウマップの深度がスクリーン座標のZより小さい
    if (depth.r < screenPos.z)
    {
        color.xyz = shadowWeight;
    }
    return color;
}