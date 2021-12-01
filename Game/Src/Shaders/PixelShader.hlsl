
cbuffer ColorCB : register(b0)
{
    float3 color;
};

struct vs_out
{
    float4 position_clip : SV_POSITION;
    float4 position_world : POSITION_WORLD;
    float4 normal_world : NORMAL_WORLD;
    float2 textureUV : TEXCOORD;
};

float4 main(vs_out input) : SV_TARGET
{
    
	return float4(color, 1.0f);
}