/*!
 * @brief   シャドウ(影)共通処理
 *
 * フォワード(model.fx)とデファード(deferredLighting.fx)の両方から影を
 * 計算するため、シャドウマップのリソース宣言と CalcShadow() をここにまとめる。
 *
 * シャドウマップは MAX_SHADOW_NUM 枚。ディレクションライト i が
 * シャドウマップ i(mLVP[i] / g_shadowMap[i])に影を1枚落とす。
 *
 * 【前提】このファイルより前に common/Lighting.hlsli を include しておくこと。
 *         LightingCB(mLVP / localBias / MAX_SHADOW_NUM)に依存する。
 *         common/ 配下から兄弟ファイルを #include すると、実行時シェーダー
 *         コンパイラ(D3D_COMPILE_STANDARD_FILE_INCLUDE)がパスを解決できず
 *         X1507 になるため、ModelVSCommon.hlsli と同様に include 側へ委ねる。
 */
#ifndef SHADOW_H_INCLUDED
#define SHADOW_H_INCLUDED

////////////////////////////////////////////////
// シャドウマップのリソース(t10 ～ t10+MAX_SHADOW_NUM-1)。
// t10 は VS 側の g_worldMatrixArray(インスタンシング用)とレジスタが重なるが、
// 影を参照するのは PS 側だけなので、コンパイル後のバインドは衝突しない。
////////////////////////////////////////////////
Texture2D<float4> g_shadowMap[MAX_SHADOW_NUM] : register(t10);
SamplerComparisonState g_shadowMapSampler : register(s1);

////////////////////////////////////////////////
// 影(0=影なし、1=影)を計算。PCF + 傾斜依存バイアス。
//   worldPos    : ワールド座標
//   N           : 正規化済みワールド法線
//   L           : 正規化済みライト方向(model.fx と同じ規約。実際の入射方向は -L)
//   shadowIndex : シャドウマップ番号(= ディレクションライト番号)
//
// shadowIndex は呼び出し側の [unroll] 展開後にリテラルになる前提。
// (テクスチャ配列は動的 index できないため)
////////////////////////////////////////////////
float CalcShadow(const float3 worldPos, const float3 N, const float3 L, const int shadowIndex)
{
    const float4 posInLVP = mul(mLVP[shadowIndex], float4(worldPos, 1.0f));
    float2 shadowMapUV = posInLVP.xy / posInLVP.w;
    shadowMapUV = shadowMapUV * float2(0.5f, -0.5f) + 0.5f;
    const float zInLVP = posInLVP.z / posInLVP.w;

    if (shadowMapUV.x < 0.0f || shadowMapUV.x > 1.0f
     || shadowMapUV.y < 0.0f || shadowMapUV.y > 1.0f
     || zInLVP < 0.0f || zInLVP > 1.0f)
    {
        return 0.0f;
    }

    // 傾斜依存バイアス(これを使わないと、モデルに模様が出る)
    const float bias = max(localBias * (1.0f - dot(N, -L)), 0.0001f);

    return g_shadowMap[shadowIndex].SampleCmpLevelZero(g_shadowMapSampler, shadowMapUV, zInLVP - bias);
}

#endif // SHADOW_H_INCLUDED
