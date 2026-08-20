/**
 * @file    samplingLuminance.fx
 * @brief   Shader for sampling luminance for bloom effect.
 */


#include "BloomVSCommon.hlsli"


/**
 * @brief   Constant buffer for bloom effect.
 */
cbuffer BloomCB : register(b1)
{
    float threshold;
    float padding[3]; // Padding to align to 16 bytes
};


Texture2D<float4> g_mainTexture : register(t0);
sampler g_sampler : register(s0);


float4 PSMain(PSInput In) : SV_Target0
{
    float4 color = g_mainTexture.Sample(g_sampler, In.uv);
    float t = dot(color.xyz, float3(0.2125f, 0.7154f, 0.0721f));
    clip(t - threshold);
    return color;
}
