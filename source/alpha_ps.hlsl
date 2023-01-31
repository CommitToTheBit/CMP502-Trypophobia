Texture2D textures[1];
SamplerState SampleType;

cbuffer TimeBuffer : register(b0)
{
    float time;
};

cbuffer AlphaBuffer : register(b1)
{
    float alpha;
};

struct InputType
{
    float4 position : SV_POSITION;
    float3 tex : TEXCOORD0;
};

float4 main(InputType input) : SV_TARGET
{
    float baseAlpha = textures[0].Sample(SampleType, float2(input.position.x/1280, input.position.y/720));
    baseAlpha = 1.0-(1.0-baseAlpha)*(1.0-alpha);

    return float4(baseAlpha, baseAlpha, baseAlpha, 1.0);
}