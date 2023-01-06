#include <flecs-luajit/module_base.h>
#include <flecs.h>

int main(void) {
    ecs_world_t* world = ecs_init();
    ecs_set_threads(world, 2);

    ECS_IMPORT(world, FlecsLuajitBase);

    ecs_luajit_init(world);

    ecs_luajit_run(world, &(ecs_luajit_run_desc_t) {
        .script = { .name = "example/init.lua" },
    });

    ecs_luajit_system_init(world, &(ecs_luajit_system_desc_t) {
        .entity = ecs_entity(world, {
            .name = "HelloAgain",
            .add = { ecs_dependson(EcsOnUpdate) },
        }),
        .query.filter.expr = "",
    });

    ecs_luajit_observer_init(world, &(ecs_luajit_observer_desc_t) {
        .entity = ecs_entity(world, {
            .name = "OnSetPosition",
        }),
        .filter.expr = "Position",
        .events = { EcsOnSet },
        .yield_existing = true,
    });

    ecs_progress(world, 0);

    return ecs_fini(world);
}
