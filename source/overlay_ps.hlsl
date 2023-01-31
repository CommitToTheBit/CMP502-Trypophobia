Texture2D textures[3];
SamplerState SampleType;

cbuffer TimeBuffer : register(b0)
{
    float time;
};

struct InputType
{
    float4 position : SV_POSITION;
};

float4 main(InputType input) : SV_TARGET
{
    float4 textureColor = textures[0].Sample(SampleType, float2(input.position.x/1280, input.position.y/720));
    float4 overlayColor = textures[1].Sample(SampleType, float2(input.position.x/1280, input.position.y/720));
    float overlayAlpha = textures[2].Sample(SampleType, float2(input.position.x/1280, input.position.y/720));

    float4 color = (1.0-overlayAlpha)*textureColor+overlayAlpha*overlayColor;

    return color;
}