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
    float3 worldPos : TEXCOORD1; // World-space position  (for specular later).
    float4 posInLVP : TEXCOORD2; // Light View Projection space position.
};


/////////////////////////////////////////////////
// Pixel shader output (G-Buffer).
/////////////////////////////////////////////////
struct SPSOut
{
    float4 albedo : SV_Target0;   // Albedo (base color).
    float4 normal : SV_Target1;   // Normal (world-space, packed to 0~1).
    float3 worldPos : SV_Target2; // World-space position.
};


///////////////////////////////////////
// Common vertex shader code.
// Provides: ModelCb(b0: mWorld/mView/mProj), SVSIn, bone matrices (t3),
//           and the entry points VSMain / VSMainSkin / VSMainInstancing, etc.
///////////////////////////////////////
#include "ModelVSCommon.hlsli"

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
    psIn.worldPos = psIn.pos;

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
    psOut.normal.xyz = normalize(psIn.normal * 0.5f + 0.5f);
    psOut.normal.w = 1.0f;

    // ワールド座標:R32G32B32A32_FLOATなので、そのまま
    psOut.worldPos = psIn.worldPos;

    return psOut;
}