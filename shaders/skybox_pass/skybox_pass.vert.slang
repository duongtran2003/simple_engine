struct VSInput {};

struct SkyboxVSOutput {
  float4 pos: SV_Position;
  float3 inDirection;
};

struct UniformBuffer {
  float4x4 view;
  float4x4 proj;
  float3 cameraPos;

  float3 directionalLightDirection;
  float3 directionalLightColor;

  float3 pointLightPosition;
  float3 pointLightColor;
};
[[vk::binding(0, 0)]]
ConstantBuffer<UniformBuffer> ubo;

[shader("vertex")]
SkyboxVSOutput vertMain(uint vertexID: SV_VertexID) {

  // Hard-coded skybox for now
  const float3 positions[8] = {
    float3(-1.0, -1.0, -1.0), float3( 1.0, -1.0, -1.0),
    float3( 1.0,  1.0, -1.0), float3(-1.0,  1.0, -1.0),
    float3(-1.0, -1.0,  1.0), float3( 1.0, -1.0,  1.0),
    float3( 1.0,  1.0,  1.0), float3(-1.0,  1.0,  1.0)
  };

  const uint indices[36] = {
    0, 2, 1, 2, 0, 3,  // -Z
    4, 5, 6, 6, 7, 4,  // +Z
    0, 1, 5, 5, 4, 0,  // -Y
    2, 3, 7, 7, 6, 2,  // +Y
    0, 4, 7, 7, 3, 0,  // -X
    1, 2, 6, 6, 5, 1   // +X
  };

  uint index = indices[vertexID];
  float3 inPos = positions[index];

  float3x3 noTranslationView = float3x3(ubo.view);
  float4 pos = mul(ubo.proj, float4(mul(noTranslationView, inPos), 1.0f));

  SkyboxVSOutput output;
  output.pos = pos.xyww;
  output.inDirection = inPos;
  
  return output;
}
