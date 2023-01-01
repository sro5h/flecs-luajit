#include <flecs-luajit/module_base.h>
#include <flecs.h>

int main(void) {
        ecs_world_t* world = ecs_init();
        ecs_set_threads(world, 2);

        ECS_IMPORT(world, FlecsLuajitBase);

        ecs_luajit_init(world);
        ecs_luajit_run(world, &(ecs_luajit_run_desc_t) {
            .script = { "example/init.lua", .is_file = true },
        });

        ecs_progress(world, 0);

        return ecs_fini(world);
}
