cbuffer TransformConstants : register(b0)
{
    float4x4 worldViewProjection;
    float4x4 normalTransform;
};

struct VertexOutput
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
};

VertexOutput VSMain(float3 position : POSITION, float3 normal : NORMAL)
{
    VertexOutput output;
    output.position = mul(float4(position, 1.0f), worldViewProjection);
    output.normal = mul(normal, (float3x3)normalTransform);
    return output;
}

float4 PSMain(VertexOutput input) : SV_TARGET
{
    const float3 directionToLight = normalize(float3(0.4f, 1.0f, -0.6f));
    const float3 baseColour = float3(1.0f, 0.5f, 0.1f);
    float3 normal = input.normal / max(length(input.normal), 0.0001f);
    float diffuse = max(dot(normal, directionToLight), 0.0f);
    float3 colour = baseColour * (0.18f + 0.82f * diffuse);

    // Approximate display gamma: our UNORM render target doesn't encode sRGB.
    return float4(pow(saturate(colour), 1.0f / 2.2f), 1.0f);
}
