#include "../config.h"

struct Input {
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

struct RaySlice {
    float top;
    float bottom;
    float brightness;
    float distance;
};

StructuredBuffer<RaySlice> rays : register(t0, space2);

float4 main(Input input) : SV_Target0
{
    uint column = (uint)(input.uv.x * RAY_COUNT);
    if (column >= RAY_COUNT) column = RAY_COUNT - 1;

    RaySlice slice = rays[column];

    float4 sky = float4(0.45f, 0.70f, 1.0f, 1.0f);
    float4 floorCol = float4(0.20f, 0.20f, 0.20f, 1.0f);

    if (slice.distance == -1.0f) {
        return (input.uv.y < 0.5f) ? sky : floorCol;
    }

    if (input.uv.y < slice.top) {
        return sky;
    }

    if (input.uv.y > slice.bottom) {
        return floorCol;
    }

    return float4(slice.brightness, slice.brightness, slice.brightness, 1.0f);
}