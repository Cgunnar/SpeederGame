
Texture2D albedoTexture : register(t0);
Texture2D metallicRoughnessTexture : register(t1);
Texture2D normalMap : register(t2);
Texture2D emissiveTexture : register(t3);
Texture2D splitSumBrdfLookUpMap : register(t4);
TextureCube skyMap : register(t5);
TextureCube skyIrrMap : register(t6);
Texture2D shadowMap : register(t7);


SamplerState linearWrapSampler : register(s0);
SamplerState skyMapSampler : register(s1);
SamplerState splitSumLookUpSampler : register(s2);
SamplerState renderPassSpecificSampler : register(s3);
SamplerState shadowMapSampler : register(s4);





cbuffer PointLight : register(b0)
{
    float3 pointLightPosition;
    float pointLightStrength;
    float3 pointLightColor;
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

cbuffer DirLight : register(b3)
{
    float3 directionalLightDir;
    float directionalLightStrength;
    float3 directionalLightColor;
};

cbuffer ShadowMap : register(b4)
{
    matrix shadowMapVP;
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
    return F0 + (1.0 - F0) * pow(saturate(1.0 - cosTheta), 5.0);
}

float3 fresnelSchlickRoughness(float cosTheta, float3 F0, float roughness)
{
    return F0 + (max(float3(1.0 - roughness, 1.0 - roughness, 1.0 - roughness), F0) - F0) * pow(saturate(1.0 - cosTheta), 5.0);
}

float3 BRDF(float3 albedo, float metallic, float roughness, float3 F0, float3 normal, float3 vDir, float oOmega, float3 lDir, float att, float3 lightColor, float lightStrength)
{
    float3 h = normalize(lDir + vDir); //halfway vector
    float iOmega = saturate(dot(normal, lDir));
    float3 lightRadiance = lightStrength * lightColor * att;
    
    float D = DistributionGGX(normal, h, roughness);
    float G = GeometrySmith(normal, vDir, lDir, roughness);
    float3 F = fresnelSchlick(saturate(dot(vDir, h)), F0);
    float3 kd = float3(1, 1, 1) - F;
    kd = lerp(kd, float3(0, 0, 0), metallic * float3(1, 1, 1));
    
    float3 brdfSpec = D * G * F;
    brdfSpec /= max(0.001, 4 * iOmega * oOmega);
    
    float3 brdfDiff = kd * albedo; // /3.1415; //should i use pi ??
    return (brdfDiff + brdfSpec) * lightRadiance * iOmega;
}

float4 main(vs_out input) : SV_TARGET
{
    //shadow mapping
    float4 shadowCoord = mul(shadowMapVP, input.position_world);
    shadowCoord.xyz = shadowCoord.xyz / shadowCoord.w;
    shadowCoord.x = shadowCoord.x * 0.5f + 0.5f;
    shadowCoord.y = -shadowCoord.y * 0.5f + 0.5f;
    float shadowFactor = 1;
    float bias = max(0.001 * (1.0 - dot(normalize(input.normal_world.xyz), -normalize(directionalLightDir))), 0.0001);
    if (shadowCoord.z <= 1 && shadowMap.Sample(shadowMapSampler, shadowCoord.xy).r + bias < shadowCoord.z)
        shadowFactor = 0.1f;
    
    
    
    //pbr
    float3 albedo = albedoFactor.rgb;
    float alpha = albedoFactor.a;
    
#ifdef ALBEDO_TEXTURE
    float4 albedoTextureVal = albedoTexture.Sample(renderPassSpecificSampler, input.textureUV);
    albedo *= albedoTextureVal.rgb;
    alpha *= albedoTextureVal.a;
#endif
    
    
#ifndef NO_METALLIC_ROUGHNESS_TEXTURE
    float4 metallicRoughnessTextureVal = metallicRoughnessTexture.Sample(renderPassSpecificSampler, input.textureUV);
    
    float ambientOcclusion = metallicRoughnessTextureVal.r;
    float metallic = metallicRoughnessTextureVal.b * metallicFactor;
    float roughness = metallicRoughnessTextureVal.g * roughnessFactor;
#else
    float ambientOcclusion = 1;
    float metallic = metallicFactor;
    float roughness = roughnessFactor;
#endif
    
    roughness = max(roughness, 0.05);
    
    float3 emissive = emissiveFactor;
#ifdef EMISSIVE
    emissive *= emissiveTexture.Sample(renderPassSpecificSampler, input.textureUV).xyz;
#endif
    
    float3 normal = normalize(input.normal_world.xyz);
    
#ifdef NORMAL_MAP
    float3 normalTanSpace = normalMap.Sample(renderPassSpecificSampler, input.textureUV).xyz;
    normal = NormalMap(normalTanSpace, input.tangent_world, input.biTangent_world, input.normal_world.xyz);
#endif
    
#ifdef TERRAINSHADING
    albedo = TerrainShader(input.position_world.xyz, normal, albedo);
#endif
    
    
    float3 cameraPos = -mul((float3x3) transpose(viewMatrix), float3(viewMatrix[0][3], viewMatrix[1][3], viewMatrix[2][3]));
    float3 vDir = normalize(cameraPos - input.position_world.xyz);
    float oOmega = saturate(dot(normal, vDir));
    
    float3 LightOutPut = { 0, 0, 0 };
    float3 F0 = float3(0.04, 0.04, 0.04);
    F0 = lerp(F0, albedo, metallic);
    
    //pointLight
    
    float3 l = pointLightPosition - input.position_world.xyz;
    float3 lDir = normalize(l);
    float att = 1.0f / dot(l, l);
    LightOutPut += BRDF(albedo, metallic, roughness, F0, normal, vDir, oOmega, lDir, att, pointLightColor, pointLightStrength);
    
    
    //dirLight
    lDir = -normalize(directionalLightDir);//should not be needed but i dont trust this to be normalized
    att = 1;
    LightOutPut += shadowFactor * BRDF(albedo, metallic, roughness, F0, normal, vDir, oOmega, lDir, att, directionalLightColor, directionalLightStrength);
    
    
    //fix ambient
    
    float3 skyDiffIrradiance = skyIrrMap.Sample(skyMapSampler, normal).rgb;
    
    
    float3 F = fresnelSchlickRoughness(oOmega, F0, roughness);
    float3 kd = 1 - F;
    kd *= 1 - metallic;
    
    float3 diffuseAmbient = skyDiffIrradiance * albedo;
    
    
    
    
    float2 specSplitSum = splitSumBrdfLookUpMap.Sample(splitSumLookUpSampler, float2(oOmega, roughness)).rg;
    
    uint width, height, mipLevels;
    skyMap.GetDimensions(0, width, height, mipLevels);
    float3 skySpecIrradiance = skyMap.SampleLevel(linearWrapSampler, reflect(-vDir, normal), roughness * mipLevels);
    
    float3 specularAmbient = skySpecIrradiance * (F0 * specSplitSum.r + specSplitSum.g);
    
    
    float3 ambient = (kd * diffuseAmbient + specularAmbient) * ambientOcclusion;
    
    
    //-------------------------
    float3 finalColor = LightOutPut + ambient + emissive;
    
    
    float s = saturate((length(cameraPos - input.position_world.xyz) - 6000.0f) / 800.0f);
    s *= s;
    float3 sky = skyMap.Sample(linearWrapSampler, -vDir).xyz;
    finalColor = lerp(finalColor, sky, float3(s, s, s));
    
    //finalColor = finalColor / (finalColor + 1);
    return float4(finalColor, alpha);
    //return float4(normal, 1);
//#ifdef NORMAL_MAP
//    return float4(input.tangent_world, 1);
//#endif
    //float3 dn = normalize(input.normal_world.xyz);
    //if (input.position_world.x > 1)
    //    return float4(dn.x, 0, dn.z, 1);
    //return float4(normal.x, 0, normal.z, alpha);
}