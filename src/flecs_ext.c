#include "../include/flecs-luajit/flecs_ext.h"

#include <assert.h>

static int16_t s_vector_offset(
        ecs_size_t size,
        int16_t offset) {
    (void) size;
    return offset;
}

int16_t ecs_ext_vector_offset(
        ecs_size_t alignment) {
    return s_vector_offset(ECS_VECTOR_U(0, alignment));
}
