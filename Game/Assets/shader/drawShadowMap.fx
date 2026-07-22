#include "color.hlsli"


// 頂点シェーダーへの入力
struct SPSIn
{
    float4 pos      : SV_POSITION;  // Clip-space position.
    float3 normal   : NORMAL;       // World-space normal.
    float3 tangent  : TANGENT;      // World-space tangent   (for normal mapping later).
    float3 biNormal : BINORMAL;     // World-space binormal  (for normal mapping later).
    float2 uv       : TEXCOORD0;    // UV.
    float3 worldPos : TEXCOORD1;    // World-space position  (for specular later).
};


#include "ModelVSCommon.hlsli"


////////////////////////////////////////////////
// Vertex shader core (called by the VSMain* entry points in ModelVSCommon.h).
////////////////////////////////////////////////
SPSIn VSMainCore(SVSIn vsIn, float4x4 mWorldLocal, uniform bool isUsePreComputedVertexBuffer)
{
    SPSIn psIn;
    psIn.pos = CalcVertexPositionInWorldSpace(vsIn.pos, mWorldLocal, isUsePreComputedVertexBuffer);
    psIn.pos = mul(mView, psIn.pos);
    psIn.pos = mul(mProj, psIn.pos);
    psIn.uv = vsIn.uv;
    psIn.normal = mul((float3x3)mWorldLocal, vsIn.normal);
    return psIn;
}




/**
 * @brief ピクセルシェーダーのメイン関数
 * @param psIn ピクセルシェーダーへの入力
 * @return 出力カラー
 */
float4 PSMain(SPSIn psIn) : SV_Target0
{
    return g_colorBlack;
}