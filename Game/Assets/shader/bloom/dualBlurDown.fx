/**
 * @file   dualBlurDown.fx
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

    // --- ダウンサンプル(5タップ) : 中心を重く、四隅を混ぜながら縮小 ---
    float4 sum = srcTexture.Sample(g_sampler, uv) * 4.0f;
    sum += srcTexture.Sample(g_sampler, uv + float2(-o.x, -o.y));
    sum += srcTexture.Sample(g_sampler, uv + float2(o.x, -o.y));
    sum += srcTexture.Sample(g_sampler, uv + float2(-o.x, o.y));
    sum += srcTexture.Sample(g_sampler, uv + float2(o.x, o.y));
    return sum / 8.0f;
}
