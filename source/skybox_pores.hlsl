cbuffer TimeBuffer : register(b0)
{
    float time;
};

struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
};

float random(float2 xy)
{
    return frac(34227.56*sin(dot(xy, float2(256.3, 444.7))));
}

float4 main(InputType input) : SV_TARGET
{
    float2 st = 50*input.tex;
    int2 ist = floor(st);
    float2 fst = frac(st);
    if (random(ist) < 0.05 && length(2.0*fst-1.0) < 0.1)
        return float4(1.0, 1.0, 1.0, 1.0);

    return float4(0.0, 0.0, 0.0, 1.0);
}