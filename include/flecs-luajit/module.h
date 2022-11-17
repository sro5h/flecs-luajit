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

typedef struct ecs_luajit_system_desc_t {
        ecs_entity_t entity;
        ecs_query_desc_t query;
        char const* callback;
        void* ctx;
        ecs_ctx_free_t ctx_free;
        ecs_ftime_t interval;
        int32_t rate;
        ecs_entity_t tick_source;
        bool multi_threaded;
        bool no_readonly;
} ecs_luajit_system_desc_t;

#ifdef __cplusplus
extern "C" {
#endif

FLECS_LUAJIT_API
void ecs_luajit_ensure_stages(
                ecs_world_t* world);

FLECS_LUAJIT_API
ecs_entity_t ecs_luajit_system_init(
                ecs_world_t* world,
                ecs_luajit_system_desc_t const* desc);

FLECS_LUAJIT_API
void FlecsLuajitImport(
                ecs_world_t* world);

#ifdef __cplusplus
}
#endif

#endif
