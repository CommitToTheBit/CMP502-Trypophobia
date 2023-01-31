Texture2D textures[8];
SamplerState SampleType;

cbuffer TimeBuffer : register(b0)
{
    float time;
};

cbuffer LightBuffer : register(b1)
{
    float4 ambientColor;
    float4 diffuseColor;
    float3 lightPosition;
    float sourceStrength;
};

cbuffer RefractionBuffer : register(b2)
{
    float opacity;
    float refractiveIndex;
    float culling;
}

cbuffer CameraBuffer : register(b3)
{
    float3 cameraPosition;
}

struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 relativePosition : TEXCOORD2;
    float3 position3D : TEXCOORD5;
    float3 normal : NORMAL;
    float3 tangent: TANGENT;
    float3 binormal : BINORMAL;
};

float3 find_environment_st(float3 v)
{
    const float PI = 3.14159265;

    float extremity = max(length(v.x), max(length(v.y), length(v.z)));
    float extremities[6] = { -v.x, v.z, v.x, -v.z, v.y, -v.y };
    float2 sts[6] = { float2(-v.z, -v.y), float2(-v.x, -v.y), float2(v.z, -v.y), float2(v.x, -v.y), float2(-v.x, -v.z), float2(-v.x, v.z) };
    for (int i = 0; i < 6; i++)
    {
        if (extremity == extremities[i])
        {
            float2 st = sts[i]/extremity;
            st = 0.5*(st+float2(1.0, 1.0));
            return float3(st.x, st.y, i);
        }
    }

    return float3(0, 0, -1);
}

float4 main(InputType input) : SV_TARGET
{
    // STEP 1: Sample from the base textures to calculate the pixel's base colour
    float4 textureColor = textures[0].Sample(SampleType, input.tex);

    /* ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ */
    /* This enclosed section has been adapted from: RasterTek (no date) Tutorial 20: Bump Mapping. Available at https://www.rastertek.com/dx11tut20.html (Accessed: 28 December 2022) */

    // STEP 2: Sample from the normal map to calculate 'pixel normal'
    // FIXME: Need to account for back-face culling!
    float4 normalMap = 2.0f * textures[1].Sample(SampleType, input.tex) - 1.0f;
    float3 normal = normalMap.x * input.tangent + normalMap.y * input.binormal + normalMap.z * input.normal;
    normal = culling*normalize(normal);

    /* ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ */

    // STEP 3: Calculate lighting hitting pixel
    float3 lightDirection = normalize(input.position3D - lightPosition);
    float3 lightDistance = max(length(input.position3D - lightPosition), pow(sourceStrength, 0.5));
    float lightIntensity = sourceStrength*saturate(dot(normal, -lightDirection))/(lightDistance*lightDistance);
    float4 lightColor = ambientColor + diffuseColor * lightIntensity;
    lightColor = saturate(lightColor);

    // STEP 4: Calculate point on environment map light is refracted from
    float3 vRefraction = refract((input.position3D-cameraPosition), normal, refractiveIndex);
    float3 stRefraction = find_environment_st(vRefraction);
    float4 refractionColor = float4(0.0, 0.0, 0.0, 1.0);
    for (int i = 0; i < 6; i++)
        if (i == (int)stRefraction.z)
            refractionColor = textures[2+i].Sample(SampleType, stRefraction.xy);

    // STEP 5: Applying lighting to pixel's base colour.
    float4 color = lightColor * (opacity* textureColor+(1.0-opacity)*refractionColor);

    return color;
}