struct VertexInput
{
    float2 position;
    float3 color;
    float2 texCoord;
};

struct UniformBuffer {
    float4x4 model;
    float4x4 view;
    float4x4 projection;
};
ConstantBuffer<UniformBuffer> mvp;

struct VertexOutput
{
    float4 position : SV_Position;
    float3 color;
    float2 texCoord;
};

[shader("vertex")]
VertexOutput vertMain(VertexInput input)
{
    VertexOutput output;
    output.position = mul(mvp.projection, mul(mvp.view, mul(mvp.model, float4(input.position, 0.0, 1.0))));
    output.color = input.color;
    output.texCoord = input.texCoord;
    return output;
}

Sampler2D texture;

[shader("fragment")]
float4 fragMain(VertexOutput input) : SV_Target
{
    return texture.Sample(input.texCoord);
}