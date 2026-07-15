/*!
 * @brief   ライティング共通処理
 */
#ifndef LIGHTING_H_INCLUDED
#define LIGHTING_H_INCLUDED

/*!
 * @brief   Directional light data.
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
// Lambert diffuse lighting calculation.
////////////////////////////////////////////////
float3 CalcDiffuseLighting(
    const float3 normedNormal,
    const float3 normedLightDir,
    const float3 lightColor
    )
{
    const float NdotL = max(dot(normedNormal, normedLightDir) * -1, 0.0f);
    return lightColor * NdotL;
}

////////////////////////////////////////////////
// Specular lighting calculation (Phong model).
////////////////////////////////////////////////
float3 CalcSpecularLighting(
    const float3 normedNormal,
    const float3 normedLightDir,
    const float3 viewPos,
    const float3 worldPos,
    const float3 lightColor,
    const float shininess
    )
{
    const float3 R = reflect(normedLightDir, normedNormal);
    const float3 V = normalize(viewPos - worldPos);
    const float RdotV = max(dot(R, V), 0.0f);
    const float NdotL = max(dot(normedNormal, normedLightDir) * -1, 0.0f);
    const float specular = pow(RdotV, shininess) * step(0.0001f, NdotL);
    return lightColor * specular;
}

#endif // LIGHTING_H_INCLUDED