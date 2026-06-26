struct VSInput {
  [[vk::location(0)]] float3 inPosition;
  [[vk::location(1)]] float3 inNormal;
  [[vk::location(2)]] float2 inTexCoords;
};

struct UniformBuffer {
  float4x4 normalModel;
  float4x4 view;
  float4x4 proj;

  float3 directionalLightDirection;
  float3 directionalLightColor;

  float3 pointLightPosition;
  float3 pointLightColor;
};
[[vk::binding(0, 0)]]
ConstantBuffer<UniformBuffer> ubo;

struct PushConstants {
  float4x4 modelMatrix;
  float3 cameraPos;
  uint meshTextureIndex;
};
[[vk::push_constant]] PushConstants push;


struct VSOutput {
  float4 pos: SV_Position;
  float3 normal;
  float2 texCoords;
  float3 worldSpacePos;
};

[shader("vertex")]
VSOutput vertMain(VSInput input) {
  VSOutput output;
  float4x4 mvp = mul(ubo.proj, mul(ubo.view, push.modelMatrix));
  output.pos = mul(mvp, float4(input.inPosition, 1.0f));
  output.worldSpacePos = mul(push.modelMatrix, float4(input.inPosition, 1.0f)).xyz;

  output.normal = normalize(mul(float3x3(ubo.normalModel), input.inNormal));
  output.texCoords = input.inTexCoords;

  return output;
}
