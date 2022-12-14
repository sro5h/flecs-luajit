#ifndef FLECS_LUAJIT_MODULE_BASE_H
#define FLECS_LUAJIT_MODULE_BASE_H

#include "config.h"
#include "host.h"

#include <flecs.h>
#include <lua.h>

FLECS_LUAJIT_API
extern ECS_COMPONENT_DECLARE(EcsLuajitHost);

typedef ecs_luajit_host_t EcsLuajitHost;

typedef struct ecs_luajit_run_desc_t {
    ecs_luajit_script_t script;
    bool once;
} ecs_luajit_run_desc_t;

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

typedef struct ecs_luajit_observer_desc_t {
    ecs_entity_t entity;
    ecs_filter_desc_t filter;
    ecs_entity_t events[ECS_OBSERVER_DESC_EVENT_COUNT_MAX];
    bool yield_existing;
    char const* callback;
    void* ctx;
    ecs_ctx_free_t ctx_free;
    ecs_poly_t* observable;
    int32_t* last_event_id;
} ecs_luajit_observer_desc_t;

#ifdef __cplusplus
extern "C" {
#endif

FLECS_LUAJIT_API
void ecs_luajit_init(
        ecs_world_t* world);

FLECS_LUAJIT_API
bool ecs_luajit_run(
        ecs_world_t* world,
        ecs_luajit_run_desc_t const* desc);

FLECS_LUAJIT_API
ecs_entity_t ecs_luajit_system_init(
        ecs_world_t* world,
        ecs_luajit_system_desc_t const* desc);

FLECS_LUAJIT_API
ecs_entity_t ecs_luajit_observer_init(
        ecs_world_t* world,
        ecs_luajit_observer_desc_t const* desc);

// TODO: FlecsLuajitConfig would be prettier but that won't work with the current
// import function that just checks whether an entity already exists and not
// whether it has the EcsModule tag.
FLECS_LUAJIT_API
void FlecsLuajitConfigImport(
        ecs_world_t* world);

FLECS_LUAJIT_API
void FlecsLuajitBaseImport(
        ecs_world_t* world);

#ifdef __cplusplus
}
#endif

#endif
