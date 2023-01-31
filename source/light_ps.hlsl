Texture2D textures[2];
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

struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 position3D : TEXCOORD2;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 binormal : BINORMAL;
};

float4 main(InputType input) : SV_TARGET
{
    const float PI = 3.14159265;

    // STEP 1: Sample from the base textures to calculate the pixel's base colour
    float4 textureColor = textures[0].Sample(SampleType, input.tex);

    /* ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ */ 
    /* This enclosed section has been adapted from: RasterTek (no date) Tutorial 20: Bump Mapping. Available at https://www.rastertek.com/dx11tut20.html (Accessed: 28 December 2022) */

    // STEP 2: Sample from the normal map to calculate 'pixel normal'
    float4 normalMap = 2.0f * textures[1].Sample(SampleType, input.tex) - 1.0f;
    float3 normal = normalMap.x * input.tangent + normalMap.y * input.binormal + normalMap.z * input.normal;
    normal = normalize(normal);

    /* ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ */

    // STEP 3: Calculate lighting hitting pixel
    float3 lightDirection = normalize(input.position3D - lightPosition);
    float3 lightDistance = max(length(input.position3D - lightPosition), pow(sourceStrength, 0.5));
    float lightIntensity = sourceStrength*saturate(dot(normal, -lightDirection))/(lightDistance*lightDistance);
    float4 lightColor = ambientColor + diffuseColor * lightIntensity;
    lightColor = saturate(lightColor);

    // STEP 4: Applying lighting to pixel's base colour.
    float4 color = lightColor * textureColor;

    return color;
}
