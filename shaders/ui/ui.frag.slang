[[vk::binding(0,0)]]
Sampler2D fontTexture;

struct VSOutput {
  float4 pos: SV_Position;
  float2 texCoords;
  float4 color : COLOR0;
};

[shader("fragment")]
float4 fragMain(VSOutput input) {
  return input.color * fontTexture.Sample(input.texCoords);
}
