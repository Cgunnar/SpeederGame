TextureCube skyMap : register(t4);
SamplerState skyMapSampler : register(s1);

struct vs_out
{
    float4 pos : SV_Position;
    float3 samplePos : POSITION;
};

static const float planetRadius = 2010.0f;
static const float atmosphereRadius = 2110.0f;
static const float densityFallOff = 0.5f;
static const float3 scatteringCoefficients = float3(1, 1, 1);
static const int numOpticalDepthPoints = 10;
static const int numScatterPoints = 10;
#define FLT_MAX 3.402823466E+38

static const float3 dirToSun = float3(0, 1, 0);
static const float3 planetCenter = float3(0, -2000, 0);

float DensityAtPoint(float3 p)
{
    float altitude = p.y;
    float height = altitude / (atmosphereRadius - planetRadius);
    return exp(-height * densityFallOff) * (1.0f - height);
}

float OpticalDepth(float3 rayOrigin, float3 rayDir, float rayLength)
{
    float3 densitySamplePoint = rayOrigin;
    float s = rayLength / (float) (numOpticalDepthPoints - 1);
    float opticalDepth = 0;
    
    for (int i = 0; i < numOpticalDepthPoints; i++)
    {
        float density = DensityAtPoint(densitySamplePoint);
        opticalDepth += density * s;
        densitySamplePoint += rayDir * s;
    }
    return opticalDepth;
}

float2 RaySphere(float3 center, float radius, float3 origin, float3 dir)
{
    float3 offset = origin - center;
    float a = 1.0f;
    float b = 2.0f * dot(offset, dir);
    float c = dot(offset, offset) - radius * radius;
    float d = b * b - 4.0f * a * c;
    
    if(d > 0)
    {
        float s = sqrt(d);
        float dstToSphereNear = max(0.0f, (-b - s) / (2.0f * a));
        float dstToSphereFar = (-b + s) / (2.0f * a);
        if(dstToSphereFar >= 0.0f)
        {
            return float2(dstToSphereNear, dstToSphereFar - dstToSphereNear);
        }
    }
    return float2(FLT_MAX, 0.0f);

}

float3 AtmosphereicScattering(float3 rayOrigin, float3 rayDir, float rayLength)
{
   
    
    float3 inScatterPoint = rayOrigin;
    float stepSize = rayLength / (float)(numScatterPoints - 1);
    float3 inScatterdLight = 0;
    
    for (int i = 0; i < numScatterPoints; i++)
    {
        float sunRayLength = RaySphere(planetCenter, atmosphereRadius, inScatterPoint, dirToSun).y;
        float sunRayOpticalDepth = OpticalDepth(inScatterPoint, dirToSun, sunRayLength);
        float viewRayOpticalDepth = OpticalDepth(inScatterPoint, -rayDir, stepSize * i);
        float3 transmittance = exp(-(sunRayOpticalDepth + viewRayOpticalDepth) * scatteringCoefficients);
        float density = DensityAtPoint(inScatterPoint);

        inScatterdLight += density * transmittance * scatteringCoefficients * stepSize;
        inScatterPoint += rayDir * stepSize;
    }
    return inScatterdLight;
}


float4 main(vs_out input) : SV_TARGET
{
    //float3 sky = skyMap.Sample(skyMapSampler, input.samplePos).xyz;
    //sky = sky / (sky + float3(1, 1, 1));
    //return float4(sky, 1.0f);
    
    float3 rayOrigin = float3(0, 0, 0);
    float3 rayDir = input.samplePos;
    
    float dstToSurfance = 0;
    float2 hitInfo = RaySphere(planetCenter, atmosphereRadius, rayOrigin, rayDir);
    float dstToAtmosphere = hitInfo.x;
    float dstThroughAtmosphere = min(hitInfo.y, dstToSurfance - dstToAtmosphere);
    return float4(dstToAtmosphere, dstThroughAtmosphere, 0, 1);
    //if (dstThroughAtmosphere > 0)
    {
        const float epsilon = 0.0001f;
        float3 pointInAtmosphere = rayOrigin + rayDir * (dstToAtmosphere + epsilon);
        float3 sky = AtmosphereicScattering(pointInAtmosphere, rayDir, dstThroughAtmosphere - epsilon * 2);
        return float4(sky, 1);
    }
    //else
    //{
    //    return float4(1, 0, 0, 1);
    //}
}
