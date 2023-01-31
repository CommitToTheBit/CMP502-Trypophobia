cbuffer TimeBuffer : register(b0)
{
    float time;
};

struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
};

/*float random(float3 xyz)
{
    return frac(34227.56*sin(dot(xyz, float3(256.3, 444.7, 524.0))));
}*/

float3 random3(float2 xy)
{
    return frac(34227.56*sin(float3(dot(xy, float2(256.3, 444.7)), dot(xy, float2(199.5, 270.4)), dot(xy, float2(390.5, 275.2)))));
}

float2 tile_st(float2 st)
{
    const int TILES_N = 3;
    const int2 TILES = int2(2*TILES_N, 2*round((2.0*TILES_N)/sqrt(3)));

    const float PERIOD = 0.6;
    const float VARIANCE = 0.5;

    const float PI = 3.14159265;

    st.x *= TILES.x;
    st.y *= TILES.y;

    int2 ist = floor(st);
    float2 fst = frac(st);

    st *= int2(2, 2);

    // STEP 0: Find ist as triangle tile coordinates
    int idirection = (ist.x%2+2)%2;
    float iboundary = (idirection == 0) ? fst.x : 1.0-fst.x;
    if (fst.y < 0.5-0.5*iboundary)
        ist = 2*ist+int2(idirection, 0);
    else if (fst.y < 0.5+0.5*iboundary)
        ist = 2*ist+int2(1-idirection, 1);
    else
        ist = 2*ist+int2(idirection, 2);

    // STEP 1: Finding ist, the index of the distorted tile st lies in...
    idirection = (ist.x%2+2)%2;
    float2 ivertices[4];
    int2 iindices[4] = { int2(-1, -idirection), int2(1, idirection-1), int2(1-2*idirection, 1), int2(0, 0) };
    for (int i = 0; i < 4; i++)
    {
        int direction = ((idirection+iindices[i].x)%2+2)%2;
        float centre = (direction == 0) ? 1.0-0.5/tan(PI/3) : 0.5/tan(PI/3);

        float3 triangularRandomness = random3((ist+iindices[i]+TILES)%TILES);
        triangularRandomness = VARIANCE*sin(PERIOD*(1.0+length(triangularRandomness))*time+6.2831*triangularRandomness);
        triangularRandomness *= 0.5*tan(PI/6); // Keeps point within the triangle's inner circumference, for VARIANCE < 1.0

        float theta[3] = { -2+direction*PI/3, direction*PI/3, 2+direction*PI/3 };
        float2 randomness = float2(dot(triangularRandomness, float3(cos(theta[0]), cos(theta[1]), cos(theta[2]))), dot(triangularRandomness, float3(sin(theta[0]), sin(theta[1]), sin(theta[2]))));

        ivertices[i] = ist+iindices[i]+randomness+float2(centre, 0.0);
    }

    int iq = (ivertices[0].y < ivertices[3].y || idirection == 1) ? 2 : 0;
    int2 iqindices[3] = { int2(1-2*(1-idirection), -1), int2(1, idirection), int2(-1, 1-idirection) };
    float itheta = atan2(st.y-ivertices[3].y, st.x-ivertices[3].x);
    for (int i = iq; i < iq+4; i++)
    {
        if (itheta < atan2(ivertices[(i+1)%3].y-ivertices[3].y, ivertices[(i+1)%3].x-ivertices[3].x))
        {
            iq = i%3;
            break;
        }
    }
    ist += iqindices[iq]-int2(idirection, 0);

    // STEP 2: Finding fst, relative to the boundary of our (distorted) tile ist...
    float2 fvertices[6];
    int2 findices[6] = { int2(-1, 0), int2(0, -1), int2(1, -1), int2(2, 0), int2(1, 1), int2(0, 1) };
    for (int i = 0; i < 6; i++)
    {
        int direction = i%2;
        float centre = (direction == 0) ? 1.0-0.5/tan(PI/3) : 0.5/tan(PI/3);

        float3 triangularRandomness = random3((ist+findices[i]+TILES)%TILES);
        triangularRandomness = VARIANCE*sin(PERIOD*(1.0+length(triangularRandomness))*time+6.2831*triangularRandomness);
        triangularRandomness *= 0.5*tan(PI/6); // Keeps point within the triangle's inner circumference, for VARIANCE < 1.0

        float theta[3] = { -2+direction*PI/3, direction*PI/3, 2+direction*PI/3 };
        float2 randomness = float2(dot(triangularRandomness, float3(cos(theta[0]), cos(theta[1]), cos(theta[2]))), dot(triangularRandomness, float3(sin(theta[0]), sin(theta[1]), sin(theta[2]))));

        fvertices[i] = ist+findices[i]+randomness+float2(centre, 0.0);
    }
    float2 fvertexMean = (fvertices[0]+fvertices[1]+fvertices[2]+fvertices[3]+fvertices[4]+fvertices[5])/6;

    ist = floor((float2)ist/2);
    fst = float2(0.5, 0.5);
    if (length(st-fvertexMean) == 0)
        return ist+fst;
    else if (length(st.x-fvertexMean.x) < 0.0005) // NB: Avoids division by zero!
        st.x = fvertexMean.x+0.0005;

    int fq = (fvertices[0].y < fvertexMean.y) ? 5 : 0; // NB: Note assumption of low variance...
    float ftheta = atan2(st.y-fvertexMean.y, st.x-fvertexMean.x);
    for (int i = fq; i < fq+6; i++)
    {
        if (ftheta < atan2(fvertices[(i+1)%6].y-fvertexMean.y, fvertices[(i+1)%6].x-fvertexMean.x))
        {
            fq = i%6;
            break;
        }
    }

    // Finding where a line from vertexMean to st intersects with an integer edge...
    float a[2] = { (st.y-fvertexMean.y)/(st.x-fvertexMean.x), (fvertices[(fq+1)%6].y-fvertices[fq].y)/(fvertices[(fq+1)%6].x-fvertices[fq].x) };
    float b[2] = { fvertexMean.y-a[0]*fvertexMean.x, fvertices[fq].y-a[1]*fvertices[fq].x };
    float xIntersect = (b[1]-b[0])/(a[0]-a[1]);
    float yIntersect = a[0]*xIntersect+b[0];
    float2 intersect = float2(xIntersect, yIntersect);

    float outPrime = length(st-fvertexMean)/length(intersect-fvertexMean);
    float thetaPrime = atan2(intersect.y-fvertexMean.y, intersect.x-fvertexMean.x);

    float thetaRelative = acos(dot(intersect-fvertexMean, fvertices[fq]-fvertexMean)/(length(intersect-fvertexMean)*length(fvertices[fq]-fvertexMean)));
    float thetaRange = acos(dot(fvertices[(fq+1)%6]-fvertexMean, fvertices[fq]-fvertexMean)/(length(fvertices[(fq+1)%6]-fvertexMean)*length(fvertices[fq]-fvertexMean)));

    float2 f = float2(1.0, clamp(tan((thetaRelative/thetaRange-0.5)*PI/3), tan(-PI/6), tan(PI/6)));
    fst += 0.5*cos(PI/6)*outPrime*float2(f.x*cos(5*PI/6-fq*(PI/3))+f.y*sin(5*PI/6-fq*(PI/3)), -f.x*sin(5*PI/6-fq*(PI/3))+f.y*cos(5*PI/6-fq*(PI/3)));

    return ist+fst;
}

/* ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- */
/* This enclosed section has been adapted from: The Book of Shaders (no date) The Book of Shaders: Noise. Available at: https://thebookofshaders.com/11/ (Accessed: 30 December 2022) */
/*float noise(float2 st, int I)
{
    const float PI = 3.14159265;

    int2 ist = floor(st);
    float2 fst = frac(st);

    float ftheta = atan2(fst.y-0.5, fst.x-0.5);

    int i = (floor((I*ftheta)/(2*PI))+I)%I;
    float f = frac((I*ftheta)/(2*PI));
    float u = f*f*f*(f*(f*6.-15.0)+10.0);

    return lerp(random(float3(ist.x, ist.y, i)), random(float3(ist.x, ist.y, (i+1)%I)), u);
}*/
/* ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- */
/* This enclosed section has been adapted from: The Book of Shaders (no date) The Book of Shaders: Fractal Brownian Motion. Available at: https://thebookofshaders.com/13/ (Accessed: 30 December 2022) */
/*float fractal_noise(float2 st, int I)
{
    float amplitude = 0.5;
    float frequency = 1.0;

    float y = 0;
    for (int i = 0; i < 6; i++)
    {
        y += amplitude * noise(st, frequency*I);
        frequency *= 2.0;
        amplitude *= 0.5;
    }

    return y;
}*/
/* ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- */

float4 main(InputType input) : SV_TARGET
{
    const float PERIOD = 3.0f;
    const float PI = 3.14159265;

    const float H_MAX = 0.75+0.25*sin(2*PI*time/PERIOD);
    const float H_MIN = 0.0;

    const int PRIME[6] = { 1, 2, 3, 5, 7, 11 };

    float R[6];
    R[0] = 0.85-0.05*sin(2*PI*time/(PRIME[0]*PERIOD));
    for (int i = 1; i < 6; i++)
        R[i] = 0.55+0.2*(1-pow(2, -i))*sin(2*PI*time/(PRIME[i]*PERIOD));

    float2 tiled_st = tile_st(input.tex);
    int2 ist = floor(tiled_st);
    float2 fst = frac(tiled_st);

    float fr = (2.0/cos(PI/6))*length(fst-float2(0.5, 0.5)); // NB: Fitting to inside of hex...
    fr = min(fr, 1.0);

    float ftheta = atan2(fst.y-0.5, fst.x-0.5);
    float fphi = PI/2;
    for (int i = 0; i < 6; i++)
    {
        if (fr > R[i])
        {
            if (i == 0)
                break;

            float fgrad = -pow(2, i+1)*H_MAX/(R[i-1]-R[i])*pow(1-(fr-R[i])/(R[i-1]-R[i]), 2);
            fphi = atan2(1, -fgrad);
            break;
        }
    }

    return float4(0.5+0.5*cos(fphi)*cos(ftheta), 0.5+0.5*cos(fphi)*sin(ftheta), 0.5+0.5*sin(fphi), 1.0);
}