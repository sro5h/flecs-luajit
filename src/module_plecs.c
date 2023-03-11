#include "../include/flecs-luajit/module_plecs.h"

ECS_TAG_DECLARE(EcsLuajitSystem);
ECS_COMPONENT_DECLARE(EcsLuajitSystemDesc);

static ECS_MOVE(EcsLuajitSystemDesc, dst, src, {
    ecs_os_free(dst->expr);
    dst->expr = src->expr;
    src->expr = NULL;
    dst->interval = src->interval;
    src->interval = 0;
    dst->rate = src->rate;
    src->rate = 0;
    dst->tick_source = src->tick_source;
    src->tick_source = 0;
    dst->multi_threaded = src->multi_threaded;
    src->multi_threaded = false;
    dst->no_readonly = src->no_readonly;
    src->no_readonly = false;
})

ECS_COPY(EcsLuajitSystemDesc, dst, src, {
    ecs_os_strset(&dst->expr, src->expr);
    dst->interval = src->interval;
    dst->rate = src->rate;
    dst->tick_source = src->tick_source;
    dst->multi_threaded = src->multi_threaded;
    dst->no_readonly = src->no_readonly;
})

ECS_DTOR(EcsLuajitSystemDesc, ptr, {
    ecs_os_free(ptr->expr);
})

void FlecsLuajitPlecsImport(
        ecs_world_t* world) {
    ECS_MODULE(world, FlecsLuajitPlecs);
    ECS_IMPORT(world, FlecsLuajitBase);

    ecs_set_name_prefix(world, "EcsLuajit");

    ECS_TAG_DEFINE(EcsLuajitSystem);
    ECS_COMPONENT_DEFINE(EcsLuajitSystemDesc);

    ecs_set_hooks(world, EcsLuajitSystemDesc, {
        .ctor = ecs_default_ctor,
        .move = ecs_move(EcsLuajitSystemDesc),
        .copy = ecs_copy(EcsLuajitSystemDesc),
        .dtor = ecs_dtor(EcsLuajitSystemDesc),
    });
}
