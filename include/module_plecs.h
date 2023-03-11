#ifndef FLECS_LUAJIT_MODULE_PLECS_H
#define FLECS_LUAJIT_MODULE_PLECS_H

#include "config.h"

FLECS_LUAJIT_API
extern ECS_DECLARE(EcsLuajitSystem);

FLECS_LUAJIT_API
extern ECS_COMPONENT_DECLARE(EcsLuajitSystemDesc);

typedef struct EcsLuajitSystemDesc {
    char* expr;
    ecs_ftime_t interval;
    int32_t rate;
    ecs_entity_t tick_source;
    bool multi_threaded;
    bool no_readonly;
} EcsLuajitSystemDesc;

#ifdef __cplusplus
extern "C" {
#endif

FLECS_LUAJIT_API
void FlecsLuajitPlecsImport(
        ecs_world_t* world);

#ifdef __cplusplus
}
#endif

#endif
