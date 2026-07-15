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
Texture2D<float4> albedoTexture : register(t0);
sampler Sampler : register(s0);

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
// Lambert diffuse lighting calculation.
////////////////////////////////////////////////
float3 CalcDiffuseLighting(const float3 normedNormal, const float3 normedLightDir, const float3 lightColor)
{
    // 内積を計算し、正負を反転(向きが逆なほど明るいため)、0未満は0にする(当たってない部分は暗くする)
    const float NdotL = max(dot(normedNormal, normedLightDir) * -1, 0.0f);
    // ライトカラーに拡散反射光を掛けて返す
    return lightColor * NdotL;
}


////////////////////////////////////////////////
// Specular lighting calculation (Phong model).
////////////////////////////////////////////////
float3 CalcSpecularLighting(const float3 normedNormal, const float3 normedLightDir, const float3 viewPos, const float3 worldPos, const float3 lightColor, const float shininess)
{
    // 法線と
    // 反射ベクトルを計算
    const float3 R = reflect(normedLightDir, normedNormal);
    // 視線ベクトルを計算
    const float3 V = normalize(viewPos - worldPos);
    // 反射ベクトルと視線ベクトルの内積を計算し、正負を反転(向きが逆なほど明るいため)、0未満は0にする(当たってない部分は暗くする)
    const float RdotV = max(dot(R, V) * -1, 0.0f);
    // 鏡面反射光を計算
    const float NdotL = max(dot(normedNormal, -normedLightDir), 0.0f);
    const float specular = pow(RdotV, shininess) * step(0.0001f, NdotL);
    return lightColor * specular;
}


////////////////////////////////////////////////
// Pixel shader.
// For now: just output the albedo texture. Add your lighting here.
////////////////////////////////////////////////
float4 PSMain(SPSIn In) : SV_Target0
{
    float4 albedoColor = albedoTexture.Sample(Sampler, In.uv);

    // ライトの方向と法線を正規化
    const float3 L = normalize(dirLight.lightDir);
    const float3 N = normalize(In.normal);


    // 拡散反射光を計算
    const float3 diffuse = CalcDiffuseLighting(N, L, dirLight.lightColor.xyz);

    // 鏡面反射光を計算
    const float3 specular = CalcSpecularLighting(N, L, eyePos, In.worldPos, dirLight.lightColor.xyz, 32.0f);

    // 反射光を合成
    const float3 refLight = diffuse + specular;


    // 教材通りのもの
    const float3 ligColor = ambientColor.xyz + refLight;
    albedoColor.xyz *= ligColor;

    // 品質重視のもの
    // const float3 finalColor = albedoColor.xyz * refLight + ambientColor.xyz;
    // albedoColor.xyz = finalColor;

    return albedoColor;
}
