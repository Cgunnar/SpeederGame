RWTexture2DArray<float4> outputTexture : register(u0);


static const float planetRadius = 2000.0f;
static const float atmosphereRadius = 2100.0f;
static const float densityFallOff = 4.0f;
static const float3 scatteringStrength = 1.0f;
static const float3 scatteringCoefficients = scatteringStrength * float3(pow(400.0f / 700.0f, 4), pow(400.0f / 530.0f, 4), pow(400.0f / 440.0f, 4));
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
    
    if (d > 0)
    {
        float s = sqrt(d);
        float dstToSphereNear = max(0.0f, (-b - s) / (2.0f * a));
        float dstToSphereFar = (-b + s) / (2.0f * a);
        if (dstToSphereFar >= 0.0f)
        {
            return float2(dstToSphereNear, dstToSphereFar - dstToSphereNear);
        }
    }
    return float2(FLT_MAX, 0.0f);
}

float3 AtmosphereicScattering(float3 rayOrigin, float3 rayDir, float rayLength)
{
    float3 inScatterPoint = rayOrigin;
    float stepSize = rayLength / (float) (numScatterPoints - 1);
    float3 inScatterdLight = float3(0, 0, 0);
    
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

float3 getSamplingVector(uint3 ThreadID)
{
    float outputWidth, outputHeight, outputDepth;
    outputTexture.GetDimensions(outputWidth, outputHeight, outputDepth);

    float2 st = ThreadID.xy / float2(outputWidth, outputHeight);
    float2 uv = 2.0 * float2(st.x, 1.0 - st.y) - float2(1.0, 1.0);

	// Select vector based on cubemap face index.
    float3 ret;
    switch (ThreadID.z)
    {
        case 0:
            ret = float3(1.0, uv.y, -uv.x);
            break;
        case 1:
            ret = float3(-1.0, uv.y, uv.x);
            break;
        case 2:
            ret = float3(uv.x, 1.0, -uv.y);
            break;
        case 3:
            ret = float3(uv.x, -1.0, uv.y);
            break;
        case 4:
            ret = float3(uv.x, uv.y, 1.0);
            break;
        case 5:
            ret = float3(-uv.x, uv.y, -1.0);
            break;
    }
    return normalize(ret);
}

[numthreads(32, 32, 1)]
void main(uint3 ThreadID : SV_DispatchThreadID)
{
    float3 rayDir = getSamplingVector(ThreadID);
    float3 rayOrigin = float3(0, 50, 0);
    
    float dstToSurfance = RaySphere(planetCenter, planetRadius, rayOrigin, rayDir).x;
    float2 hitInfo = RaySphere(planetCenter, atmosphereRadius, rayOrigin, rayDir);
    float dstToAtmosphere = hitInfo.x;
    float dstThroughAtmosphere = min(hitInfo.y, dstToSurfance - dstToAtmosphere);
   
    float3 sky;
    if (dstThroughAtmosphere > 0.0f)
    {
        const float epsilon = 0.0001f;
        float3 pointInAtmosphere = rayOrigin + rayDir * (dstToAtmosphere + epsilon);
        sky = AtmosphereicScattering(pointInAtmosphere, rayDir, dstThroughAtmosphere - epsilon * 2.0f);
    }
    else
    {
        sky = float3(0, 1, 0);
    }

    outputTexture[ThreadID] = float4(sky, 1);
}