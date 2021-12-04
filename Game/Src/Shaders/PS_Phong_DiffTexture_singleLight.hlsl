

Texture2D colorTexture : register(t0);
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

cbuffer Material : register(b2)
{
    float4 materialAmbientColor;
    float4 materialDiffuseColor;
    float3 materialSpeculaColor;
    float materialShininess;
};

struct vs_out
{
    float4 position_clip : SV_POSITION;
    float4 position_world : POSITION_WORLD;
    float4 normal_world : NORMAL_WORLD;
    float2 textureUV : TEXCOORD;
};

float Attenuate(float attConst, float attLin, float attExp, float distance)
{
    return 1.0f / (attConst + attLin * distance + attExp * (distance * distance));
}


float4 main(vs_out input) : SV_TARGET
{
    float4 textureColor = colorTexture.Sample(mySampler, input.textureUV);
    
    float3 ka = textureColor.xyz;
    float3 kd = textureColor.xyz;
    float3 ks = materialSpeculaColor;
    float shininess = materialShininess;
    
    float3 ia = 0.2 * float3(1, 1, 1);
    float3 id = lightColor;
    float3 is = float3(1, 1, 1);
    
    float3 la = float3(0, 0, 0);
    float3 ld = float3(0, 0, 0);
    float3 ls = float3(0, 0, 0);
    
    la = ka * ia;
    
    float3 dirToLight = normalize(lightPosition - input.position_world.xyz);
    float3 N = normalize(input.normal_world.xyz);
    
    float diffuseFactor = dot(dirToLight, N);
    
    if(diffuseFactor > 0)
    {
        ld = kd * id * diffuseFactor;
        
        float3 R = normalize(reflect(-dirToLight, N));
        float3 cameraPos = -float3(viewMatrix[0][3], viewMatrix[1][3], viewMatrix[2][3]);
        float3 V = normalize(cameraPos - input.position_world.xyz);
        ls = ks * is * pow(saturate(dot(R, V)), shininess);
    }
    
    float3 finalColor = la + (ld + ls) * Attenuate(constantAttenuation, LinearAttenuation, exponentialAttenuation, length(input.position_world.xyz - lightPosition));
    return float4(finalColor , 1.0f);
}