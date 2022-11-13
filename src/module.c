#include "../include/flecs-luajit/module.h"

ECS_COMPONENT_DECLARE(EcsLuajitHost);

void FlecsLuajitImport(
                ecs_world_t* world) {
        ECS_MODULE(world, FlecsLuajit);

        ecs_set_name_prefix(world, "EcsLuajit");

        ECS_COMPONENT_DEFINE(world, EcsLuajitHost);
}
