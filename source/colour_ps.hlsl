Texture2D textures[1];
SamplerState SampleType;

struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 position3D : TEXCOORD2;
};

float4 main(InputType input) : SV_TARGET
{
    return textures[0].Sample(SampleType, input.tex);
}