#include "../include/flecs-luajit/module.h"

#include <lualib.h>
#include <lauxlib.h>

ECS_COMPONENT_DECLARE(EcsLuajitHost);

void s_lua_state_init(
                lua_State* l,
                int32_t index,
                char const* init_file) {
        if (luaL_dofile(l, init_file)) {
                ecs_warn("ecs_luajit: load init file on stage %d: %s\n",
                        index, lua_tostring(l, -1));
                lua_pop(l, 1);
        }

        if (index == 0) {
                lua_getglobal(l, "on_load");

                if (lua_isfunction(l, -1)) { if (lua_pcall(l, 0, 0, 0)) {
                        ecs_warn("ecs_luajit: call on_load: %s\n",
                                lua_tostring(l, -1));
                        lua_pop(l, 1);
                } }

                lua_pop(l, 1);
        }

        lua_getglobal(l, "on_load_stage");
        lua_pushinteger(l, index);

        if (lua_isfunction(l, -2)) { if (lua_pcall(l, 1, 0, 0)) {
                ecs_warn("ecs_luajit: call on_load_stage on stage %d: %s\n",
                        index, lua_tostring(l, -1));
                lua_pop(l, 1);
        } }

        lua_pop(l, 2);
}


void ecs_luajit_ensure_stages(
                ecs_world_t* world) {
        int32_t stage_count = ecs_get_stage_count(world);
        EcsLuajitHost* host = ecs_singleton_get_mut(world, EcsLuajitHost);

        if (host->count == stage_count) {
                return;
        }

        for (int32_t i = 0; i < host->count; ++ i) {
                lua_close(host->states[i]);
        }

        ecs_os_free(host->states);

        host->states = ecs_os_calloc(sizeof(*host->states) * stage_count);
        host->count = stage_count;

        for (int32_t i = 0; i < host->count; ++ i) {
                host->states[i] = luaL_newstate();
                luaL_openlibs(host->states[i]);

                s_lua_state_init(host->states[i], i, "init.lua");
        }

        ecs_singleton_modified(world, EcsLuajitHost);
}

void flecs_luajit_fini(
                ecs_world_t* world,
                void* context) {
        (void) context;

        EcsLuajitHost const* host = ecs_singleton_get(world, EcsLuajitHost);

        for (int32_t i = 0; i < host->count; ++ i) {
                lua_close(host->states[i]);
        }

        ecs_os_free(host->states);
}

void FlecsLuajitImport(
                ecs_world_t* world) {
        ECS_MODULE(world, FlecsLuajit);

        ecs_set_name_prefix(world, "EcsLuajit");
        ecs_atfini(world, flecs_luajit_fini, NULL);

        ECS_COMPONENT_DEFINE(world, EcsLuajitHost);

        ecs_singleton_set(world, EcsLuajitHost, { 0 });
        ecs_luajit_ensure_stages(world);
}
