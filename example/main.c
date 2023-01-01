#include <flecs-luajit/module.h>
#include <flecs.h>

int main(void) {
        ecs_world_t* world = ecs_init();
        ecs_set_threads(world, 2);

        ECS_IMPORT(world, FlecsLuajitBase);

        ecs_progress(world, 0);

        return ecs_fini(world);
}
