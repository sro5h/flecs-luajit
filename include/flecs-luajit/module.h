#ifndef FLECS_LUAJIT_MODULE_H
#define FLECS_LUAJIT_MODULE_H

#include "config.h"

#include <flecs.h>
#include <lua.h>

FLECS_LUAJIT_API
extern ECS_COMPONENT_DECLARE(EcsLuajitHost);

typedef struct EcsLuajitHost {
        lua_State** states;
        int32_t count;
} EcsLuajitHost;

#ifdef __cplusplus
extern "C" {
#endif

FLECS_LUAJIT_API
void FlecsLuajitImport(
                ecs_world_t* world);

#ifdef __cplusplus
}
#endif

#endif
