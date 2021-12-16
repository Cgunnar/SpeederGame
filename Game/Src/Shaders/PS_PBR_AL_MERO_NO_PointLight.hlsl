
Texture2D albedoTexture : register(t0);
Texture2D metallicRoughnessTexture : register(t1);
Texture2D normalMap : register(t2);

SamplerState mySampler : register(s0);





cbuffer PointLight : register(b0)
{
    float3 lightPosition;
    float lightStrength;
    float3 lightColor;
    float constantAttenuation;
    float LinearAttenuation;
    float exponentialAttenuation;
};

cbuffer VP : register(b1)
{
    matrix viewMatrix;
    matrix projectionMatrix;
};

//cbuffer Material : register(b2)
//{
//    float4 materialAmbientColor;
//    float3 materialDiffuseColor;
//    float opacity;
//    float3 materialSpeculaColor;
//    float materialShininess;
//};

struct vs_out
{
    float4 position_clip : SV_POSITION;
    float4 position_world : POSITION_WORLD;
    float4 normal_world : NORMAL_WORLD;
    float2 textureUV : TEXCOORD;
    float3 tangent_world : TANGENT;
    float3 biTangent_world : BITANGENT;
};

float Attenuate(float attConst, float attLin, float attExp, float distance)
{
    return 1.0f / (attConst + attLin * distance + attExp * (distance * distance));
}

float3 NormalMap(float3 valueFromNomalMap ,float3 tangent, float3 biTangent, float3 normal)
{
    float3 normalTanSpace = 2 * valueFromNomalMap - float3(1, 1, 1);
    normalTanSpace = normalize(normalTanSpace);
    float3 T = normalize(tangent);
    float3 B = normalize(biTangent);
    float3 N = normalize(normal);
    float3x3 TBN = float3x3(T, B, N);
    TBN = transpose(TBN); //so i can use mul the right way :/
    return normalize(mul(TBN, normalTanSpace));
}


float4 main(vs_out input) : SV_TARGET
{
    float3 albedo = albedoTexture.Sample(mySampler, input.textureUV).xyz;
    
    float3 ia = 0.2 * float3(1, 1, 1);
    float3 id = lightColor;
    float3 is = lightColor;

    
    float3 normalTanSpace = normalMap.Sample(mySampler, input.textureUV).xyz;
    float3 normal = NormalMap(normalTanSpace, input.tangent_world, input.biTangent_world, input.normal_world.xyz);
    
    
    float3 finalColor = float3(albedo);
    return float4(finalColor, 1.0f);
    return float4(normal, 1.0f);
}
