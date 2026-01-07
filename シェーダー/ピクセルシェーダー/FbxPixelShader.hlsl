struct PS_IN
{
	float4 pos : SV_POSITION;
	float4 color : COLOR;
	float2 uv : TEXTURE0;
};

Texture2D    Texture : register(t0, space0); // Textureをスロット0の0番目のテクスチャレジスタに設定
SamplerState Sampler : register(s0, space0); // Samplerをスロット0の0番目のサンプラレジスタに設定

float4 main(PS_IN input) : SV_Target
{
	// テクスチャカラーの取得
	float4 tex_color = Texture.Sample(Sampler, input.uv);

	// アンビエントカラー + ディフューズカラー + テクスチャカラー
	float4 finalColor = tex_color * input.color;

	// 輝度値を計算
    // float y = 0.299f * finalColor.r + 0.587f * finalColor.g + 0.114f * finalColor.b;
	
    // float Cb = -0.091f;
    // float Cr = 0.056f;

    // float blue = 1.772f * Cb + y;
    // float red = 1.402f * Cr + y;
    // float green = y + Cb * -0.344f + Cr * -0.714f;
	
    // finalColor.rgb = float3(red, green, blue);	// セピア化
    // finalColor.rgb = float3(y, y, y);	// グレースケール化
	
	return finalColor;
}
