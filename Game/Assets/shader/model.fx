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


#include "common/Lighting.hlsli"


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


///////////////////////////////////////
// Common vertex shader code.
// Provides: ModelCb(b0: mWorld/mView/mProj), SVSIn, bone matrices (t3),
//           and the entry points VSMain / VSMainSkin / VSMainInstancing, etc.
///////////////////////////////////////
#include "common/ModelVSCommon.hlsli"

///////////////////////////////////////
// Shader resources.
// The tkm material binds the albedo texture to t0.
// (t1 = normal map, t2 = metallic/smooth — you can add them when you need them.)
///////////////////////////////////////
Texture2D<float4> g_albedoTexture : register(t0);
Texture2D<float4> g_normalTexture : register(t1);
Texture2D<float4> g_specularTexture : register(t2);
sampler g_sampler : register(s0);

///////////////////////////////////////
// シャドウ共通処理。
// g_shadowMap / g_shadowMapSampler のリソース宣言と CalcShadow() を提供する。
// フォワード(このファイル)とデファード(deferredLighting.fx)で共有する。
///////////////////////////////////////
#include "common/Shadow.hlsli"

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

    // 視線方向を正規化
    const float3 V = normalize(eyePos - In.worldPos);

    // ディレクションライトの反射光を計算(ライトごとに専用シャドウマップで影を落とす)
    float3 directionRef = float3(0.0f, 0.0f, 0.0f);
    [unroll]
    for (int d = 0; d < MAX_DIRECTION_LIGHT_NUM; ++d)
    {
        if (d < usingDirectionLightNum)
        {
            const float3 Ld = normalize(dirLights[d].lightDir);
            const float shadow = CalcShadow(In.worldPos, N, Ld, d);
            const float3 diffuse = CalcDiffuseLighting(N, Ld, dirLights[d].lightColor);
            const float3 specular = CalcSpecularLighting(N, Ld, V, dirLights[d].lightColor, shininess, specIntensity) * specFactor;
            directionRef += (diffuse + specular) * (1.0f - shadow);
        }
    }

    // ポイントライトの反射光を計算
    float3 pointRef = float3(0.0f, 0.0f, 0.0f);
    for (int i = 0; i < usingPointLightNum; ++i)
    {
        pointRef += CalcPointLightLighting(N, V, In.worldPos, pointLights[i], shininess, specFactor, specIntensity);
    }

    // スポットライトの反射光を計算(影は落とさない)
    float3 spotRef = float3(0.0f, 0.0f, 0.0f);
    for (int j = 0; j < usingSpotLightNum; ++j)
    {
        spotRef += CalcSpotLightLighting(N, V, In.worldPos, spotLights[j], shininess, specFactor, specIntensity);
    }

    // 反射光を合成
    const float3 refLight = directionRef + pointRef + spotRef;


    // アンビエントは影の影響を受けない。直接光だけを影で減衰させる。
    const float3 ligColor = ambientLight.lightColor.xyz + refLight;
    albedoColor.xyz *= ligColor;

    // αにカメラからの距離を格納する(DoFで使用)。
    return float4(albedoColor.xyz, length(In.worldPos - eyePos));
}