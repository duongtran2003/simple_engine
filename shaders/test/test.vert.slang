struct VSInput {
  [[vk::location(0)]] float3 inPosition;
  [[vk::location(1)]] float3 inNormal;
  [[vk::location(2)]] float2 inTexCoords;
};

struct UniformBuffer {
  float4x4 model;
  float4x4 normalModel;
  float4x4 view;
  float4x4 proj;
  float3 lightDirection;
  float3 objectColor;
};

[[vk::binding(0, 0)]]
ConstantBuffer<UniformBuffer> ubo;

struct VSOutput {
  float4 pos: SV_Position;
  float3 normal;
  float2 texCoords;
};

[shader("vertex")]
VSOutput vertMain(VSInput input) {
  VSOutput output;
  float4x4 mvp = mul(ubo.proj, mul(ubo.view, ubo.model));
  output.pos = mul(mvp, float4(input.inPosition, 1.0f));

  // This transforms normal to world space
  output.normal = normalize(mul(ubo.normalModel, float4(input.inNormal, 0.0f)).xyz);
  output.texCoords = input.inTexCoords;

  return output;
}
