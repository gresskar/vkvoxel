struct VertexInput
{
    float3 position;
    float3 color;
    float2 texCoord;
    float4 texAtlas;
};

struct UniformBuffer {
    float4x4 model;
    float4x4 view;
    float4x4 projection;
};

ConstantBuffer<UniformBuffer> ubo;

struct VertexOutput
{
    float4 position : SV_Position;
    float3 color;
    float2 texCoord;
    float4 texAtlas;
};

[shader("vertex")]
VertexOutput vertMain(VertexInput input)
{
    VertexOutput output;
    output.position = mul(ubo.projection, mul(ubo.view, mul(ubo.model, float4(input.position, 1.0))));
    output.color = input.color;
    output.texCoord = input.texCoord;
    output.texAtlas = input.texAtlas;
    return output;
}

Sampler2D texture;

[shader("fragment")]
float4 fragMain(VertexOutput input) : SV_Target
{
    float2 atlasUV = input.texAtlas.xy + frac(input.texCoord) * input.texAtlas.zw;
    float4 texel = texture.Sample(atlasUV);
    return float4(texel.rgb * input.color, texel.a);
}
