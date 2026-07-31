struct SkyboxVSOutput {
  float4 pos: SV_Position;
  float2 uv;
};

[[vk::binding(0, 3)]]
Sampler2D resources[];

float3 acesFilm(float3 x) {
  float a = 2.51f;
  float b = 0.03f;
  float c = 2.43f;
  float d = 0.59f;
  float e = 0.14f;
  return saturate((x * (a * x + b)) / (x * (c * x + d) + e));
}

float3 uncharted2TonemapPartial(float3 x) {
  float A = 0.15f;
  float B = 0.50f;
  float C = 0.10f;
  float D = 0.20f;
  float E = 0.02f;
  float F = 0.30f;
  return ((x*(A*x+C*B)+D*E)/(x*(A*x+B)+D*F))-E/F;
}

float3 uncharted2Filmic(float3 x) {
  float exposureBias = 4.0f;
  float3 curr = uncharted2TonemapPartial(x * exposureBias);

  float3 white = float3(11.2f);
  float3 whiteScale = float3(1.0f) / uncharted2TonemapPartial(white);

  return curr * whiteScale;
}

[shader("fragment")]
float4 fragMain(SkyboxVSOutput input) : SV_Target {
  float4 sampledColor = resources[1].Sample(input.uv);

  float3 color = uncharted2Filmic(sampledColor.xyz);

  return float4(color, sampledColor.a);
}
