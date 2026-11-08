struct Vin
{
    float3 pos : POSITION;
    float2 uv : TEXCOORD;
};
struct Vout
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
};
Vout VSMain(Vin i)
{
    Vout o;
    o.pos = float4(i.pos, 1.0f);
    o.uv = i.uv;
    return o;
}
Texture2D tex0 : register(t0);
SamplerState samp0 : register(s0);
float4 PSMain(Vout i) : SV_TARGET
{
    return tex0.Sample(samp0, i.uv);
}