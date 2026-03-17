#include "../../Headers/Systems/Debugger.h"
#include "../../config.h"

static void screen_to_ndc(float screenX, float screenY, float* ndcX, float* ndcY) {
    *ndcX = (screenX / (SCREEN_WIDTH * 0.5f)) - 1.0f;
    *ndcY = 1.0f - (screenY / (SCREEN_HEIGHT * 0.5f));
}

static bool push_line(DebugVertex* out, Uint32 max, Uint32* count,
                      float x1, float y1, float x2, float y2,
                      float r, float g, float b, float a) {
    if (*count + 2 > max) {
        return false;
    }

    out[*count + 0] = (DebugVertex){ x1, y1, r, g, b, a };
    out[*count + 1] = (DebugVertex){ x2, y2, r, g, b, a };
    *count += 2;
    return true;
}

static void push_rect_outline(DebugVertex* out, Uint32 max, Uint32* count,
                              float screenX, float screenY, float w, float h,
                              float r, float g, float b, float a) {
    float x1 = screenX;
    float y1 = screenY;
    float x2 = screenX + w;
    float y2 = screenY + h;

    float ax, ay, bx, by;

    screen_to_ndc(x1, y1, &ax, &ay);
    screen_to_ndc(x2, y1, &bx, &by);
    push_line(out, max, count, ax, ay, bx, by, r, g, b, a);

    screen_to_ndc(x2, y1, &ax, &ay);
    screen_to_ndc(x2, y2, &bx, &by);
    push_line(out, max, count, ax, ay, bx, by, r, g, b, a);

    screen_to_ndc(x2, y2, &ax, &ay);
    screen_to_ndc(x1, y2, &bx, &by);
    push_line(out, max, count, ax, ay, bx, by, r, g, b, a);

    screen_to_ndc(x1, y2, &ax, &ay);
    screen_to_ndc(x1, y1, &bx, &by);
    push_line(out, max, count, ax, ay, bx, by, r, g, b, a);
}

Uint32 debugger_build_vertices(const Player* player, const WallsList* walls,
                               DebugVertex* outVertices, Uint32 maxVertices) {
    Uint32 count = 0;

    for (int i = 0; i < walls->count; i++) {
        const float x = walls->items[i].position.x;
        const float y = walls->items[i].position.y;
        const float w = walls->items[i].scale.x;
        const float h = walls->items[i].scale.y;

        const float screenX = (x + SCREEN_WIDTH / 2.0f) - w / 2.0f;
        const float screenY = (SCREEN_HEIGHT / 2.0f - y) - h / 2.0f;

        push_rect_outline(
            outVertices, maxVertices, &count,
            screenX, screenY, w, h,
            walls->items[i].color.x / 255.0f,
            walls->items[i].color.y / 255.0f,
            walls->items[i].color.z / 255.0f,
            1.0f
        );
    }

    {
        const float x = player->position.x;
        const float y = player->position.y;
        const float a = player->scale;

        const float screenX = (x + SCREEN_WIDTH / 2.0f) - a / 2.0f;
        const float screenY = (SCREEN_HEIGHT / 2.0f - y) - a / 2.0f;

        push_rect_outline(outVertices, maxVertices, &count,
                          screenX, screenY, a, a,
                          1.0f, 1.0f, 0.2f, 1.0f);
    }

    return count;
}