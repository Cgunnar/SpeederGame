static const float3 CUBE[36] =
{
    float3(0.5f, -0.5f, -0.5f),
float3(-0.5f, -0.5f, -0.5f),
float3(-0.5f, 0.5f, -0.5f),

float3(-0.5f, 0.5f, -0.5f),
float3(0.5f, 0.5f, -0.5f),
float3(0.5f, -0.5f, -0.5f),

float3(-0.5f, -0.5f, 0.5f),
float3(0.5f, -0.5f, 0.5f),
float3(0.5f, 0.5f, 0.5f),

float3(0.5f, 0.5f, 0.5f),
float3(-0.5f, 0.5f, 0.5f),
float3(-0.5f, -0.5f, 0.5f),

float3(-0.5f, -0.5f, -0.5f),
float3(-0.5f, -0.5f, 0.5f),
float3(-0.5f, 0.5f, 0.5f),
    
float3(-0.5f, 0.5f, 0.5f),
float3(-0.5f, 0.5f, -0.5f),
float3(-0.5f, -0.5f, -0.5f),

float3(0.5f, -0.5f, 0.5f),
float3(0.5f, -0.5f, -0.5f),
float3(0.5f, 0.5f, -0.5f),

float3(0.5f, 0.5f, -0.5f),
float3(0.5f, 0.5f, 0.5f),
float3(0.5f, -0.5f, 0.5f),

float3(0.5f, 0.5f, -0.5f),
float3(-0.5f, 0.5f, -0.5f),
float3(-0.5f, 0.5f, 0.5f),

float3(-0.5f, 0.5f, 0.5f),
float3(0.5f, 0.5f, 0.5f),
float3(0.5f, 0.5f, -0.5f),

float3(0.5f, -0.5f, 0.5f),
float3(-0.5f, -0.5f, 0.5f),
float3(-0.5f, -0.5f, -0.5f),
    
float3(-0.5f, -0.5f, -0.5f),
float3(0.5f, -0.5f, -0.5f),
float3(0.5f, -0.5f, 0.5f)
};

cbuffer Transforms : register(b0)
{
    matrix worldMatrix;
};

cbuffer VP : register(b1)
{
    matrix viewMatrix;
    matrix projectionMatrix;
};

struct vs_out
{
    float4 pos : SV_Position;
    float3 samplePos : POSITION;
};

vs_out main(uint vID : SV_VertexID)
{
    vs_out output;
    
    output.samplePos = mul(worldMatrix, float4(normalize(CUBE[vID]), 0.f)).xyz;

    output.pos = mul(viewMatrix, float4(CUBE[vID], 0.f));
    output.pos = mul(projectionMatrix, output.pos);
    output.pos.z = output.pos.w; // Place with furthest distance

    return output;
}