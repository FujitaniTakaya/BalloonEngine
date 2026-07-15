/*!
 * @brief	スプライトシェーダー�?
 */

cbuffer cb : register(b0){
	float4x4 mvp;		// モ�?ルビュープロジェクション行�??
	float4 mulColor;	// 乗算カラー。ピクセルシェーダーの出力結果に乗算されます�?
};


/*!
 * @brief   Directional light data. *
 */
struct DirectionLight
{
    float3 lightDir;
    float pad;
    float4 lightColor;
};


/*!
 * @brief   Constant buffer for lighting data.
 */
cbuffer LightCb : register(b1)
{
    DirectionLight dirLight;
    float4 ambientColor;
    float3 eyePos;
    float pad;
};


struct VSInput{
	float4 pos : POSITION;
	float2 uv  : TEXCOORD0;
};

struct PSInput{
	float4 pos : SV_POSITION;
	float2 uv  : TEXCOORD0;
};

Texture2D<float4> g_albedoTexture : register(t0);
Texture2D<float4> g_normalTexture : register(t1);
sampler g_sampler : register(s0);

PSInput VSMain(VSInput In) 
{
	PSInput psIn;
	psIn.pos = mul( mvp, In.pos );
	psIn.uv = In.uv;
	return psIn;
}
float4 PSMain( PSInput In ) : SV_Target0
{
	// G-Bufferの内容を使ってライティング
	float4 color = g_albedoTexture.Sample(g_sampler, In.uv);
	float3 normal = g_normalTexture.Sample(g_sampler, In.uv).xyz;
	normal = (normal * 2.0f) - 1.0f;

	// ライトを計算
	float3 lig = 0.0f;
	float t = max(0.0f, dot(normal, dirLight.lightDir) * -1.0f);
	lig = dirLight.lightColor.xyz * t;
	float4 finalColor = color;
	finalColor.xyz *= lig;
	
	return finalColor;

	return g_albedoTexture.Sample(g_sampler, In.uv) * mulColor;
}
float4 PSMainGamma( PSInput In ) : SV_Target0
{
	float4 color = g_albedoTexture.Sample(g_sampler, In.uv) * mulColor;
	color.xyz =  pow(max( color.xyz, 0.0001f ), 1.0f/2.2f);
	return color;
}


float4 PSMainDeferred(PSInput In) : SV_Target0
{
	float4 albedo = g_albedoTexture.Sample(g_sampler, In.uv) * mulColor;
	float3 normal = g_normalTexture.Sample(g_sampler, In.uv).xyz * 2.0f - 1.0f;

	float4 ligColor = dirLight.lightColor * max(0.0f, dot(normal, dirLight.lightDir) * -1.0f);
	return (albedo * ligColor) + ambientColor;
}