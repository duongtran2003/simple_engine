struct SkyboxVSOutput {
  float4 pos: SV_Position;
  float2 uv;
};

[[vk::binding(0, 2)]]
Sampler2D resources[];

[shader("fragment")]
float4 fragMain(SkyboxVSOutput input) : SV_Target {
  return float4(1.0f, 1.0f, 1.0f, 1.0f);
}
