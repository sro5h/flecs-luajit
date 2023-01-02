#include "../include/flecs-luajit/flecs_ext.h"

int32_t ecs_ext_iter_term_count(
        ecs_iter_t const* iter) {
    ecs_filter_t const* filter = ecs_query_get_filter(iter->priv.iter.query.query);
    return filter->term_count;
}
