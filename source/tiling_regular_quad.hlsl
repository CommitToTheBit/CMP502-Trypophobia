cbuffer TimeBuffer : register(b0)
{
    float time;
};

struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
};

float2 tile_st(float2 st)
{
    const int TILES = 6;

    return TILES*st;
}

float4 main(InputType input) : SV_TARGET
{
    float2 tiled_st = tile_st(input.tex);
    int2 ist = floor(tiled_st);
    float2 fst = frac(tiled_st);

    return float4(fst.x, fst.y, 0.0f, 1.0f);
}