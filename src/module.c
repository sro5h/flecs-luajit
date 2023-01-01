#include "../include/flecs-luajit/module.h"
#include "../include/flecs-luajit/util_lua.h"

#include <lualib.h>
#include <lauxlib.h>

void FlecsLuajitConfigImport(
                ecs_world_t* world) {
        ECS_MODULE(world, FlecsLuajitConfig);

        ecs_set_name_prefix(world, "EcsLuajit");
}

void FlecsLuajitBaseImport(
                ecs_world_t* world) {
        ECS_MODULE(world, FlecsLuajitBase);
        ECS_IMPORT(world, FlecsLuajitConfig);

        ecs_set_name_prefix(world, "EcsLuajit");
}
