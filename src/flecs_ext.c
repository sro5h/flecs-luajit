#include "../include/flecs-luajit/flecs_ext.h"

#include <assert.h>

int32_t ecs_ext_iter_term_count(
        ecs_iter_t const* iter) {
    ecs_filter_t const* filter = ecs_query_get_filter(iter->priv.iter.query.query);
    return filter->term_count;
}

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
