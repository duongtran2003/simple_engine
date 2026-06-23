struct VSOutput {
  float4 pos: SV_Position;
  float3 normal;
  float2 texCoords;
};

struct UniformBuffer {
  float4x4 model;
  float4x4 view;
  float4x4 proj;
  float3 lightDirection;
  float3 objectColor;
};

[[vk::binding(0, 0)]]
ConstantBuffer<UniformBuffer> ubo;

[shader("fragment")]
float4 fragMain(VSOutput inVert) : SV_Target {
  float3 color = ubo.objectColor;

  float3 invertedLightDirection = normalize(-ubo.lightDirection);
  float3 normal = normalize(inVert.normal);
  float angle = degrees(acos(dot(invertedLightDirection, normal)));
  float diffuseStrength = (90.0f - angle) / 90.0f;
  return float4(color * diffuseStrength, 1.0);
}

