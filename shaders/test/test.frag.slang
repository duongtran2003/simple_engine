struct VSOutput {
  float4 pos: SV_Position;
  float3 normal;
  float2 texCoords;
  float3 worldSpacePos;
};

struct UniformBuffer {
  float4x4 model;
  float4x4 normalModel;
  float4x4 view;
  float4x4 proj;

  float3 directionalLightDirection;
  float3 directionalLightColor;

  float3 pointLightPosition;
  float3 pointLightColor;

  float3 cameraPos;
};

[[vk::binding(0, 0)]]
ConstantBuffer<UniformBuffer> ubo;

[[vk::binding(0, 1)]]
Sampler2D albedoTexture;

[shader("fragment")]
float4 fragMain(VSOutput inVert) : SV_Target {
  float4 texColor = albedoTexture.Sample(inVert.texCoords);

  float3 directionalLightColor = ubo.directionalLightColor;
  float3 invertedDirectionalLightDirection = normalize(-ubo.directionalLightDirection);
  float3 normal = normalize(inVert.normal);

  float directionalLightDiffuseStrength = max(dot(invertedDirectionalLightDirection, normal), 0.0f);

  float3 invertedPointLightDir = normalize(ubo.pointLightPosition - inVert.worldSpacePos);
  float pointLightDiffuseStrength = max(dot(invertedPointLightDir, normal), 0.0f);
  float pointLightSpecularStrength = 0.0f;

  if (directionalLightDiffuseStrength >= 0) {
    float3 invertedViewDir = normalize(ubo.cameraPos - inVert.worldSpacePos);
    float3 halfVec = normalize(invertedViewDir + invertedPointLightDir);
    float shininess = 32.0f;
    pointLightSpecularStrength = pow(max(dot(inVert.normal, halfVec), 0.0f), shininess);
  }

  float3 ambient = directionalLightColor * 0.01f;
  float3 diffuse = directionalLightColor * directionalLightDiffuseStrength * 0.2f + ubo.pointLightColor * pointLightDiffuseStrength * 0.5f;
  float3 specular = ubo.pointLightColor * pointLightSpecularStrength;

  return float4(texColor * float4((ambient + diffuse + specular), 1.0f));
}

