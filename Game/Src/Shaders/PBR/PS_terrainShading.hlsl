#define NO_METALLIC_ROUGHNESS_TEXTURE
#define TERRAINSHADING

float3 TerrainShader(float3 position, float3 normal, float3 albedo)
{
   //terrain shading, move to seperate shader
    float blendAmount;
    float3 c = albedo;
    float slope = 1.0 - normal.y;
    float vegH = 1.0 - clamp(position.y / 250.0, 0.0, 1.0);
    float3 grassColor = vegH * float3(0, 0.02, 0);
    float3 slopeColor = lerp(albedo, 0.4 * albedo + 0.6 * grassColor, vegH);
    if (slope < 0.17)
    {
        blendAmount = slope / 0.2f;
        c = lerp(grassColor, slopeColor, blendAmount);
    }
    else if ((slope < 0.4) && (slope >= 0.17f))
    {
        blendAmount = (slope - 0.2f) * (1.0f / (0.7f - 0.2f));
        
        c = lerp(slopeColor, albedo, blendAmount);
    }
    return c;
}
#include "PS_PBR.hlsl"