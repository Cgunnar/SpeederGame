


cbuffer Transforms : register(b0)
{
    matrix worldMatrix;
};

cbuffer VP : register(b1)
{
    matrix viewMatrix;
    matrix projectionMatrix;
};

float4 main( float4 pos : POSITION ) : SV_POSITION
{
	return pos;
}