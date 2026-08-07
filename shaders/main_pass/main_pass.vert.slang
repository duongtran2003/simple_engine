struct VSInput {
  [[vk::location(0)]] float3 inPosition;
  [[vk::location(1)]] float3 inNormal;
  [[vk::location(2)]] float2 inTexCoords;
  [[vk::location(3)]] float4 inTangent;
};

struct UniformBuffer {
  float4x4 view;
  float4x4 proj;
  float3 cameraPos;

  float directionalLightIntensity;
  float3 directionalLightDirection;
  float3 directionalLightColor;

  float pointLightIntensity;
  float3 pointLightPosition;
  float3 pointLightColor;
};
[[vk::binding(0, 0)]]
ConstantBuffer<UniformBuffer> ubo;

struct PushConstants {
  uint albedoIndex;
  uint normalIndex;
  uint metallicRoughnessIndex;
  uint uniformIndex;
};
[[vk::push_constant]] PushConstants push;

struct ObjectData {
  float4x4 model;
  float4x4 normalModel;
  float4 pbrBaseColorFactor;
  float pbrMetallicFactor;
  float pbrRoughnessFactor;
};
[[vk::binding(0, 2)]]
StructuredBuffer<ObjectData> objectBuffers;

struct VSOutput {
  float4 pos: SV_Position;
  float3 normal;
  float4 tangent;
  float2 texCoords;
  float3 worldSpacePos;
};

[shader("vertex")]
VSOutput vertMain(VSInput input) {
  VSOutput output;
  float4x4 mvp = mul(ubo.proj, mul(ubo.view, objectBuffers[push.uniformIndex].model));
  output.pos = mul(mvp, float4(input.inPosition, 1.0f));
  output.worldSpacePos = mul(objectBuffers[push.uniformIndex].model, float4(input.inPosition, 1.0f)).xyz;

  output.normal = normalize(mul(float3x3(objectBuffers[push.uniformIndex].normalModel), input.inNormal));
  output.tangent = float4(normalize(mul(float3x3(objectBuffers[push.uniformIndex].normalModel), input.inTangent.xyz)), input.inTangent.w);
  output.texCoords = input.inTexCoords;

  return output;
}
