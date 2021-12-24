TextureCube skyMap : register(t4);
SamplerState skyMapSampler : register(s1);

struct vs_out
{
    float4 pos : SV_Position;
    float3 samplePos : POSITION;
};


float4 main(vs_out input) : SV_TARGET
{
    return float4(skyMap.Sample(skyMapSampler, input.samplePos).xyz, 1.0f);
}