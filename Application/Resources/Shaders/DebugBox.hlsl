cbuffer TransformConstants : register(b0)
{
    float4x4 worldViewProjection;
    float4x4 normalTransform;
};

float4 VSMain(float3 position : POSITION, float3 normal : NORMAL) : SV_POSITION
{
    return mul(float4(position, 1.0f), worldViewProjection);
}

float4 PSMain() : SV_TARGET
{
    return float4(0.2f, 1.0f, 0.4f, 0.35f);
}
