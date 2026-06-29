struct VSOutput {
  float4 pos: SV_Position;
  float3 normal;
  float3 tangent;
  float2 texCoords;
  float3 worldSpacePos;
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

[[vk::binding(0, 1)]]
Sampler2D globalTextures[];

struct PushConstants {
  float4x4 modelMatrix;
  float3 cameraPos;
  uint albedoIndex;
  uint normalIndex;
  bool useNormalMap;
};
[[vk::push_constant]] PushConstants push;

[shader("fragment")]
float4 fragMain(VSOutput inVert) : SV_Target {
  float4 texColor = globalTextures[push.albedoIndex].Sample(inVert.texCoords);
  float3 normal;
  if (push.useNormalMap) {
    normal = normalize(globalTextures[push.normalIndex].Sample(inVert.texCoords).rgb);
    normal = normalize(normal * 2.0f - 1.0f);

    float3 normalizedInNormal = normalize(inVert.normal);

    float3 biTangent = cross(inVert.tangent, normalizedInNormal);
    float3x3 TBN = float3x3(
      inVert.tangent.x, biTangent.x, normalizedInNormal.x,
      inVert.tangent.y, biTangent.y, normalizedInNormal.y,
      inVert.tangent.z, biTangent.z, normalizedInNormal.z,
    );

    normal = normalize(mul(TBN, normal));
  } else {
    normal = normalize(inVert.normal);
  }

  LightProperty directionalLight = calculateDirectionalLight(ubo.directionalLightDirection, normal);

  LightProperty pointLight = calculatePointLight(ubo.pointLightPosition, push.cameraPos, inVert.worldSpacePos, normal, 128.0f);

  float3 ambient = directionalLight.ambient * ubo.directionalLightColor;
  float3 diffuse = directionalLight.diffuse * ubo.directionalLightColor + pointLight.diffuse * ubo.pointLightColor;
  float3 specular = pointLight.specular * ubo.pointLightColor;

  float3 color = texColor.rgb * (ambient + diffuse) + specular;
  float3 toneMapped = reinhardToneMap(color);
  return float4(toneMapped, texColor.a);
}

float3 reinhardToneMap(float3 color) {
  return color / (color + float3(1.0f));
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
  output.diffuse = diffuseStrength * 0.3f;
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
  output.diffuse = diffuseStrength * attenuation * 0.4f;
  output.specular = specularStrength * attenuation * 1.0f;

  return output;
}
