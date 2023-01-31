cbuffer TimeBuffer : register(b0)
{
    float time;
};

struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
};

float4 main(InputType input) : SV_TARGET
{
    return float4(1.0,1.0,1.0,1.0);
}