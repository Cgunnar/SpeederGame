
Texture2D albedoTexture : register(t0);
Texture2D metallicRoughnessTexture : register(t1);
Texture2D normalMap : register(t2);
Texture2D emissiveTexture : register(t3);
Texture2D splitSumBrdfLookUpMap : register(t4);
TextureCube skyMap : register(t5);
TextureCube skyIrrMap : register(t6);

SamplerState mySampler : register(s0);
SamplerState skyMapSampler : register(s1);
SamplerState splitSumLookUpSampler : register(s2);





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

cbuffer pbrMats : register(b2)
{
    float4 albedoFactor;
    float3 emissiveFactor;
    float metallicFactor;
    float roughnessFactor;
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

float3 NormalMap(float3 valueFromNomalMap, float3 tangent, float3 biTangent, float3 normal)
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

//ggxtr
//float NDF(float3 normal, float3 halfwayVec, float roughness)
//{
//    roughness *= roughness; //square
//    float d = saturate(dot(normal, halfwayVec));
//    d *= d;
//    d *= (roughness - 1);
//    d += 1;
//    d *= d;
//    d *= 3.1415;
//    return roughness / d;
//}

//lol i just copy pasted this from learn opengl
float DistributionGGX(float3 N, float3 H, float a)
{
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
	
    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = 3.1415 * denom * denom;
	
    return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float k)
{
    float nom = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return nom / denom;
}
  
float GeometrySmith(float3 N, float3 V, float3 L, float k)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx1 = GeometrySchlickGGX(NdotV, k);
    float ggx2 = GeometrySchlickGGX(NdotL, k);
	
    return ggx1 * ggx2;
}

float3 fresnelSchlick(float cosTheta, float3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

float3 fresnelSchlickRoughness(float cosTheta, float3 F0, float roughness)
{
    return F0 + (max(float3(1.0 - roughness, 1.0 - roughness, 1.0 - roughness), F0) - F0) * pow(saturate(1.0 - cosTheta), 5.0);
}


float4 main(vs_out input) : SV_TARGET
{
    float3 albedo = albedoFactor.rgb;
    float alpha = albedoFactor.a;
    
#ifdef ALBEDO_TEXTURE
    float4 albedoTextureVal = albedoTexture.Sample(mySampler, input.textureUV);
    albedo *= albedoTextureVal.rgb;
    alpha *= albedoTextureVal.a;
#endif
    
    float4 metallicRoughnessTextureVal = metallicRoughnessTexture.Sample(mySampler, input.textureUV);
    
    float ambientOcclusion = metallicRoughnessTextureVal.r;
    float metallic = metallicRoughnessTextureVal.b * metallicFactor;
    float roughness = metallicRoughnessTextureVal.g * roughnessFactor;
    roughness = max(roughness, 0.05);
    
    float3 emissive = emissiveFactor;
#ifdef EMISSIVE
    emissive *= emissiveTexture.Sample(mySampler, input.textureUV).xyz;
#endif
    
    float3 normal = normalize(input.normal_world.xyz);
    
#ifdef NORMAL_MAP
    float3 normalTanSpace = normalMap.Sample(mySampler, input.textureUV).xyz;
    normal = NormalMap(normalTanSpace, input.tangent_world, input.biTangent_world, input.normal_world.xyz);
#endif
    
    float3 cameraPos = -mul((float3x3) transpose(viewMatrix), float3(viewMatrix[0][3], viewMatrix[1][3], viewMatrix[2][3]));
    float3 vDir = normalize(cameraPos - input.position_world.xyz);
    float3 l = lightPosition - input.position_world.xyz;
    float3 lDir = normalize(l);
    //halfway vector
    float3 h = normalize(lDir + vDir);
    
    float iOmega = saturate(dot(normal, lDir));
    float oOmega = saturate(dot(normal, vDir));
    
    float att = lightStrength / dot(l, l);
    
    float3 lightRadiance = lightStrength * lightColor * att;
    
    
    
    
    float3 F0 = float3(0.04, 0.04, 0.04);
    F0 = lerp(F0, albedo, metallic);
    
    float D = DistributionGGX(normal, h, roughness);
    float G = GeometrySmith(normal, vDir, lDir, roughness);
    float F = fresnelSchlick(saturate(dot(vDir, h)), F0);
    float3 kd = float3(1, 1, 1) - F;
    kd = lerp(kd, float3(0, 0, 0), metallic * float3(1, 1, 1));
    
    float3 brdfSpec = D * G * F;
    brdfSpec /= max(0.001, 4 * iOmega * oOmega);
    
    float3 brdfDiff = kd * albedo; // /3.1415; //should i use pi ??
    
    float3 LightOutPut = (brdfDiff + brdfSpec) * lightRadiance * oOmega;
    
    //fix ambient
    
    float3 skyDiffIrradiance = skyIrrMap.Sample(skyMapSampler, normal).rgb;
    
    
    F = fresnelSchlickRoughness(oOmega, F0, roughness);
    kd = 1 - F;
    kd *= 1 - metallic;
    
    float3 diffuseAmbient = skyDiffIrradiance * albedo;
    
    
    
    
    float2 specSplitSum = splitSumBrdfLookUpMap.Sample(splitSumLookUpSampler, float2(oOmega, roughness)).rg;
    
    uint width, height, mipLevels;
    skyMap.GetDimensions(0, width, height, mipLevels);
    float3 skySpecIrradiance = skyMap.SampleLevel(mySampler, reflect(-vDir, normal), roughness * mipLevels);
    
    float3 specularAmbient = skySpecIrradiance * (F0 * specSplitSum.r + specSplitSum.g);
    
    
    float3 ambient = (kd * diffuseAmbient + specularAmbient); /** ambientOcclusion*/;
    
    
    //-------------------------
    float3 finalColor = LightOutPut + ambient;
    
    finalColor += emissive;
    
    finalColor = finalColor / (finalColor + 1);
    return float4(finalColor, alpha);
}
