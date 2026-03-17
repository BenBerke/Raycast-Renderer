struct VSInput {
    float2 pos   : TEXCOORD0;
    float4 color : TEXCOORD1;
};

struct VSOutput {
    float4 color : TEXCOORD0;
    float4 pos   : SV_Position;
};

VSOutput main(VSInput input) {
    VSOutput output;
    output.pos = float4(input.pos, 0.0f, 1.0f);
    output.color = input.color;
    return output;
}