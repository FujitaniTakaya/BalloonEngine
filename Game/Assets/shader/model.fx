/*!
 * @brief   Minimal model shader (STARTING POINT).
 *
 * Right now this only draws the albedo (base color) texture. There is NO lighting.
 * Your job is to add lighting here, step by step:
 *   1. ambient        (add a constant brightness)
 *   2. diffuse         (directional light + Lambert:  dot(normal, -lightDir))
 *   3. specular        (Phong / Blinn-Phong highlight)
 *   4. specular map    (control the highlight strength per-pixel with a texture)
 *   5. normal map      (per-pixel normals using the tangent space / TBN)
 *
 * See: 学習ガイド_自作レンダリングエンジン.md  (Phase 1)
 *
 * NOTE: To pass values (light direction/color, ambient, etc.) from C++ into this
 *       shader, add your own cbuffer here and fill it from a ConstantBuffer on the
 *       C++ side. The engine does NOT feed lighting data anymore (that used to be
 *       k2Engine's job, which you are now replacing).
 */


#include "Lighting.hlsli"


////////////////////////////////////////////////
// Pixel shader input.
////////////////////////////////////////////////
struct SPSIn
{
    float4 pos      : SV_POSITION;  // Clip-space position.
    float3 normal   : NORMAL;       // World-space normal.
    float3 tangent  : TANGENT;      // World-space tangent   (for normal mapping later).
    float3 biNormal : BINORMAL;     // World-space binormal  (for normal mapping later).
    float2 uv       : TEXCOORD0;    // UV.
    float3 worldPos : TEXCOORD1;    // World-space position  (for specular later).
    float4 posInLVP : TEXCOORD2;    // Light View Projection space position.
};


/////////////////////////////////////////////////
// Pixel shader output (G-Buffer).
/////////////////////////////////////////////////
struct SPSOut
{
    float4 albedo : SV_Target0;  // Albedo (base color).
    float4 normal : SV_Target1;  // Normal (world-space, packed to 0~1).
    float3 worldPos : SV_Target2; // World-space position.
};


///////////////////////////////////////
// Common vertex shader code.
// Provides: ModelCb(b0: mWorld/mView/mProj), SVSIn, bone matrices (t3),
//           and the entry points VSMain / VSMainSkin / VSMainInstancing, etc.
///////////////////////////////////////
#include "ModelVSCommon.hlsli"

///////////////////////////////////////
// Shader resources.
// The tkm material binds the albedo texture to t0.
// (t1 = normal map, t2 = metallic/smooth — you can add them when you need them.)
///////////////////////////////////////
Texture2D<float4> g_albedoTexture : register(t0);
Texture2D<float4> g_normalTexture : register(t1);
Texture2D<float4> g_specularTexture : register(t2);
Texture2D<float4> g_shadowMap : register(t10);
sampler g_sampler : register(s0);
SamplerComparisonState g_shadowMapSampler : register(s1);

////////////////////////////////////////////////
// Vertex shader core (called by the VSMain* entry points in ModelVSCommon.h).
////////////////////////////////////////////////
SPSIn VSMainCore(SVSIn vsIn, float4x4 mWorldLocal, uniform bool isUsePreComputedVertexBuffer)
{
    SPSIn psIn;

    // Local space -> world space.
    psIn.pos = CalcVertexPositionInWorldSpace(vsIn.pos, mWorldLocal, isUsePreComputedVertexBuffer);
    psIn.worldPos = psIn.pos;

    // World -> view -> projection (clip) space.
    psIn.pos = mul(mView, psIn.pos);
    psIn.pos = mul(mProj, psIn.pos);

    // World-space normal / tangent / binormal.
    CalcVertexNormalTangentBiNormalInWorldSpace(
        psIn.normal,
        psIn.tangent,
        psIn.biNormal,
        mWorldLocal,
        vsIn.normal,
        vsIn.tangent,
        vsIn.biNormal,
        isUsePreComputedVertexBuffer
    );

    psIn.uv = vsIn.uv;
    return psIn;
}


