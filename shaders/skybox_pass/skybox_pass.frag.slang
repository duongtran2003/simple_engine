struct SkyboxVSOutput {
  float4 pos: SV_Position;
  float3 inDirection;
};

[[vk::binding(1, 1)]]
SamplerCube skyboxes[];

[shader("fragment")]
float4 fragMain(SkyboxVSOutput input) : SV_Target {
  float3 sampleDirection = normalize(input.inDirection);
  return float4(skyboxes[0].Sample(sampleDirection));
}
