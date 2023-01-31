cbuffer TimeBuffer : register(b0)
{
    float time;
};

struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
};

float2 random2(float2 xy)
{
    return frac(34227.56*sin(float2(dot(xy, float2(256.3, 444.7)), dot(xy, float2(199.5, 270.4)))));
}

float metric(float2 a, float2 b)
{
    return length(a-b);
}

float2 tile_st(float2 st)
{
    const int TILES = 4;

    const float PERIOD = 0.1;
    const float VARIANCE = 0.4; // NB: Keep < 0.5; for loops assume one of 3x3 ivertices is nearest...

    const float PI = 3.14159265;

    st *= TILES;
    int2 ist = floor(st);

    int2 closest_ist = ist;
    float closest_distance = 1.0; // NB: Assumes VARIANCE < 0.5...
    for (int i = -1; i <= 1; i++)
    {
        for (int j = -1; j <= 1; j++)
        {
            float2 randomness = random2((ist+int2(i, j)+int2(TILES, TILES))%TILES); // Modulus 'loops' our texture
            randomness = float2(0.5, 0.5)+VARIANCE*sin(PERIOD*(1.0+length(randomness))*time+6.2831*randomness);
            float2 ivertex = ist+int2(i, j)+randomness;

            float distance = metric(st, ivertex);

            // DEBUG: Highlighting relevant points in black
            if (length(st-ivertex) < 0.05)
                return int2(-1, -1);

            if (distance < closest_distance)
            {
                closest_ist = ist+int2(i, j);
                closest_distance = distance;
            }
        }
    }
    ist = closest_ist;

    return ist;
}

float4 main(InputType input) : SV_TARGET
{
    float2 tiled_st = tile_st(input.tex);
    int2 ist = floor(tiled_st);
    float2 fst = frac(tiled_st);

    return float4((ist.x+1.0)/5, (ist.y+1.0)/5, 0.0f, 1.0f);
}