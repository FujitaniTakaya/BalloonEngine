/**
 * @file   dualBlurUp.fx
 */


#include "BloomVSCommon.hlsli"


Texture2D<float4> srcTexture : register(t0);
sampler g_sampler : register(s0);


float4 PSMain(PSInput In) : SV_Target0
{
    float2 uv = In.uv;
    float2 size;
    srcTexture.GetDimensions(size.x, size.y);
    float2 o = 0.5f / size;

    // --- アップサンプル(8タップ) : 菱形に広げながら拡大 ---
    float4 sum = 0;
    sum += srcTexture.Sample(g_sampler, uv + float2(-o.x * 2.0f, 0));
    sum += srcTexture.Sample(g_sampler, uv + float2(-o.x, o.y)) * 2.0f;
    sum += srcTexture.Sample(g_sampler, uv + float2(0, o.y * 2.0f));
    sum += srcTexture.Sample(g_sampler, uv + float2(o.x, o.y)) * 2.0f;
    sum += srcTexture.Sample(g_sampler, uv + float2(o.x * 2.0f, 0));
    sum += srcTexture.Sample(g_sampler, uv + float2(o.x, -o.y)) * 2.0f;
    sum += srcTexture.Sample(g_sampler, uv + float2(0, -o.y * 2.0f));
    sum += srcTexture.Sample(g_sampler, uv + float2(-o.x, -o.y)) * 2.0f;
    return sum / 12.0f;
}
