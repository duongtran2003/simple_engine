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
  float3 normal = normalize(inVert.normal);

  LightProperty directionalLight = calculateDirectionalLight(ubo.directionalLightDirection, normal);

  LightProperty pointLight = calculatePointLight(ubo.pointLightPosition, ubo.cameraPos, inVert.worldSpacePos, normal, 128.0f);

  float3 ambient = directionalLight.ambient * ubo.directionalLightColor;
  float3 diffuse = directionalLight.diffuse * ubo.directionalLightColor + pointLight.diffuse * ubo.pointLightColor;
  float3 specular = pointLight.specular * ubo.pointLightColor;

  float3 color = texColor.rgb * (ambient + diffuse) + specular;
  return float4(color, texColor.a);
}

float calculateAttenuation(float constant, float linear, float quadratic, float distance) {
  float attenuation = 1.0f / (constant + linear * distance + quadratic * distance * distance);

  return attenuation;
}

struct LightProperty {
  float ambient = 0.0f;
  float diffuse = 0.0f;
  float specular = 0.0f;
};

LightProperty calculateDirectionalLight(float3 lightDir, float3 normal) {
  float3 invertedLightDir = normalize(-lightDir);
  float diffuseStrength = max(dot(invertedLightDir, normal), 0.0f);

  LightProperty output;
  output.diffuse = diffuseStrength * 0.2f;
  output.ambient = 0.01f;

  return output;
}

LightProperty calculatePointLight(float3 lightPos, float3 viewPos, float3 fragPos, float3 normal, float shininess) {
  float3 invertedLightDir = normalize(lightPos - fragPos);
  float diffuseStrength = max(dot(invertedLightDir, normal), 0.0f);

  float specularStrength = 0.0f;
  if (diffuseStrength > 0) {
    float3 invertedViewDir = normalize(viewPos - fragPos);
    float3 halfVec = normalize(invertedLightDir + invertedViewDir);
    specularStrength = pow(max(dot(normal, halfVec), 0.0f), shininess);
  }

  float attenuation = calculateAttenuation(1.0f, 0.007f, 0.0002f, length(lightPos - fragPos));
  LightProperty output;
  output.diffuse = diffuseStrength * attenuation * 0.6f;
  output.specular = specularStrength * attenuation;

  return output;
}
