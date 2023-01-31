Texture2D textures[6];
SamplerState SampleType;

cbuffer TimeBuffer : register(b0)
{
    float time;
};

struct InputType
{
    float4 position : SV_POSITION;
    float3 relativePosition : REL_POSITION;
};

float3 find_environment_st(float3 v)
{
    const float PI = 3.14159265;

    float extremity = max(length(v.x), max(length(v.y), length(v.z)));
    float extremities[6] = { -v.x, v.z, v.x, -v.z, v.y, -v.y };
    float2 sts[6] = { float2(-v.z, -v.y), float2(-v.x, -v.y), float2(v.z, -v.y), float2(v.x, -v.y), float2(-v.x, -v.z), float2(-v.x, v.z) };
    for (int i = 0; i < 6; i++)
    {
        if (extremity == extremities[i])
        {
            float2 st = sts[i]/extremity;

            float2 stEdge = st/max(length(st.x), length(st.y));
            float stProportion = length(st)/length(stEdge);
            st = (sin(stProportion*PI/3)/sin(PI/3))*stEdge;

            st = 0.5*(st+float2(1.0, 1.0));
            return float3(st.x, st.y, i);
        }
    }

    return float3(0, 0, -1);
}

float4 main(InputType input) : SV_TARGET
{
    float3 st = find_environment_st(input.relativePosition);
    //st.xy = fisheye(st.xy);
    //return float4(st.x, st.y, 0.0, 1.0);

    for (int i = 0; i < 6; i++)
        if (i == (int)st.z)
            return textures[i].Sample(SampleType, st.xy);

    return float4(0.0, 0.0, 0.0, 1.0);
}