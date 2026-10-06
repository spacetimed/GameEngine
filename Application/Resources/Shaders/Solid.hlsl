cbuffer TransformConstants : register(b0)
{
    float4x4 worldViewProjection;
};

float4 VSMain(float3 position : POSITION) : SV_POSITION
{
    return mul(float4(position, 1.0f), worldViewProjection);
}

float4 PSMain() : SV_TARGET
{
    return float4(1.0f, 0.5f, 0.1f, 1.0f);
}
