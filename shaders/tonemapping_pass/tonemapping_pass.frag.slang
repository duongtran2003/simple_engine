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

[shader("fragment")]
float4 fragMain(SkyboxVSOutput input) : SV_Target {
    float4 sampledColor = resources[1].Sample(input.uv);

    float exposure = 1.5f; 
    float3 hdrColor = sampledColor.rgb * exposure;

    float3 sdrLinear = acesFilm(hdrColor);

    return float4(sdrLinear, sampledColor.a);
}
