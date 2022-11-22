#ifndef FLECS_LUAJIT_MODULE_H
#define FLECS_LUAJIT_MODULE_H

#include "config.h"

#include <flecs.h>
#include <lua.h>

FLECS_LUAJIT_API
extern ECS_COMPONENT_DECLARE(EcsLuajitConfig);

typedef struct EcsLuajitConfig {
        char* init_file;
} EcsLuajitConfig;

FLECS_LUAJIT_API
extern ECS_COMPONENT_DECLARE(EcsLuajitHost);

typedef struct EcsLuajitHost {
        lua_State** states;
        int32_t count;
} EcsLuajitHost;

FLECS_LUAJIT_API
extern ECS_COMPONENT_DECLARE(EcsLuajitSystem);

typedef struct EcsLuajitSystem {
        char* query_expr;
        char* callback;
        ecs_ftime_t interval;
        int32_t rate;
        ecs_entity_t tick_source;
        bool multi_threaded;
        bool no_readonly;
} EcsLuajitSystem;

FLECS_LUAJIT_API
extern ECS_COMPONENT_DECLARE(EcsLuajitScript);

// TODO: Add different script types for scripts that define lua values and thus
// should be executed on every stage and scripts running flecs functions that
// should only be called once.
typedef struct EcsLuajitScript {
        char* code;
} EcsLuajitScript;

FLECS_LUAJIT_API
extern ECS_COMPONENT_DECLARE(EcsLuajitOnStage);

typedef struct EcsLuajitOnStage {
        int32_t stage_id;
} EcsLuajitOnStage;

FLECS_LUAJIT_API
extern ECS_DECLARE(EcsLuajitLoaded);

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
void ecs_luajit_run(
                ecs_world_t* world,
                char const* code);

FLECS_LUAJIT_API
void ecs_luajit_run_on_stage(
                ecs_world_t* world,
                int32_t stage_id,
                char const* code);

FLECS_LUAJIT_API
int32_t ecs_luajit_iter_term_count(
                ecs_iter_t const* iter);

// TODO: FlecsLuajitConfig would be prettier but that won't work with the current
// import function that just checks whether an entity already exists and not
// whether it has the EcsModule tag.
FLECS_LUAJIT_API
void FlecsConfigLuajitImport(
                ecs_world_t* world);

FLECS_LUAJIT_API
void FlecsLuajitImport(
                ecs_world_t* world);

#ifdef __cplusplus
}
#endif

#endif