////////////////////////////////////////////////
// 影(0=影なし、1=影)を計算。PCF + 傾斜依存バイアス。
////////////////////////////////////////////////
float CalcShadow(const float3 worldPos, const float3 N, const float3 L)
{
    const float4 posInLVP = mul(mLVP, float4(worldPos, 1.0f));
    float2 shadowMapUV = posInLVP.xy / posInLVP.w;
    shadowMapUV = shadowMapUV * float2(0.5f, -0.5f) + 0.5f;

    if (shadowMapUV.x <= 0.0f || shadowMapUV.x >= 1.0f
     || shadowMapUV.y <= 0.0f || shadowMapUV.y >= 1.0f)
    {
        return 0.0f;
    }

    const float zInLVP = posInLVP.z / posInLVP.w;
    // 傾斜依存バイアス(これを使わないと、モデルに模様が出る)
    const float bias = max(localBias * (1.0f - dot(N, -L)), 0.0001f);

    return g_shadowMap.SampleCmpLevelZero(g_shadowMapSampler, shadowMapUV, zInLVP - bias);
}

////////////////////////////////////////////////
// Pixel shader.
// For now: just output the albedo texture. Add your lighting here.
////////////////////////////////////////////////
float4 PSMain(SPSIn In) : SV_Target0
{
    float4 albedoColor = g_albedoTexture.Sample(g_sampler, In.uv);
    const float specFactor = g_specularTexture.Sample(g_sampler, In.uv).r;

    // 法線(ノーマルマップ使用)
    const float3 normal = CalcNormalFromNormalMap(In.tangent, In.biNormal, In.normal, g_normalTexture.Sample(g_sampler, In.uv).xyz);
    const float3 N = normalize(normal);

    // ライトの方向、視線方向を正規化
    const float3 L = normalize(dirLight.lightDir);
    const float3 V = normalize(eyePos - In.worldPos);

    // ディレクションライトの反射光を計算
    const float3 directionRef =
        CalcDiffuseLighting(N, L, dirLight.lightColor.xyz)
        + CalcSpecularLighting(N, L, V, dirLight.lightColor.xyz, shininess) * specFactor;

    // ポイントライトの反射光を計算
    float3 pointRef = float3(0.0f, 0.0f, 0.0f);
    for (int i = 0; i < usingPointLightNum; ++i)
    {
        pointRef += CalcPointLightLighting(N, V, In.worldPos, pointLights[i], shininess, specFactor);
    }

    // スポットライトの反射光を計算
    float3 spotRef = float3(0.0f, 0.0f, 0.0f);
    for (int j = 0; j < usingSpotLightNum; ++j)
    {
        spotRef += CalcSpotLightLighting(N, V, In.worldPos, spotLights[j], shininess, specFactor);
    }

    // 反射光を合成
    const float3 refLight = directionRef + pointRef + spotRef;

    // 影(0=影なし、1=影)。アンビエントには掛けず、直接光(refLight)だけを減衰させる。
    const float shadow = CalcShadow(In.worldPos, N, L);

    // アンビエントは影の影響を受けない。直接光だけを影で減衰させる。
    const float3 ligColor = ambientLight.lightColor.xyz + refLight * (1.0f - shadow);
    albedoColor.xyz *= ligColor;

    return albedoColor;
}


SPSOut PSMainDeferred(SPSIn In)
{
    // G-Bufferに出力
    SPSOut psOut;

    // アルベドカラーを出力
    psOut.albedo = g_albedoTexture.Sample(g_sampler, In.uv);

    const float3 normal = CalcNormalFromNormalMap(In.tangent, In.biNormal, In.normal, g_normalTexture.Sample(g_sampler, In.uv).xyz);

    psOut.normal.w = g_specularTexture.Sample(g_sampler, In.uv).r; // スペキュラーマップをwに格納
    // 法線を出力
    psOut.normal = float4(((normal / 2.0f) + 0.5f), 1.0f);

    // ワールド座標を出力
    psOut.worldPos = In.worldPos;

    return psOut;
}