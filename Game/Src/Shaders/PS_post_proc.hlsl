
Texture2D renderTargetInput : register(t0);
SamplerState bilinearFilter : register(s5);

struct vs_out
{
    float4 position : SV_Position;
    float2 uv : TEXCOORD;
};

float4 main(vs_out input) : SV_TARGET
{
    float4 color = renderTargetInput.Sample(bilinearFilter, input.uv);
    
    color.rgb = color.rgb / (color.rgb + float3(1, 1, 1));
    return float4(color.rgb, 1.0f);
}