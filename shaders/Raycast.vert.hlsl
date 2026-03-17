struct RaySlice {
    float wallHeight;
    float u;
    int textureIndex;
    float brightness;
};

StructuredBuffer<RaySlice> slices : register(t0, space0);

struct VSInput {
    float3 pos : TEXCOORD0;
};

struct VSOutput {
    float brightness : TEXCOORD0;
    float4 pos : SV_Position;
};

VSOutput main(VSInput input, uint instanceID : SV_InstanceID) {
    VSOutput output;

    RaySlice data = slices[instanceID];

    float stripWidth = 2.0f / 320.0f;
    float xCenter = -1.0f + ((float(instanceID) + 0.5f) * stripWidth);
    float finalX = xCenter + (input.pos.x * stripWidth);

    float finalY = input.pos.y * (data.wallHeight / 480.0f);

    output.brightness = data.brightness;
    output.pos = float4(finalX, finalY, 0.0f, 1.0f);
    return output;
}