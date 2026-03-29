

struct Input
{
    float3 a_position : TEXCOORD0;
    float2 a_uv : TEXCOORD1;
};

struct Output
{
    float4 v_position : SV_Position;
    float2 v_uv : TEXCOORD0;
};

Output main(Input input)
{
    Output output;
    output.v_position = float4(input.a_position.x, input.a_position.y, input.a_position.z, 1.0f);
    output.v_uv = input.a_uv;
    return output;
}