

Texture2D colorTexture : register(t0);
SamplerState mySampler : register(s0);

struct vs_out
{
    float4 position_clip : SV_POSITION;
    float4 position_world : POSITION_WORLD;
    float4 normal_world : NORMAL_WORLD;
    float2 textureUV : TEXCOORD;
};

float4 main(vs_out input) : SV_TARGET
{
    float3 texColor = colorTexture.Sample(mySampler, input.textureUV).xyz;
    return float4(texColor, 1.0f);
}