struct PushConstants {
  float2 scale;
  float2 translate;
};
[[vk::push_constant]] PushConstants push;

struct VSInput {
  [[vk::location(0)]] float2 inPosition;
  [[vk::location(1)]] float2 inTexCoords;
  [[vk::location(2)]] float4 inColor;
};

struct VSOutput {
  float4 pos: SV_Position;
  float2 texCoords;
  float4 color : COLOR0;
};

[shader("vertex")]
VSOutput vertMain(VSInput input) {
  VSOutput output;

  output.texCoords = input.inTexCoords;
  output.color = input.inColor;

  output.pos.x = input.inPosition.x * push.scale.x + push.translate.x;
  output.pos.y = input.inPosition.y * push.scale.y + push.translate.y;
  output.pos.z = 0.0f;
  output.pos.w = 1.0f;

  return output;
}
