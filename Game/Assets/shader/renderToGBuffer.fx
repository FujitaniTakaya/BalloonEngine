////////////////////////////////////////////////
// Pixel shader input.
////////////////////////////////////////////////
struct SPSIn
{
    float4 pos : SV_POSITION;    // Clip-space position.
    float3 normal : NORMAL;      // World-space normal.
    float3 tangent : TANGENT;    // World-space tangent   (for normal mapping later).
    float3 biNormal : BINORMAL;  // World-space binormal  (for normal mapping later).
    float2 uv : TEXCOORD0;       // UV.
    float4 posInLVP : TEXCOORD1; // Light View Projection space position.
};


/////////////////////////////////////////////////
// Pixel shader output (G-Buffer).
/////////////////////////////////////////////////
struct SPSOut
{
    float4 albedo : SV_Target0;   // Albedo (base color).
    float4 normal : SV_Target1;   // Normal (world-space, packed to 0~1).
    float depth : SV_Target2; // Depth.
};


///////////////////////////////////////
// Common vertex shader code.
// Provides: ModelCb(b0: mWorld/mView/mProj), SVSIn, bone matrices (t3),
//           and the entry points VSMain / VSMainSkin / VSMainInstancing, etc.
///////////////////////////////////////
#include "common/ModelVSCommon.hlsli"

///////////////////////////////////////
// Shader resources.
// The tkm material binds the albedo texture to t0.
// (t1 = normal map, t2 = metallic/smooth — you can add them when you need them.)
///////////////////////////////////////
Texture2D<float4> g_albedoTexture : register(t0);
Texture2D<float4> g_normalTexture : register(t1);
Texture2D<float4> g_specularTexture : register(t2);
sampler g_sampler : register(s0);


////////////////////////////////////////////////
// Vertex shader core (called by the VSMain* entry points in ModelVSCommon.h).
////////////////////////////////////////////////
SPSIn VSMainCore(SVSIn vsIn, float4x4 mWorldLocal, uniform bool isUsePreComputedVertexBuffer)
{
    SPSIn psIn;

    // Local space -> world space.
    psIn.pos = CalcVertexPositionInWorldSpace(vsIn.pos, mWorldLocal, isUsePreComputedVertexBuffer);

    // World -> view -> projection (clip) space.
    psIn.pos = mul(mView, psIn.pos);
    psIn.pos = mul(mProj, psIn.pos);

    // World-space normal / tangent / binormal.
    CalcVertexNormalTangentBiNormalInWorldSpace(
        psIn.normal,
        psIn.tangent,
        psIn.biNormal,
        mWorldLocal,
        vsIn.normal,
        vsIn.tangent,
        vsIn.biNormal,
        isUsePreComputedVertexBuffer
    );

    psIn.uv = vsIn.uv;
    return psIn;
}



/////////////////////////////////////////////////
// Pixel shader
/////////////////////////////////////////////////
SPSOut PSMain(SPSIn psIn)
{
    SPSOut psOut;
    // アルベド:テクスチャの色をそのまま
    psOut.albedo = g_albedoTexture.Sample(g_sampler, psIn.uv);

    // 法線:(-1 ~ +1)のままでは色として保存できないので(0 ~ 1)に変換
    // NOTE: 正規化してから *0.5+0.5 する。順序を逆にすると向きが歪む。
    psOut.normal.xyz = normalize(psIn.normal) * 0.5f + 0.5f;
    psOut.normal.w = 1.0f;

    // 深度:射影空間の深度値(NDC z、0~1)。
    // psIn.pos は SV_POSITION なので、この .z は既に w 除算済みの深度バッファ値。
    // (もう一度 .w で割らないこと。ワールド座標はライティングパスで mViewProjInv から復元する)
    psOut.depth = psIn.pos.z;

    return psOut;
}