/**
 * @file dof.fx
 * @brief 被写界深度用のシェーダー 
 */



cbuffer DofCB : register(b1)
{
    // ピントが合う距離
    float focusDistance;
    // ピントが合う幅
    float focusRange;
    // 4バイトアラインメントのためのパディング
    float padding[2];
};


#include "BloomVSCommon.hlsli"


Texture2D<float4> g_mainTexture : register(t0);
Texture2D<float4> g_bokeTexture : register(t1);
sampler g_sampler : register(s0);


float4 PSMain(PSInput In) : SV_Target0
{
    float4 sharp = g_mainTexture.Sample(g_sampler, In.uv);
    float3 boke = g_bokeTexture.Sample(g_sampler, In.uv).xyz;

    // ピントから離れるほど1に近づく
    float rate = saturate(abs(sharp.w - focusDistance) / focusRange); 
    return float4(lerp(sharp.xyz, boke, rate), 1.0f);
}