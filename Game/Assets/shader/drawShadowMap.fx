#include "color.hlsli"


// モデル用の定数バッファー
cbuffer ModelCb : register(b0)
{
    float4x4 mWorld;
    float4x4 mView;
    float4x4 mProj;
};

////////////////////////////////////////////////
// 構造体
////////////////////////////////////////////////

// 頂点シェーダーへの入力
struct SVSIn
{
    float4 pos : POSITION;          //頂点座標。
};


// 頂点シェーダーへの入力
struct SPSIn
{
    float4 pos      : SV_POSITION;  // Clip-space position.
    float4 posInLVP : TEXCOORD2;    // Light View Projection space position.
};





/**
 * @brief 頂点シェーダーのメイン関数
 * @param vsIn 頂点シェーダーへの入力
 * @return ピクセルシェーダーへの出力
 */
SPSIn VSMain(SVSIn vsIn)
{
    // mul()は、第一引数に行列を入れる。でないと、行列の転置が必要になるので注意。
    SPSIn psIn;
    psIn.pos = mul(mWorld, vsIn.pos);
    psIn.pos = mul(mView, psIn.pos);
    psIn.pos = mul(mProj, psIn.pos);
    psIn.posInLVP = psIn.pos;
    return psIn;
}




/**
 * @brief ピクセルシェーダーのメイン関数
 * @param psIn ピクセルシェーダーへの入力
 * @return 出力カラー
 */
float4 PSMain(SPSIn psIn) : SV_Target0
{
    // 深度(ライトから近いほど小さい値)を「色」として書き込む
    return psIn.posInLVP.z / psIn.posInLVP.w;
}