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
 * @brief   Spot light data.
 */
struct SpotLight
{
    PointLight pointLight;
    float3 direction;
    float spotAngle;
};

/** スポットライトの最大数 */
static const int MAX_SPOT_LIGHT_NUM = 4;

/** シャドウマップの最大数 */
static const int MAX_SHADOW_NUM = 2;

/*!
 * @brief   Constant buffer for lighting data.
 */
cbuffer LightCb : register(b1)
{
    DirectionLight dirLight;
    AmbientLight ambientLight;
    int usingPointLightNum;
    int usingSpotLightNum;
    float2 pad1;
    PointLight pointLights[MAX_POINT_LIGHT_NUM];
    SpotLight spotLights[MAX_SPOT_LIGHT_NUM];
    float3 eyePos;
    float pad2;
    float shininess;
    float localBias;
    float2 pad3;
    float4x4 mLVP[MAX_SHADOW_NUM];
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
    const float3 normedViewDir,
    const float3 lightColor,
    const float shininess
    )
{
    const float3 R = reflect(normedLightDir, normedNormal);
    const float RdotV = max(dot(R, normedViewDir), 0.0f);
    const float NdotL = max(dot(normedNormal, normedLightDir) * -1, 0.0f);
    const float specular = pow(RdotV, shininess) * step(0.0001f, NdotL);
    return lightColor * specular;
}


////////////////////////////////////////////////
// Point light lighting (diffuse + specular, distance attenuation).
////////////////////////////////////////////////
float3 CalcPointLightLighting(
    const float3 N,
    const float3 V,
    const float3 worldPos,
    const PointLight pointLight,
    const float shininess,
    const float specFactor
    )
{
    const float3 ligDir = normalize(worldPos - pointLight.position);
    const float dist = length(worldPos - pointLight.position);
    const float affect = pow(saturate(1.0f - (dist / pointLight.range)), 3.0f);

    const float3 diffuse = CalcDiffuseLighting(N, ligDir, pointLight.lightColor) * affect;
    const float3 specular = CalcSpecularLighting(N, ligDir, V, pointLight.lightColor, shininess) * affect * specFactor;

    return diffuse + specular;
}

////////////////////////////////////////////////
// Spot light lighting (diffuse + specular, distance + angle attenuation).
////////////////////////////////////////////////
float3 CalcSpotLightLighting(
    const float3 N,
    const float3 V,
    const float3 worldPos,
    const SpotLight spotLight,
    const float shininess,
    const float specFactor
    )
{
    const float3 ligDir = normalize(worldPos - spotLight.pointLight.position);
    const float dist = length(worldPos - spotLight.pointLight.position);
    const float3 spotDir = normalize(spotLight.direction);
    const float angle = abs(acos(clamp(dot(ligDir, spotDir), -1.0f, 1.0f)));

    const float distAffect = pow(saturate(1.0f - (dist / spotLight.pointLight.range)), 3.0f);
    const float angleAffect = pow(saturate(1.0f - (angle / spotLight.spotAngle)), 3.0f);
    const float affect = distAffect * angleAffect;

    const float3 diffuse = CalcDiffuseLighting(N, ligDir, spotLight.pointLight.lightColor) * affect;
    const float3 specular = CalcSpecularLighting(N, ligDir, V, spotLight.pointLight.lightColor, shininess) * affect * specFactor;

    return diffuse + specular;
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