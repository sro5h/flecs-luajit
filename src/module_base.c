#include "../include/flecs-luajit/module_base.h"
#include "../include/flecs-luajit/lua_ext.h"

#include "res/flecs.lua.h"
#include "res/boot.lua.h"
#include "res/cdef.lua.h"
#include "res/glue.lua.h"

#include <lualib.h>
#include <lauxlib.h>

// TODO: Fixes warning about `abort` not being declared, which should be fixed
//       on flecs' side
#include <stdlib.h>

ECS_COMPONENT_DECLARE(EcsLuajitHost);

static char const* s_key_system_run = "flecs.luajit.system_run";
static char const* s_key_observer_callback = "flecs.luajit.observer_callback";

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

    ecs_luajit_host_call(host, &(ecs_luajit_call_desc_t) {
        .stage_id = stage_id,
        .script = { .name = "@cdef.lua", .source = g_flecs_file_cdef_lua },
        .results = 1,
    });
    lua_setfield(l, -2, "cdef");

    lua_setfield(l, -2, "flecs.luajit");
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
        .script = { .name = "@flecs.lua", .source = g_flecs_file_flecs_lua },
        .results = 1,
    });

    lua_setfield(l, -2, "flecs");
    lua_pop(l, 2);

    luaX_stack_guard_epilog(l, 0);
}

void s_init_registry_refs(
        EcsLuajitHost const* host,
        int32_t stage_id) {
    lua_State* l = ecs_luajit_host_at(host, stage_id);
    luaX_stack_guard_prolog(l);

    lua_getglobal(l, "package");
    lua_getfield(l, -1, "loaded");
    lua_getfield(l, -1, "flecs");
    lua_getfield(l, -1, "luajit");
    lua_getfield(l, -1, "system_run");
    lua_setfield(l, LUA_REGISTRYINDEX, s_key_system_run);
    lua_getfield(l, -1, "observer_callback");
    lua_setfield(l, LUA_REGISTRYINDEX, s_key_observer_callback);
    lua_pop(l, 4);

    luaX_stack_guard_epilog(l, 0);
}

void ecs_luajit_init(
        ecs_world_t* world) {
    ecs_assert(world != NULL, ECS_INVALID_PARAMETER, NULL);

    EcsLuajitHost* host = ecs_singleton_get_mut(world, EcsLuajitHost);
    ecs_luajit_host_init(host, ecs_get_stage_count(world));

    for (int32_t i = 0; i < ecs_luajit_host_count(host); ++ i) {
        s_init_module_flecs_luajit(host, i, world);
        s_init_module_flecs(host, i);

        ecs_luajit_host_call(host, &(ecs_luajit_call_desc_t) {
            .stage_id = i,
            .script = { .name = "@boot.lua", .source = g_flecs_file_boot_lua },
        });

        ecs_luajit_host_call(host, &(ecs_luajit_call_desc_t) {
            .stage_id = i,
            .script = { .name = "@glue.lua", .source = g_flecs_file_glue_lua },
        });

        s_init_registry_refs(host, i);
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

static void s_system_run(
        ecs_iter_t* iter) {
    EcsLuajitHost const* host = ecs_singleton_get(iter->world, EcsLuajitHost);

    lua_State* l = ecs_luajit_host_at(host, ecs_get_stage_id(iter->world));
    char const* callback = iter->binding_ctx;

    luaX_stack_guard_prolog(l);

    lua_getfield(l, LUA_REGISTRYINDEX, s_key_system_run);
    lua_pushstring(l, callback);
    // Push as lightuserdata, use `ffi.cast` to obtain typed pointer
    // https://www.freelists.org/post/luajit/Transferring-cdata-between-states,7
    lua_pushlightuserdata(l, iter);

    if (lua_pcall(l, 2, 0, 0)) {
        ecs_err("run luajit system %s: %s", callback, lua_tostring(l, -1));
        lua_pop(l, 1);
    }

    luaX_stack_guard_epilog(l, 0);
}

static void s_callback_free(
        void* callback) {
    ecs_os_free(callback);
}

// TODO: Should luajit systems also query for `EcsLuajitHost` to prevent
//       simultaneous access to it? Can different systems be run at the same
//       time?
ecs_entity_t ecs_luajit_system_init(
        ecs_world_t* world,
        ecs_luajit_system_desc_t const* desc) {
    ecs_assert(world != NULL, ECS_INVALID_PARAMETER, NULL);
    ecs_assert(desc != NULL, ECS_INVALID_PARAMETER, NULL);

    if (!desc->callback && !desc->entity) {
        return 0;
    }

    char const* callback = desc->callback;

    if (!callback) {
        callback = ecs_get_name(world, desc->entity);
    }

    return ecs_system(world, {
        .entity = desc->entity,
        .query = desc->query,
        .run = s_system_run,
        .ctx = desc->ctx,
        .binding_ctx = ecs_os_strdup(callback),
        .ctx_free = desc->ctx_free,
        .binding_ctx_free = s_callback_free,
        .interval = desc->interval,
        .rate = desc->rate,
        .tick_source = desc->tick_source,
        .multi_threaded = desc->multi_threaded,
        .no_readonly = desc->no_readonly,
    });
}

static void s_observer_callback(
        ecs_iter_t* iter) {
    EcsLuajitHost const* host = ecs_singleton_get(iter->world, EcsLuajitHost);

    lua_State* l = ecs_luajit_host_at(host, ecs_get_stage_id(iter->world));
    char const* callback = iter->binding_ctx;

    luaX_stack_guard_prolog(l);

    lua_getfield(l, LUA_REGISTRYINDEX, s_key_observer_callback);
    lua_pushstring(l, callback);
    lua_pushlightuserdata(l, iter);

    if (lua_pcall(l, 2, 0, 0)) {
        ecs_err("run luajit observer %s: %s", callback, lua_tostring(l, -1));
        lua_pop(l, 1);
    }

    luaX_stack_guard_epilog(l, 0);
}

ecs_entity_t ecs_luajit_observer_init(
        ecs_world_t* world,
        ecs_luajit_observer_desc_t const* desc) {
    ecs_assert(world != NULL, ECS_INVALID_PARAMETER, NULL);
    ecs_assert(desc != NULL, ECS_INVALID_PARAMETER, NULL);

    if (!desc->callback && !desc->entity) {
        return 0;
    }

    char const* callback = desc->callback;

    if (!callback) {
        callback = ecs_get_name(world, desc->entity);
    }

    ecs_observer_desc_t tmp_desc = {
        .entity = desc->entity,
        .filter = desc->filter,
        .yield_existing = desc->yield_existing,
        .callback = s_observer_callback,
        .ctx = desc->ctx,
        .binding_ctx = ecs_os_strdup(callback),
        .ctx_free = desc->ctx_free,
        .binding_ctx_free = s_callback_free,
        .observable = desc->observable,
        .last_event_id = desc->last_event_id,
    };

    ecs_size_t const bytes = sizeof(*desc->events) * ECS_OBSERVER_DESC_EVENT_COUNT_MAX;
    ecs_os_memcpy(tmp_desc.events, desc->events, bytes);

    return ecs_observer_init(world, &tmp_desc);
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
