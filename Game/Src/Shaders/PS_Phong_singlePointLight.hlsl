
#ifdef DIFFUSE_TEXTURE
    Texture2D colorTexture : register(t0);
#endif

#ifdef SPECULAR_MAP
    Texture2D specTex : register(t1);
#endif

#ifdef NORMAL_MAP
    Texture2D normalMap : register(t2);
#endif

//#if  defined(DIFFUSE_TEXTURE) || defined(SPECULAR_MAP) || defined(NORMAL_MAP)
//    SamplerState mySampler : register(s0);
//#endif

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
    float3 materialDiffuseColor;
    float opacity;
    float3 materialSpeculaColor;
    float materialShininess;
};

#ifdef NORMAL_MAP
struct vs_out
{
    float4 position_clip : SV_POSITION;
    float4 position_world : POSITION_WORLD;
    float4 normal_world : NORMAL_WORLD;
    float2 textureUV : TEXCOORD;
    float3 tangent_world : TANGENT;
    float3 biTangent_world : BITANGENT;
};
#else
struct vs_out
{
    float4 position_clip : SV_POSITION;
    float4 position_world : POSITION_WORLD;
    float4 normal_world : NORMAL_WORLD;
    float2 textureUV : TEXCOORD;
};
#endif

float Attenuate(float attConst, float attLin, float attExp, float distance)
{
    return 1.0f / (attConst + attLin * distance + attExp * (distance * distance));
}


float4 main(vs_out input) : SV_TARGET
{   
    //set material colors
    float3 ka = materialAmbientColor.xyz;
    float3 kd = materialDiffuseColor;
    float3 ks = materialSpeculaColor;
    float shininess = materialShininess;
    float alpha = opacity;
    
#ifdef DIFFUSE_TEXTURE
    float4 texColor = colorTexture.Sample(mySampler, input.textureUV);
    ka = texColor.xyz;
    kd = texColor.xyz;
    alpha = texColor.w;
#endif
    
    float3 ia = 0.2 * float3(1, 1, 1);
    float3 id = lightColor;
    float3 is = lightColor;
    
    float3 la = float3(0, 0, 0);
    float3 ld = float3(0, 0, 0);
    float3 ls = float3(0, 0, 0);
    
    float3 normal = normalize(input.normal_world.xyz);
    
#ifdef NORMAL_MAP
    float3 normalTanSpace = /*normalize*/(normalMap.Sample(mySampler, input.textureUV).xyz); //normalization fucks up sponza and brickwall but fixes crysis
    normalTanSpace = 2 * normalTanSpace - float3(1, 1, 1);
    normalTanSpace = normalize(normalTanSpace);
    
    float3 T = normalize(input.tangent_world);
    float3 B = normalize(input.biTangent_world);
    float3 N = normal;
    
    float3x3 TBN = float3x3(T, B, N);
    TBN = transpose(TBN); //so i can use mul the right way :/
    normal = normalize(mul(TBN, normalTanSpace));
    
#endif
    
    
    float3 dirToLight = normalize(lightPosition - input.position_world.xyz);
    float3 cameraPos = -float3(viewMatrix[0][3], viewMatrix[1][3], viewMatrix[2][3]);
    float3 V = normalize(cameraPos - input.position_world.xyz);
    float3 R = normalize(reflect(-dirToLight, normal));
    
    float att = Attenuate(constantAttenuation, LinearAttenuation, exponentialAttenuation, length(input.position_world.xyz - lightPosition));
    
    la = ka * ia;
    ld = kd * id * saturate(dot(dirToLight, normal));
    ls = ks * is * pow(saturate(dot(R, V)), shininess);
    
    float3 finalColor = la + (ld + ls) * att * lightStrength;
    return float4(finalColor, 1.0f);
    return float4(normal, 1.0f);
}
