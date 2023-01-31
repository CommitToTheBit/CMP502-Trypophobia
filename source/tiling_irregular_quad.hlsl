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

float2 tile_st(float2 st)
{
    const int TILES = 6;

    const float PERIOD = 0.2;
    const float VARIANCE = 0.2; // NB: Keep < 0.25; fst calculations assume convexity 

    const float PI = 3.14159265;

    st *= TILES;
    st += float2(0.5, 0.5);
    int2 ist = floor(st); // Setting this as a placeholder for STEP 1...
    float2 fst = float2(0.5, 0.5);

    // STEP 1: Finding ist, the index of the distorted tile st lies in...
    float2 ivertices[5];
    int2 iindices[5] = { int2(-1, 0), int2(0, -1), int2(1, 0), int2(0, 1), int2(0, 0) };
    for (int i = 0; i < 5; i++)
    {
        float2 randomness = random2((ist+iindices[i]+int2(TILES, TILES))%TILES); // Modulus 'loops' our texture
        randomness = float2(0.5, 0.5)+VARIANCE*sin(PERIOD*(1.0+length(randomness))*time+6.2831*randomness);
        ivertices[i] = ist+iindices[i]+randomness;
    }

    int iq = (ivertices[0].y < ivertices[4].y) ? 3 : 0;
    int2 iqindices[4] = { int2(0, 0), int2(1, 0), int2(1, 1), int2(0, 1) };
    float itheta = atan2(st.y-ivertices[4].y, st.x-ivertices[4].x);
    for (int i = iq; i < iq+4; i++)
    {
        if (itheta < atan2(ivertices[(i+1)%4].y-ivertices[4].y, ivertices[(i+1)%4].x-ivertices[4].x))
        {
            iq = i%4;
            break;
        }
    }
    ist += iqindices[iq]-int2(1, 1);

    // STEP 2: Finding fst, relative to the boundary of our (distorted) tile ist...
    float2 fvertices[4];
    int2 findices[4] = { int2(0, 0), int2(1, 0), int2(1, 1), int2(0, 1) };
    for (int i = 0; i < 4; i++)
    {
        float2 randomness = random2((ist+findices[i]+int2(TILES, TILES))%TILES); // Modulus 'loops' our texture
        randomness = float2(0.5, 0.5)+VARIANCE*sin(PERIOD*(1.0+length(randomness))*time+6.2831*randomness);
        fvertices[i] = ist+findices[i]+randomness;
    }
    float2 fvertexMean = (fvertices[0]+fvertices[1]+fvertices[2]+fvertices[3])/4;

    if (length(st-fvertexMean) == 0)
        return ist+fst;

    int fq;
    float ftheta = atan2(st.y-fvertexMean.y, st.x-fvertexMean.x);
    if (fvertices[3].y <= fvertexMean.y) // We start going from down right to down, in quadrant 2 (recall, y == 0 is upper left!)
        fq = 2;
    else if (fvertices[0].y <= fvertexMean.y) //  We start going from down to centre, in quadrant 3
        fq = 3;
    else // We start going from centre to right, in quadrant 0; we will start from right to down right, as vertexMean is a mean
        fq = 0;
    for (int i = fq; i < fq+4; i++)
    {
        if (ftheta < atan2(fvertices[(i+1)%4].y-fvertexMean.y, fvertices[(i+1)%4].x-fvertexMean.x))
        {
            fq = i%4;
            break;
        }
    }

    // Finding where a line from vertexMean to st intersects with an integer edge...
    float a[2] = { (st.y-fvertexMean.y)/(st.x-fvertexMean.x), (fvertices[(fq+1)%4].y-fvertices[fq].y)/(fvertices[(fq+1)%4].x-fvertices[fq].x) };
    float b[2] = { fvertexMean.y-a[0]*fvertexMean.x, fvertices[fq].y-a[1]*fvertices[fq].x };
    float xIntersect = (b[1]-b[0])/(a[0]-a[1]);
    float yIntersect = a[0]*xIntersect+b[0];
    float2 intersect = float2(xIntersect, yIntersect);

    float outPrime = length(st-fvertexMean)/length(intersect-fvertexMean);
    float thetaPrime = atan2(intersect.y-fvertexMean.y, intersect.x-fvertexMean.x);

    float thetaRelative = acos(dot(intersect-fvertexMean, fvertices[fq]-fvertexMean)/(length(intersect-fvertexMean)*length(fvertices[fq]-fvertexMean)));
    float thetaRange = acos(dot(fvertices[(fq+1)%4]-fvertexMean, fvertices[fq]-fvertexMean)/(length(fvertices[(fq+1)%4]-fvertexMean)*length(fvertices[fq]-fvertexMean)));

    float2 f = float2(1.0, clamp(tan((thetaRelative/thetaRange-0.5)*PI/2), tan(-PI/4), tan(PI/4)));
    fst += 0.5*outPrime*float2(f.x*cos(PI-fq*(PI/2))+f.y*sin(PI-fq*(PI/2)), -f.x*sin(PI-fq*(PI/2))+f.y*cos(PI-fq*(PI/2)));
    fst = float2(1.0f-fst.y, fst.x); // NB: Converting back to 'regular' st coordinate axes...

    return ist+fst;
}

float4 main(InputType input) : SV_TARGET
{
    float2 tiled_st = tile_st(input.tex);
    int2 ist = floor(tiled_st);
    float2 fst = frac(tiled_st);

    return float4(fst.x, fst.y, 0.0f, 1.0f);
}