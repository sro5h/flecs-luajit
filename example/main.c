#include <flecs-luajit/module.h>
#include <flecs.h>

int main(void) {
        ecs_world_t* world = ecs_init();

        ecs_set_threads(world, 2);

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
                "print('called system script')\n"
                "function update(iter)\n"
                "       print('update()')\n"
                "end\n"
        });

        ecs_entity_t Bob = ecs_entity_init(world, &(ecs_entity_desc_t) {
                .name = "Bob",
        });

        ecs_set(world, Bob, EcsLuajitScript, {
                "print('Hello from Bob on main stage')\n"
        });

        ecs_set_pair(world, Bob, EcsLuajitOnStage, ecs_id(EcsLuajitScript), {
                .stage_id = 0,
        });


        // Equivalent alternative
        /*ecs_luajit_system_init(world, &(ecs_luajit_system_desc_t) {
                .entity = LuajitSystem,
                .query.filter.expr = "",
                .callback = "update",
        });*/

        ecs_progress(world, 0);

        ecs_set(world, Bob, EcsLuajitScript, {
                "print('Hello again from Bob on the next tick')\n"
        });

        ecs_progress(world, 0);

        return ecs_fini(world);
}
