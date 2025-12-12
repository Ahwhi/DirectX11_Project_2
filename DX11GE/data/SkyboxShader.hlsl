cbuffer CB : register(b0) // row-major로 보낼 것(DirectXMath의 XMMatrixTranspose 사용)
{
    float4x4 gViewProjNoTrans;
};

struct VSIn
{
    float3 pos : POSITION;
};
struct VSOut
{
    float4 svpos : SV_POSITION;
    float3 dir : TEXCOORD0;
};

VSOut VSMain(VSIn v)
{
    VSOut o;
    o.dir = v.pos;

    float4 p = float4(v.pos, 1.0);
    o.svpos = mul(p, gViewProjNoTrans);

    // ★ 핵심: 하늘을 무한대(=far plane)로 밀어버림
    o.svpos.z = o.svpos.w;

    return o;
}

TextureCube gSky : register(t0);
SamplerState gSamp : register(s0);

float4 PSMain(VSOut i) : SV_TARGET
{
    return gSky.Sample(gSamp, normalize(i.dir));
}
