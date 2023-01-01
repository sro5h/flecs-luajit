#include "../include/flecs-luajit/host.h"
#include "../include/flecs-luajit/util_lua.h"

#include <lualib.h>
#include <lauxlib.h>

void ecs_luajit_host_init(
        ecs_luajit_host_t* self,
        int32_t count) {
    *self = (ecs_luajit_host_t) { 0 };
    self->states = ecs_os_calloc(sizeof(*self->states) * count);
    self->count = count;

    for (int32_t i = 0; i < ecs_luajit_host_count(self); ++ i) {
        lua_State* l = luaL_newstate();
        luaL_openlibs(l);
        self->states[i] = l;
    }
}

void ecs_luajit_host_fini(
        ecs_luajit_host_t* self) {
    if (!self) {
        return;
    }

    for (int32_t i = 0; i < ecs_luajit_host_count(self); ++ i) {
        lua_close(self->states[i]);
    }

    ecs_os_free(self->states);
    *self = (ecs_luajit_host_t) { 0 };
}

static bool s_host_pcall(
        ecs_luajit_host_t const* self,
        int32_t stage_id,
        int args,
        int results) {
    lua_State* l = ecs_luajit_host_at(self, stage_id);
    luaX_stack_guard_prolog(l);

    if (lua_pcall(l, args, results, 0)) {
        ecs_err("pcall on stage %d: %s", stage_id, lua_tostring(l, -1));
        lua_pop(l, 1);

        luaX_stack_guard_epilog(l, -(args + 1));
        return false;
    }

    luaX_stack_guard_epilog(l, results - (args + 1));
    return true;
}

bool ecs_luajit_host_call(
        ecs_luajit_host_t const* self,
        ecs_luajit_call_desc_t const* desc) {
    ecs_assert(self != NULL, ECS_INVALID_PARAMETER, NULL);
    ecs_assert(desc != NULL, ECS_INVALID_PARAMETER, NULL);

    lua_State* l = ecs_luajit_host_at(self, desc->stage_id);
    luaX_stack_guard_prolog(l);

    int load_result = 0;

    if (desc->script.is_file) {
        load_result = luaL_loadfile(l, desc->script.source);
    } else {
        load_result = luaL_loadstring(l, desc->script.source);
    }

    if (load_result) {
        ecs_err("load script on stage %d: %s", desc->stage_id, lua_tostring(l, -1));
        lua_pop(l, 1 + desc->args);

        luaX_stack_guard_epilog(l, -desc->args);
        return false;
    }

    lua_insert(l, -(desc->args + 1)); // Move function below arguments

    luaX_stack_guard_epilog(l, 1);
    return s_host_pcall(self, desc->stage_id, desc->args, desc->results);
}

int32_t ecs_luajit_host_count(
        ecs_luajit_host_t const* self) {
    ecs_assert(self != NULL, ECS_INVALID_PARAMETER, NULL);
    return self->count;
}

lua_State* ecs_luajit_host_at(
        ecs_luajit_host_t const* self,
        int32_t stage_id) {
    ecs_assert(self != NULL, ECS_INVALID_PARAMETER, NULL);
    ecs_assert(stage_id >= 0, ECS_INVALID_PARAMETER, NULL);
    ecs_assert(stage_id < self->count, ECS_INVALID_PARAMETER, NULL);
    return self->states[stage_id];
}
