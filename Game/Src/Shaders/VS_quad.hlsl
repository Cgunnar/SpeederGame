struct vs_out
{
    float4 position : SV_Position;
    float2 uv : TEXCOORD;
};

vs_out main(uint id : SV_VertexID)
{
    vs_out output;
    output.uv = float2((id << 1) & 2, id & 2);
    output.position = float4(output.uv * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f), 0.0f, 1.0f);
    return output;
}