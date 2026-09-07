
cbuffer cb : register(b0)
{
    float4x4 mvp;
    float4 mulColor;
}


#include "common/Lighting.hlsli"
// NOTE: common/Shadow.hlsli は t10 / SamplerComparisonState(s1) 前提で、
//       スプライトのルートシグネチャ(s0 のみ)では使えない。
//       ここでは手動深度比較で影を計算する(学習ガイド Step 2-3 と同じ方式)。


Texture2D<float4> g_albedoTexture : register(t0);
Texture2D<float4> g_normalTexture : register(t1);
Texture2D<float4> g_worldPos : register(t2);
// シャドウマップ(ディレクションライト i → g_shadowMap[i])。t3 ～ t3+MAX_SHADOW_NUM-1。
Texture2D<float4> g_shadowMap[MAX_SHADOW_NUM] : register(t3);


sampler g_sampler : register(s0);


struct VSIn{
    float4 pos : POSITION;
    float2 uv  : TEXCOORD0;
};

struct PSIn{
	float4 pos : SV_POSITION;
	float2 uv  : TEXCOORD0;
};



PSIn VSMain(VSIn In)
{
    PSIn psIn;
    psIn.pos = mul(mvp, In.pos);
    psIn.uv = In.uv;
    return psIn;
}


////////////////////////////////////////////////
// 影(0=影なし、1=影)を計算する。
//   worldPos    : ワールド座標(G-Buffer から取得)
//   N           : 正規化済みワールド法線
//   L           : 正規化済みライト方向(model.fx と同じ規約。実際の入射方向は -L)
//   shadowIndex : シャドウマップ番号(= ディレクションライト番号)
//
// shadowIndex は呼び出し側の [unroll] 展開後にリテラルになる前提。
// (テクスチャ配列は動的 index できないため)
////////////////////////////////////////////////
float CalcShadowInDeferred(const float3 worldPos, const float3 N, const float3 L, const int shadowIndex)
{
    // ワールド座標をライトビュープロジェクション空間へ
    const float4 posInLVP = mul(mLVP[shadowIndex], float4(worldPos, 1.0f));
    float2 shadowMapUV = posInLVP.xy / posInLVP.w;
    shadowMapUV = shadowMapUV * float2(0.5f, -0.5f) + 0.5f;
    const float zInLVP = posInLVP.z / posInLVP.w;

    // シャドウマップの外 or 深度範囲外なら影なし
    if (shadowMapUV.x < 0.0f || shadowMapUV.x > 1.0f
     || shadowMapUV.y < 0.0f || shadowMapUV.y > 1.0f
     || zInLVP < 0.0f || zInLVP > 1.0f)
    {
        return 0.0f;
    }

    // 傾斜依存バイアス(model.fx / Shadow.hlsli と同じ考え方。無いとシャドウアクネが出る)
    const float bias = max(localBias * (1.0f - dot(N, -L)), 0.0005f);

    // ライトから最も近い遮蔽物の深度と、自分の深度を比較する
    const float storedZ = g_shadowMap[shadowIndex].Sample(g_sampler, shadowMapUV).r;
    return (zInLVP - bias > storedZ) ? 1.0f : 0.0f;
}


float4 PSMain(PSIn In) : SV_Target0
{
    // G-Bufferから材料を取り出す
    const float4 albedoColor = g_albedoTexture.Sample(g_sampler, In.uv);
    const float3 normal = g_normalTexture.Sample(g_sampler, In.uv).xyz * 2.0f - 1.0f;
    const float3 worldPos = g_worldPos.Sample(g_sampler, In.uv).xyz;
    // G-Buffer にスペキュラマップは無いので係数は 1 固定
    const float specFactor = 1.0f;

    //========================================================================
    // ライティング計算
    //========================================================================
    const float3 N = normalize(normal);
    const float3 V = normalize(eyePos - worldPos);

    // ディレクションライトの反射光を計算(ライトごとに専用シャドウマップで影を落とす)
    float3 directionRef = float3(0.0f, 0.0f, 0.0f);
    [unroll]
    for (int d = 0; d < MAX_DIRECTION_LIGHT_NUM; ++d)
    {
        if (d < usingDirectionLightNum)
        {
            const float3 Ld = normalize(dirLights[d].lightDir);
            const float shadow = CalcShadowInDeferred(worldPos, N, Ld, d);
            const float3 diffuse = CalcDiffuseLighting(N, Ld, dirLights[d].lightColor);
            const float3 specular = CalcSpecularLighting(N, Ld, V, dirLights[d].lightColor, shininess, specIntensity) * specFactor;
            // 直接光だけを影で減衰させる
            directionRef += (diffuse + specular) * (1.0f - shadow);
        }
    }


    // ポイントライトの反射光を計算
    float3 pointRef = float3(0.0f, 0.0f, 0.0f);
    for (int i = 0; i < usingPointLightNum; ++i)
    {
        pointRef += CalcPointLightLighting(N, V, worldPos, pointLights[i], shininess, specFactor, specIntensity);
    }


    // スポットライトの反射光を計算(影は落とさない)
    float3 spotRef = float3(0.0f, 0.0f, 0.0f);
    for (int j = 0; j < usingSpotLightNum; ++j)
    {
        spotRef += CalcSpotLightLighting(N, V, worldPos, spotLights[j], shininess, specFactor, specIntensity);
    }


    const float3 refLight = directionRef + pointRef + spotRef;

    // アンビエントは影の影響を受けない
    const float3 ligColor = ambientLight.lightColor.xyz + refLight;
    const float3 finalColor = albedoColor.xyz * ligColor;

    // αにカメラからの距離を格納する(model.fx と揃える。DoF で使用)。
    return float4(finalColor, length(worldPos - eyePos));
}
