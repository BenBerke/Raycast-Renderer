struct PSInput {
    float brightness : TEXCOORD0;
    float4 pos : SV_Position;
};

float4 main(PSInput input) : SV_Target0 {
    float3 baseColor = float3(0.0, 1.0, 0.0);
    return float4(baseColor * input.brightness, 1.0f);
}