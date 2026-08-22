struct VSOutput {
  float4 pos: SV_Position;
  float3 normal;
  float4 tangent;
  float2 texCoords;
  float3 worldSpacePos;
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

[[vk::binding(0, 1)]]
Sampler2D globalTextures[];

struct ObjectData {
  float4x4 model;
  float4x4 normalModel;
  float4 pbrBaseColorFactor;
  float pbrMetallicFactor;
  float pbrRoughnessFactor;
};
[[vk::binding(0, 2)]]
StructuredBuffer<ObjectData> objectBuffers;

struct PushConstants {
  uint albedoIndex;
  uint normalIndex;
  uint metallicRoughnessIndex;
  uint uniformIndex;
  uint shadowMapIndex;
};
[[vk::push_constant]] PushConstants push;

[shader("fragment")]
float4 fragMain(VSOutput inVert) : SV_Target {
  float4 texColor = push.albedoIndex != 0 ? globalTextures[push.albedoIndex].Sample(inVert.texCoords) * objectBuffers[push.uniformIndex].pbrBaseColorFactor : objectBuffers[push.uniformIndex].pbrBaseColorFactor;
  float3 normal;
  if (push.normalIndex != 0) {
    normal = globalTextures[push.normalIndex].Sample(inVert.texCoords).rgb;
    normal = normalize(normal * 2.0f - 1.0f);

    float3 N = normalize(inVert.normal);
    // Gram-Schmidt orthogonal correction
    float3 T = float3(inVert.tangent.xyz) - dot(float3(inVert.tangent.xyz), N) * N;
    T = normalize(T);

    float3 B = cross(T, N) * inVert.tangent.w;
    B = normalize(B);

    float3x3 TBN = transpose(float3x3(T, B, N));
    normal = normalize(mul(TBN, normal));
  } else {
    normal = normalize(inVert.normal);
  }

  float3 viewDirection = normalize(ubo.cameraPos - inVert.worldSpacePos);
  float3 arm = globalTextures[push.metallicRoughnessIndex].Sample(inVert.texCoords).rgb;

  // Calculate directional light
  float3 lightDirection = normalize(-ubo.directionalLightDirection);
  float3 directionalBrdf = cookTorranceBRDF(texColor.rgb, normal, lightDirection, viewDirection, arm);
  float3 directionalRadiance = ubo.directionalLightIntensity * ubo.directionalLightColor;
  float3 directional = directionalBrdf * max(dot(normal, lightDirection), 0.0f) * directionalRadiance;

  // Calculate point light
  float3 pointLightDirection = normalize(ubo.pointLightPosition - inVert.worldSpacePos);

  float3 point = float3(0.0f);
  float nDotL = max(dot(normal, pointLightDirection), 0.0f);
  if (nDotL > 0.0f) {
    float3 pointBrdf = cookTorranceBRDF(texColor.rgb, normal, pointLightDirection, viewDirection, arm);
    float distance = length(ubo.pointLightPosition - inVert.worldSpacePos);
    float3 radiance = ubo.pointLightIntensity * (1.0f / (distance * distance));
    point = pointBrdf * ubo.pointLightColor * radiance * nDotL;
  }

  float3 color = directional + point;
  return float4(color, texColor.a);
}

float3 fLambert(float3 c) {
  return c / float.getPi();
}

float distributionGGX(float3 n, float3 h, float a) {
  float a2 = a * a;
  float nDotH = max(dot(n, h), 0.0f);
  float nDotH2 = nDotH * nDotH;

  float nom = a2;
  float denom = nDotH2 * (a2 - 1.0f) + 1.0f;
  denom = float.getPi() * denom * denom;
  denom = max(denom, 0.000001f);

  return nom / denom;
}

float geometrySchlickGGX(float3 n, float3 v, float k) {
  float nv = max(dot(n, v), 0.0f);
  float nom = nv;
  float denom = nv * (1.0f - k) + k;
  denom = max(denom, 0.000001f);

  return nom / denom;
}

float geometrySmith(float3 n, float3 v, float3 l, float k) {
  return geometrySchlickGGX(n, v, k) * geometrySchlickGGX(n, l, k);
}

float3 fresnelSchlick(float3 h, float3 v, float3 f0) {
  float oneMinusHDotV = 1.0f - max(dot(h, v), 0.0f);
  float oneMinusHDotVP5 = oneMinusHDotV * oneMinusHDotV * oneMinusHDotV * oneMinusHDotV * oneMinusHDotV;

  return f0 + (1.0f - f0) * oneMinusHDotVP5;
}

float3 cookTorranceBRDF(float3 color, float3 n, float3 l, float3 v, float3 arm) {
  float3 h = normalize(v + l);

  float metallic = objectBuffers[push.uniformIndex].pbrMetallicFactor * arm.b;
  float roughness = objectBuffers[push.uniformIndex].pbrRoughnessFactor * arm.g;
  float3 black = float3(0.0f);

  float3 cDiff = lerp(color, black, metallic);
  float3 f0 = lerp(float3(0.04f), color, metallic);
  float a = roughness * roughness;
  float k = a / 2.0f;

  float D = distributionGGX(n, h, a);
  float3 F = fresnelSchlick(h, v, f0);
  float G = geometrySmith(n, v, l, k);

  float3 kS = F;
  float3 kD = float3(1.0f) - kS;

  float3 fDiffuse = kD * fLambert(cDiff);

  float3 specularDenom = (4.0f * max(dot(v, n), 0.0f) * max(dot(l, n), 0.0f));
  specularDenom = max(specularDenom, 0.000001f);
  float3 fSpecular = (F * G * D) / specularDenom;

  return fDiffuse + fSpecular;
}
