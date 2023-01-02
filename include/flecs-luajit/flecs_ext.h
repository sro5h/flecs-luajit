#ifndef FLECS_LUAJIT_FLECS_EXT_H
#define FLECS_LUAJIT_FLECS_EXT_H

#include "config.h"

#include <flecs.h>

#ifdef __cplusplus
extern "C" {
#endif

FLECS_LUAJIT_API
int32_t ecs_ext_iter_term_count(
        ecs_iter_t const* iter);

FLECS_LUAJIT_API
int16_t ecs_ext_vector_offset(
        ecs_size_t alignment);

#ifdef __cplusplus
}
#endif

#endif
