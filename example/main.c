#include <flecs-luajit/module.h>
#include <flecs.h>

int main(void) {
        ecs_world_t* world = ecs_init();

        ECS_IMPORT(world, FlecsConfigLuajit);

        ecs_singleton_set(world, EcsLuajitConfig, {
                .init_file = "example/init.lua"
        });

        ECS_IMPORT(world, FlecsLuajit);

        ecs_entity_t LuajitSystem = ecs_entity_init(world, &(ecs_entity_desc_t) {
                .name = "LuajitSystem",
                .add = { ecs_dependson(EcsOnUpdate) },
        });

        ecs_set(world, LuajitSystem, EcsLuajitSystem, {
                .query_expr = "",
                .callback = "update",
        });

        ecs_set(world, LuajitSystem, EcsLuajitScript, {
                "function update(iter)\n"
                "       print('update()')\n"
                "end\n"
        });


        // Equivalent alternative
        /*ecs_luajit_system_init(world, &(ecs_luajit_system_desc_t) {
                .entity = LuajitSystem,
                .query.filter.expr = "",
                .callback = "update",
        });*/

        ecs_progress(world, 0);

        return ecs_fini(world);
}
