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

#ifdef __cplusplus
}
#endif

#endif
