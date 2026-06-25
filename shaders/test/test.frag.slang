struct VSOutput {
  float4 pos: SV_Position;
  float3 normal;
  float2 texCoords;
};

struct UniformBuffer {
  float4x4 model;
  float4x4 normalModel;
  float4x4 view;
  float4x4 proj;
  float3 lightDirection;
  float3 lightColor;
};

[[vk::binding(0, 0)]]
ConstantBuffer<UniformBuffer> ubo;

[[vk::binding(0, 1)]]
Sampler2D albedoTexture;

[shader("fragment")]
float4 fragMain(VSOutput inVert) : SV_Target {
  float4 texColor = albedoTexture.Sample(inVert.texCoords);

  float3 lightColor = ubo.lightColor;
  float3 invertedLightDirection = normalize(-ubo.lightDirection);
  float3 normal = normalize(inVert.normal);

  float diffuseStrength = max(dot(invertedLightDirection, normal), 0.0f);
  float3 ambient = lightColor * 0.01f;
  float3 diffuse = lightColor * diffuseStrength;

  return float4(texColor * float4((ambient + diffuse), 1.0f));
}

