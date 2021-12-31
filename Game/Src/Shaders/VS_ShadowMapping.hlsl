
cbuffer Transforms : register(b0)
{
    matrix worldMatrix;
};


float4 main(float3 position_local : POSITION) : SV_POSITION
{
    return mul(worldMatrix, float4(position_local, 1.0f));
}