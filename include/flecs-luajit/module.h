#ifndef FLECS_LUAJIT_MODULE_H
#define FLECS_LUAJIT_MODULE_H

#include "config.h"

#include <flecs.h>
#include <lua.h>

#ifdef __cplusplus
extern "C" {
#endif

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
