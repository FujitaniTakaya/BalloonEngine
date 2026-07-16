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
#include "ModelVSCommon.h"

///////////////////////////////////////
// Shader resources.
// The tkm material binds the albedo texture to t0.
// (t1 = normal map, t2 = metallic/smooth — you can add them when you need them.)
///////////////////////////////////////
Texture2D<float4> g_albedoTexture : register(t0);
Texture2D<float4> g_normalTexture : register(t1);
Texture2D<float4> g_specularTexture : register(t2);
sampler g_sampler : register(s0);

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
    const float specPower = g_specularTexture.Sample(g_sampler, In.uv).r;
    
    // 法線
   const float3 normal = CalcNormalFromNormalMap(In.tangent, In.biNormal, In.normal, g_normalTexture.Sample(g_sampler, In.uv).xyz);

    // ライトの方向と法線を正規化
    const float3 L = normalize(dirLight.lightDir);
    
    // ノーマルマップ使用
    const float3 N = normalize(normal);
    // ノーマルマップ使用しない
    // const float3 N = normalize(In.normal);


    // 拡散反射光を計算
    const float3 diffuse = CalcDiffuseLighting(N, L, dirLight.lightColor.xyz);

    // 鏡面反射光を計算
    // スペキュラーマップを使用
    const float3 specular = CalcSpecularLighting(N, L, eyePos, In.worldPos, dirLight.lightColor.xyz, 64.0f) * specPower;
    // スペキュラーマップを使用しない
    // const float3 specular = CalcSpecularLighting(N, L, eyePos, In.worldPos, dirLight.lightColor.xyz, 64.0f);

    // 反射光を合成
    const float3 refLight = diffuse + specular;

    // 教材通りのもの
    const float3 ligColor = ambientLight.lightColor.xyz + refLight;
    albedoColor.xyz *= ligColor;

    // 品質重視のもの
    // const float3 finalColor = albedoColor.xyz * refLight + ambientLight.lightColor.xyz;
    // albedoColor.xyz = finalColor;

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