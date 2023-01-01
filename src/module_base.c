#include "../include/flecs-luajit/module_base.h"
#include "../include/flecs-luajit/util_lua.h"

#include "res/flecs.lua.h"
#include "res/flecs.cdef.lua.h"
#include "res/boot.lua.h"

#include <lualib.h>
#include <lauxlib.h>

// TODO: Fixes warning about `abort` not being declared, which should be fixed
//       on flecs' side
#include <stdlib.h>

ECS_COMPONENT_DECLARE(EcsLuajitHost);

static ECS_MOVE(EcsLuajitHost, dst, src, {
    ecs_luajit_host_fini(dst);
    dst->states = src->states;
    dst->count = src->count;
    *src = (EcsLuajitHost) { 0 };
})

static ECS_COPY(EcsLuajitHost, dst, src, {
    (void) dst; (void) src;
    ecs_abort(ECS_INVALID_OPERATION, "EcsLuajitHost cannot be copied");
})

static ECS_DTOR(EcsLuajitHost, ptr, {
    ecs_luajit_host_fini(ptr);
})

static void s_init_module_flecs_luajit(
        EcsLuajitHost const* host,
        int32_t stage_id,
        ecs_world_t* world) {
    lua_State* l = ecs_luajit_host_at(host, stage_id);
    luaX_stack_guard_prolog(l);

    lua_getglobal(l, "package");
    lua_getfield(l, -1, "loaded");

    lua_createtable(l, 0, 2);

    lua_pushinteger(l, stage_id);
    lua_setfield(l, -2, "stage_id");

    lua_pushlightuserdata(l, world);
    lua_setfield(l, -2, "world");

    lua_setfield(l, -2, "flecs.luajit");
    lua_pop(l, 2);

    luaX_stack_guard_epilog(l, 0);
}

static void s_init_module_flecs_cdef(
        EcsLuajitHost const* host,
        int32_t stage_id) {
    lua_State* l = ecs_luajit_host_at(host, stage_id);
    luaX_stack_guard_prolog(l);

    lua_getglobal(l, "package");
    lua_getfield(l, -1, "loaded");

    ecs_luajit_host_call(host, &(ecs_luajit_call_desc_t) {
        .stage_id = stage_id,
        .script = { g_flecs_file_flecs_cdef_lua },
        .results = 1,
    });

    lua_setfield(l, -2, "flecs.cdef");
    lua_pop(l, 2);

    luaX_stack_guard_epilog(l, 0);
}

static void s_init_module_flecs(
        EcsLuajitHost const* host,
        int32_t stage_id) {
    lua_State* l = ecs_luajit_host_at(host, stage_id);
    luaX_stack_guard_prolog(l);

    lua_getglobal(l, "package");
    lua_getfield(l, -1, "loaded");

    ecs_luajit_host_call(host, &(ecs_luajit_call_desc_t) {
        .stage_id = stage_id,
        .script = { g_flecs_file_flecs_lua },
        .results = 1,
    });

    lua_setfield(l, -2, "flecs");
    lua_pop(l, 2);

    luaX_stack_guard_epilog(l, 0);
}

void ecs_luajit_init(
        ecs_world_t* world) {
    ecs_assert(world != NULL, ECS_INVALID_PARAMETER, NULL);

    EcsLuajitHost* host = ecs_singleton_get_mut(world, EcsLuajitHost);
    ecs_luajit_host_init(host, ecs_get_stage_count(world));

    for (int32_t i = 0; i < ecs_luajit_host_count(host); ++ i) {
        s_init_module_flecs_luajit(host, i, world);
        s_init_module_flecs_cdef(host, i);
        s_init_module_flecs(host, i);

        ecs_luajit_host_call(host, &(ecs_luajit_call_desc_t) {
            .stage_id = i,
            .script = { g_flecs_file_boot_lua },
        });
    }

    ecs_singleton_modified(world, EcsLuajitHost);
}

bool ecs_luajit_run(
        ecs_world_t* world,
        ecs_luajit_run_desc_t const* desc) {
    ecs_assert(world != NULL, ECS_INVALID_PARAMETER, NULL);
    ecs_assert(desc != NULL, ECS_INVALID_PARAMETER, NULL);

    EcsLuajitHost const* host = ecs_singleton_get(world, EcsLuajitHost);
    bool result = true;

    if (desc->once) {
        result = ecs_luajit_host_call(host, &(ecs_luajit_call_desc_t) {
            .stage_id = 0,
            .script = desc->script,
        });
    } else { for (int32_t i = 0; i < ecs_luajit_host_count(host); ++ i) {
        result = result && ecs_luajit_host_call(host, &(ecs_luajit_call_desc_t) {
            .stage_id = i,
            .script = desc->script,
        });
    } }

    return result;
}

void FlecsLuajitConfigImport(
                ecs_world_t* world) {
        ECS_MODULE(world, FlecsLuajitConfig);

        ecs_set_name_prefix(world, "EcsLuajit");
}

void FlecsLuajitBaseImport(
                ecs_world_t* world) {
        ECS_MODULE(world, FlecsLuajitBase);
        ECS_IMPORT(world, FlecsLuajitConfig);

        ecs_set_name_prefix(world, "EcsLuajit");

        ECS_COMPONENT_DEFINE(world, EcsLuajitHost);

        ecs_set_hooks(world, EcsLuajitHost, {
            .ctor = ecs_default_ctor,
            .move = ecs_move(EcsLuajitHost),
            .copy = ecs_copy(EcsLuajitHost),
            .dtor = ecs_dtor(EcsLuajitHost),
        });
}