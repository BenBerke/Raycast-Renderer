#pragma once

#include <SDL3/SDL.h>
#include "../AppState.h"
#include "Renderer.h"
#include "../Objects/Player.h"
#include "Physics.h"

Uint32 debugger_build_vertices(const Player* player, const WallsList* walls,
                               DebugVertex* outVertices, Uint32 maxVertices);