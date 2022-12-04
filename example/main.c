#include <flecs-luajit/module.h>
#include <flecs.h>

static char const* s_init_code =
        "package.path = package.path .. ';./external/flecs-luajit-binding/?.lua;./example/?.lua'\n"
        "require 'flecs_mixin'\n"
        "require 'flecs' { world = ..., cdef = require 'flecs_cdef' }\n"
;

int main(void) {
        ecs_world_t* world = ecs_init();

        ecs_set_threads(world, 2);

        ECS_IMPORT(world, FlecsConfigLuajit);

        ecs_singleton_set(world, EcsLuajitConfig, {
                .init_code = (char*) s_init_code,
                .init_file = "example/init.lua",
        });

        ECS_IMPORT(world, FlecsLuajit);

        ecs_luajit_ensure_stages(world);

        ecs_entity_t LuajitSystem = ecs_entity_init(world, &(ecs_entity_desc_t) {
                .name = "LuajitSystem",
                .add = { ecs_dependson(EcsOnUpdate) },
        });

        ecs_set(world, LuajitSystem, EcsLuajitSystem, {
                .query_expr = "Position",
                //.callback = "LuajitSystem",
        });

        ecs_set(world, LuajitSystem, EcsLuajitScript, {
                "print('called system script')\n"
                "return function(iter)\n"
                "       for e, p in iter:each() do\n"
                "               print('[e] ' .. iter:world():name(e))\n"
                "               print('  {' .. p.x .. ',' .. p.y .. '}')\n"
                "       end\n"
                "       print('LuajitSystem()')\n"
                "end\n"
                // Alternative:
                //"function LuajitSystem(iter)\n"
                //"       print('update()')\n"
                //"end\n"
        });

        ecs_entity_t Bob = ecs_entity_init(world, &(ecs_entity_desc_t) {
                .name = "Bob",
        });

        ecs_set(world, Bob, EcsLuajitScript, {
                "print('returning Bob\\'s name')\n"
                "return 'Bobo'\n"
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
                "print('Hello again from ' .. Bob .. ' on the next tick')\n"
        });

        ecs_progress(world, 0);

        return ecs_fini(world);
}
