struct VSInput {};

struct SkyboxVSOutput {
  float4 pos: SV_Position;
  float2 uv;
};

[shader("vertex")]
SkyboxVSOutput vertMain(uint vertexID: SV_VertexID) {

  // Quad position
  const float2 positions[4] = {
    float2(-1.0f, -1.0f), float2( 1.0f, -1.0f),
    float2( 1.0f,  1.0f), float2(-1.0f,  1.0f)
  };
  const float2 uv[4] = {
    float2(0.0f, 0.0f), float2(1.0f, 0.0f),
    float2(1.0f, 1.0f), float2(0.0f, 1.0f)
  };

  const uint indices[6] = {
    2, 1, 0, 0, 3, 2
  };

  uint index = indices[vertexID];
  float2 inPos = positions[index];
  float2 inUv = uv[index];

  SkyboxVSOutput output;
  output.pos = float4(inPos, 0.0f, 1.0f);
  output.uv = inUv;  

  return output;
}
