/**
 * @file    BloomVSCommon.hlsli
 * @brief   ブルーム用のシェーダーで共通して使用する頂点シェーダー関連の定義。
 */


cbuffer cb : register(b0)
{
    float4x4 mvp;       // モデルビュープロジェクション行列。
    float4 mulColor;    // 乗算カラー。
};


struct VSInput
{
    float4 pos : POSITION;
    float2 uv  : TEXCOORD0;
};

struct PSInput
{
    float4 pos : SV_POSITION;
    float2 uv  : TEXCOORD0;
};


PSInput VSMain(VSInput In)
{
    PSInput psIn;
    psIn.pos = mul(mvp, In.pos);
    psIn.uv = In.uv;
    return psIn;
}
