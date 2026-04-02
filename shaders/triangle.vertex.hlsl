cbuffer PosUniformBlock : register(b0, space1){
    float pos;
    float3 _pad;
};

struct Input
{
    float3 a_position : TEXCOORD0;
    float4 a_color : COLOR0;
};

struct Output
{
    float4 v_position : SV_Position;
    float4 v_color : COLOR0;
};

Output main(Input input)
{
    Output output;
    output.v_position = float4(input.a_position.x, input.a_position.y, input.a_position.z, 1.0f);
    output.v_color = input.a_color;
    return output;
}