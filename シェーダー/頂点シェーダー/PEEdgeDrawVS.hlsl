#include "PEEdgeDrawHeader.hlsli"

PS_INPUT main(VS_INPUT input)
{
    // 周辺の法線と深度の値を収集する為のUV値を生成する。
    PS_INPUT output;
    float2 texSize;
    float level;
    
    // 法線バッファからテクスチャのサイズを取得
    NormalTex.GetDimensions(0, texSize.x, texSize.y, level); // Mipmaplevel outWidht, outHeight, outLevel
        
    float2 uv = input.uv;   // 初期UV座標
    output.pos = input.pos; // SV_POSITIONは必要
    
    if (input.pos.x < 0.0f)
        output.pos.x = -1.0f;
    else
        output.pos.x = 1.0f;
    
    if (input.pos.y < 0.0f)
        output.pos.y = -1.0f;
    else
        output.pos.y = 1.0f;
    
    output.tex0 = uv;
    
    float offset = 0.5f;                // 法線用隣接UV値

    float xDef = offset / texSize.x;    // 差分U
    float yDef = offset / texSize.y;    // 差分V
    
    // こっちは法線
    output.tex1.xy = uv + float2(-xDef, -yDef); // Left Top
    output.tex2.xy = uv + float2(-xDef, 0.0f);  // Left Center
    output.tex3.xy = uv + float2(-xDef, yDef);  // Left Bottom
    output.tex4.xy = uv + float2(0.0f, -yDef);  // Center Top
    
    // Center Centerはtex0
    output.tex5.xy = uv + float2(0.0f, yDef);   // Center Bottom
    output.tex6.xy = uv + float2(xDef, -yDef);  // Right Top
    output.tex7.xy = uv + float2(xDef, 0.0f);   // Right Center
    output.tex8.xy = uv + float2(xDef, yDef);   // Right Bottom
    
    // offsetを変えてzwに保存
    offset = 1.0f;  // 深度用隣接UV値
    xDef = offset / texSize.x;
    yDef = offset / texSize.y;
    
    // こっちは深度
    output.tex1.zw = uv + float2(-xDef, -yDef); // Left Top
    output.tex2.zw = uv + float2(-xDef, 0.0f);  // Left Center
    output.tex3.zw = uv + float2(-xDef, yDef);  // Left Bottom
    output.tex4.zw = uv + float2(0.0f, -yDef);  // Center Top
    
    // Center Centerはtex0
    output.tex5.zw = uv + float2(0.0f, yDef);   // Center Bottom
    output.tex6.zw = uv + float2(xDef, -yDef);  // Right Top
    output.tex7.zw = uv + float2(xDef, 0.0f);   // Right Center
    output.tex8.zw = uv + float2(xDef, yDef);   // Right Bottom

    return output;
}
    