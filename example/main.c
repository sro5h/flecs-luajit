#include <flecs-luajit/module.h>
#include <flecs.h>

int main(void) {
        ecs_world_t* world = ecs_init();
        ECS_IMPORT(world, FlecsLuajit);

        ecs_entity_t LuajitSystem = ecs_entity_init(world, &(ecs_entity_desc_t) {
                .name = "LuajitSystem",
                .add = { ecs_pair(EcsDependsOn, EcsOnUpdate), EcsOnUpdate },
        });

        ecs_luajit_system_init(world, &(ecs_luajit_system_desc_t) {
                .entity = LuajitSystem,
                .query.filter.expr = "",
                .callback = "update",
        });

        ecs_progress(world, 0);

        return ecs_fini(world);
}
