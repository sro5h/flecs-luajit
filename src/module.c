#include "../include/flecs-luajit/module.h"

#include <lualib.h>
#include <lauxlib.h>

ECS_COMPONENT_DECLARE(EcsLuajitConfig);
ECS_COMPONENT_DECLARE(EcsLuajitHost);

typedef struct ecs_luajit_binding_t {
        char* callback;
} ecs_luajit_binding_t;

ecs_luajit_binding_t* ecs_luajit_binding_init(
                char const* callback);

void ecs_luajit_binding_fini(
                ecs_luajit_binding_t* self);

static void s_lua_state_init(
                lua_State* l,
                int32_t index,
                EcsLuajitConfig const* config);

static void s_luajit_system_run(
                ecs_iter_t* iter);

void ecs_luajit_ensure_stages(
                ecs_world_t* world) {
        int32_t stage_count = ecs_get_stage_count(world);
        EcsLuajitConfig const* config = ecs_singleton_get(world, EcsLuajitConfig);
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

                s_lua_state_init(host->states[i], i, config);
        }

        ecs_singleton_modified(world, EcsLuajitHost);
}

ecs_entity_t ecs_luajit_system_init(
                ecs_world_t* world,
                ecs_luajit_system_desc_t const* desc) {
        char const* callback = desc->callback;
        if (!callback && desc->entity) {
                callback = ecs_get_name(world, desc->entity);
        }

        ecs_luajit_binding_t* binding = ecs_luajit_binding_init(callback);

        return ecs_system_init(world, &(ecs_system_desc_t) {
                .entity = desc->entity,
                .query = desc->query,
                .run = s_luajit_system_run,
                .binding_ctx = binding,
                .binding_ctx_free = (ecs_ctx_free_t) ecs_luajit_binding_fini,
                .multi_threaded = desc->multi_threaded,
                .no_readonly = desc->no_readonly,
        });
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

FLECS_LUAJIT_API
void FlecsConfigLuajitImport(
                ecs_world_t* world) {
        ECS_MODULE(world, FlecsConfigLuajit);

        ecs_set_name_prefix(world, "EcsLuajit");

        ECS_COMPONENT_DEFINE(world, EcsLuajitConfig);
}

void FlecsLuajitImport(
                ecs_world_t* world) {
        ECS_MODULE(world, FlecsLuajit);
        ECS_IMPORT(world, FlecsConfigLuajit);

        ecs_set_name_prefix(world, "EcsLuajit");
        ecs_atfini(world, flecs_luajit_fini, NULL);

        ECS_COMPONENT_DEFINE(world, EcsLuajitHost);

        if (!ecs_has(world, ecs_id(EcsLuajitConfig), EcsLuajitConfig)) {
                ecs_singleton_set(world, EcsLuajitConfig, {
                        .init_file = "init.lua",
                });
        }

        ecs_singleton_set(world, EcsLuajitHost, { 0 });
        ecs_luajit_ensure_stages(world);
}

ecs_luajit_binding_t* ecs_luajit_binding_init(
                char const* callback) {
        ecs_luajit_binding_t* self = ecs_os_calloc(sizeof(*self));
        self->callback = ecs_os_strdup(callback);
        return self;
}

void ecs_luajit_binding_fini(
                ecs_luajit_binding_t* self) {
        if (self) {
                ecs_os_free(self->callback);
                ecs_os_free(self);
        }
}

static void s_lua_state_init(
                lua_State* l,
                int32_t index,
                EcsLuajitConfig const* config) {
        // TODO: Add stack guards
        if (luaL_dofile(l, config->init_file)) {
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

static void s_luajit_system_run(
                ecs_iter_t* iter) {
        EcsLuajitHost const* host = ecs_singleton_get(iter->world, EcsLuajitHost);

        int32_t stage_id = ecs_get_stage_id(iter->world);
        // TODO: Assert stage_id is in valid range
        lua_State* l = host->states[stage_id];
        ecs_luajit_binding_t const* binding = iter->binding_ctx;

        lua_getglobal(l, "ecs_luajit_system_runner");
        lua_getglobal(l, binding->callback);
        // Push `ecs_iter_t*` as lightuserdata and cast it to the correct cdata.
        // See: https://www.freelists.org/post/luajit/Transferring-cdata-between-states,7
        lua_pushlightuserdata(l, iter);

        if (lua_pcall(l, 2, 0, 0) != 0) {
                ecs_warn("ecs_luajit: run luajit system %s: %s\n",
                                binding->callback, lua_tostring(l, -1));
                lua_pop(l, 1);
        }
}
