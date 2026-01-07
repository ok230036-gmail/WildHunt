#include "PEEdgeDrawHeader.hlsli"
float4 main(PS_INPUT input) : SV_Target
{
    float4 color;   // 出力
    float3 normal;  // 法線ベクトルのラプラシアン合計
    float depth;    // 深度値のラプラシアン合計
    
    // 中央を-8倍して周りを足す。周辺のUV値はxy。
    normal = -8.0 * NormalTex.Sample(Sampler, input.tex0).xyz;  // 中央の法線ベクトル
    normal += NormalTex.Sample(Sampler, input.tex1.xy).xyz;
    normal += NormalTex.Sample(Sampler, input.tex2.xy).xyz;
    normal += NormalTex.Sample(Sampler, input.tex3.xy).xyz;
    normal += NormalTex.Sample(Sampler, input.tex4.xy).xyz;
    normal += NormalTex.Sample(Sampler, input.tex5.xy).xyz;
    normal += NormalTex.Sample(Sampler, input.tex6.xy).xyz;
    normal += NormalTex.Sample(Sampler, input.tex7.xy).xyz;
    normal += NormalTex.Sample(Sampler, input.tex8.xy).xyz;
    
    // 深度の周辺UV値はzw。値がスカラーなので結果が＋ーどちらもありえる。
    depth = -8.0f * DepthTex.Sample(Sampler, input.tex0).x; // 中央の深度値
    depth += DepthTex.Sample(Sampler, input.tex1.zw).x;
    depth += DepthTex.Sample(Sampler, input.tex2.zw).x;
    depth += DepthTex.Sample(Sampler, input.tex3.zw).x;
    depth += DepthTex.Sample(Sampler, input.tex4.zw).x;
    depth += DepthTex.Sample(Sampler, input.tex5.zw).x;
    depth += DepthTex.Sample(Sampler, input.tex6.zw).x;
    depth += DepthTex.Sample(Sampler, input.tex7.zw).x;
    depth += DepthTex.Sample(Sampler, input.tex8.zw).x;
    
    // ここで値チェック
    // 合成法線の長さが0.8以上、または合成深度値が+-0.003を越えていたなら 端と判定
    if (length(normal) >= 0.8f || abs(depth) > 0.003f)
    {
        color = float4(0.4f, 0.4f, 0.4f, 1); // 端なので輪郭線。色を暗くする。RTVの設定で色は乗算。
    }
    else
    {
        color = float4(1, 1, 1, 1); // 真っ白。つまりそのままの色。
    }
    
    return color;
}
    