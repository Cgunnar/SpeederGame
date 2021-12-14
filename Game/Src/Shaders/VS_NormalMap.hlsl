

cbuffer Transforms : register(b0)
{
    matrix worldMatrix;
};

cbuffer VP : register(b1)
{
    matrix viewMatrix;
    matrix projectionMatrix;
};

struct vs_in
{
    float3 position_local : POSITION;
    float3 normal_local : NORMAL;
    float2 textureUV : TEXCOORD;
    float3 tangent_local : TANGENT;
    float3 biTangent_local : BITANGENT;
};

struct vs_out
{
    float4 position_clip : SV_POSITION;
    float4 position_world : POSITION_WORLD;
    float4 normal_world : NORMAL_WORLD;
    float2 textureUV : TEXCOORD;
    float3 tangent_world : TANGENT;
    float3 biTangent_world : BITANGENT;
};

vs_out main(vs_in input)
{
    vs_out output = (vs_out) 0;
    output.textureUV = input.textureUV;
    output.position_world = float4(input.position_local, 1.0f);
    output.position_clip = float4(input.position_local, 1.0f);
    output.normal_world = float4(input.normal_local, 0.0f);

    output.position_world = mul(worldMatrix, output.position_world);
    output.normal_world = normalize(mul(worldMatrix, output.normal_world));
    output.tangent_world = normalize(mul(worldMatrix, float4(input.tangent_local, 0)).xyz);
    output.biTangent_world = normalize(mul(worldMatrix, float4(input.biTangent_local, 0)).xyz);

    matrix MVP;
    MVP = mul(viewMatrix, worldMatrix);
    MVP = mul(projectionMatrix, MVP);

    output.position_clip = mul(MVP, output.position_clip);
    return output;
}