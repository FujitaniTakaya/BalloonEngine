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
    float pad1;
    float3 lightColor;
    float pad2;
};


/*!
 * @brief   Ambient light data.
 */
struct AmbientLight
{
    float3 lightColor;
    float pad;
};


/*!
 * @brief   Point light data.
 */
struct PointLight
{
    float3 position;
    float range;
    float3 lightColor;
    float pad;
};

/** ポイントライトの最大数 */
static const int MAX_POINT_LIGHT_NUM = 4;

/*!
 * @brief   Constant buffer for lighting data.
 */
cbuffer LightCb : register(b1)
{
    DirectionLight dirLight;
    AmbientLight ambientLight;
    int usingPointLightNum;
    float3 pad1;
    PointLight pointLights[MAX_POINT_LIGHT_NUM];
    float3 eyePos;
    float pad2;
    float shininess;
    float localBias;
    float2 pad3;
    float4x4 mLVP;
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


///////////////////////////////////////////////////
// Normal map to world space normal conversion.
////////////////////////////////////////////////////
float3 CalcNormalFromNormalMap(
    const float3 tangent,
    const float3 biNormal,
    const float3 normal,
    const float3 localNormal
    )
{
    const float3 convertedNormal = (localNormal * 2.0f) - 1.0f; // Convert from [0,1] to [-1,1]
    
    return tangent * convertedNormal.x + biNormal * convertedNormal.y + normal * convertedNormal.z;
}

#endif // LIGHTING_H_INCLUDED