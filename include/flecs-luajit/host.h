#ifndef FLECS_LUAJIT_HOST_H
#define FLECS_LUAJIT_HOST_H

#include "config.h"

#include <flecs.h>
#include <lua.h>

typedef struct ecs_luajit_host_t {
    lua_State** states;
    int32_t count;
} ecs_luajit_host_t;

typedef struct ecs_luajit_script_t {
    char const* source;
    bool is_file;
} ecs_luajit_script_t;

typedef struct ecs_luajit_call_desc_t {
    int32_t stage_id;
    ecs_luajit_script_t script;
    int args;
    int results;
} ecs_luajit_call_desc_t;

FLECS_LUAJIT_API
void ecs_luajit_host_init(
        ecs_luajit_host_t* self,
        int32_t count);

FLECS_LUAJIT_API
void ecs_luajit_host_fini(
        ecs_luajit_host_t* self);

FLECS_LUAJIT_API
bool ecs_luajit_host_call(
        ecs_luajit_host_t const* self,
        ecs_luajit_call_desc_t const* desc);

FLECS_LUAJIT_API
int32_t ecs_luajit_host_count(
        ecs_luajit_host_t const* self);

FLECS_LUAJIT_API
lua_State* ecs_luajit_host_at(
        ecs_luajit_host_t const* self,
        int32_t stage_id);

#endif
