TextureCube skyMap : register(t4);
SamplerState skyMapSampler : register(s1);

struct vs_out
{
    float4 pos : SV_Position;
    float3 samplePos : POSITION;
};


float4 main(vs_out input) : SV_TARGET
{
    float3 sky = skyMap.Sample(skyMapSampler, input.samplePos).xyz;
    return float4(sky, 1.0f);
}