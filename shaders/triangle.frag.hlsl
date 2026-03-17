cbuffer UniformBlock : register(b0, space3){
    float time;
};
struct Input {
    float4 v_color : COLOR0;
};
float4 main(Input input) : SV_Target0 {
    float pulse = sin(time * 2.0) * 0.5 + 0.5;
    return float4(input.v_color.rgb * (0.8 + pulse * 0.5), input.v_color.a);
}