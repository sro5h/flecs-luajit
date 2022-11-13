#include <flecs-luajit/module.h>
#include <flecs.h>

int main(void) {
        ecs_world_t* world = ecs_init();
        ECS_IMPORT(world, FlecsLuajit);

        return ecs_fini(world);
}
