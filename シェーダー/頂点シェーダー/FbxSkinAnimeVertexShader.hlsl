struct VS_IN
{
	float3 pos : POSITION0;
	float3 nor : NORMAL0;

    float3 tan : TANGENT0;

	float4 color : COLOR0;
	float2 uv : TEXTURE0;

	uint4	indices0 : BLENDINDICES0;
	uint4	indices1 : BLENDINDICES1;
	float4	weights0 : BLENDWEIGHT0;
	float4	weights1 : BLENDWEIGHT1;

};

struct VS_OUT
{
	float4 pos : SV_POSITION;
	float4 color : COLOR;
	float2 uv : TEXTURE0;
};

cbuffer ViewBuffer : register(b0)		// これはカメラの位置で決まる
{
	float4x4	View;					// ビュー変換行列
}

cbuffer ProjectionBuffer : register(b1)	// これはカメラの位置で決まる
{
	float4x4	Projection;				// 透視射影変換行列
}

cbuffer ModelBuffer : register(b2)		// メッシュが持つ自身のマトリクス
{
    float4x4	Model;					// ワールド変換行列
}

cbuffer MeshBone : register(b3)
{
	float4x4	bones[2];				// ボーン用配列。(1だと固定長になるので、2にして可変長に)
}

VS_OUT main(VS_IN input)
{
	VS_OUT output;
	output.pos = float4(input.pos, 1.0f);

	// スキンアニメマトリクス反映
	float4x4 boneMtx;

	boneMtx = bones[input.indices0[0]] * input.weights0[0];
	boneMtx += bones[input.indices0[1]] * input.weights0[1];
	boneMtx += bones[input.indices0[2]] * input.weights0[2];
	boneMtx += bones[input.indices0[3]] * input.weights0[3];
	boneMtx += bones[input.indices1[0]] * input.weights1[0];
	boneMtx += bones[input.indices1[1]] * input.weights1[1];

	output.pos = mul(boneMtx, output.pos);

	// ローカル座標 * ワールド座標変換行列
    output.pos = mul(output.pos, Model);

	// ワールド座標 * ビュー座標変換行列
	output.pos = mul(output.pos, View);
	
	// ビュー座標 * プロジェクション座標変換行列
	output.pos = mul(output.pos, Projection);

	// 頂点カラー
	output.color = input.color;

	//  Texture座標指定
	output.uv = input.uv;

	return output;
}
