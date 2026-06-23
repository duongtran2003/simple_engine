struct VSOutput {
  float4 pos: SV_Position;
  float3 normal;
  float2 texCoords;
};

[shader("fragment")]
float4 fragMain(VSOutput inVert) : SV_Target {
  float3 color = inVert.normal;
  return float4(color, 1.0);
}

