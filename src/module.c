#include "../include/flecs-luajit/module.h"
#include "../include/flecs-luajit/util_lua.h"

#include <lualib.h>
#include <lauxlib.h>

ECS_COMPONENT_DECLARE(EcsLuajitConfig);
ECS_COMPONENT_DECLARE(EcsLuajitHost);
ECS_COMPONENT_DECLARE(EcsLuajitSystem);
ECS_COMPONENT_DECLARE(EcsLuajitScript);
ECS_COMPONENT_DECLARE(EcsLuajitOnStage);
ECS_DECLARE(EcsLuajitLoaded);

typedef struct ecs_luajit_binding_t {
        char* callback;
} ecs_luajit_binding_t;

ecs_luajit_binding_t* ecs_luajit_binding_init(
                char const* callback);

void ecs_luajit_binding_fini(
                ecs_luajit_binding_t* self);

static void s_luajit_run_init_file(
                lua_State* l,
                int32_t index,
                char const* init_file,
                ecs_world_t* world);

static void s_luajit_run_callbacks(
                lua_State* l,
                int32_t index);

static void s_luajit_system_run(
                ecs_iter_t* iter);

static void s_luajit_loadable_on_set(
                ecs_iter_t* iter);

static void s_luajit_system_on_load(
                ecs_iter_t* iter);

static bool s_luajit_run(
                lua_State* l,
                int32_t stage_id,
                char const* code,
                int32_t args,
                int32_t results);

static void s_luajit_script_on_load(
                ecs_iter_t* iter);

static char const* s_luajit_identifier(
                ecs_world_t const* world,
                ecs_entity_t entity);

static void ecs_move(EcsLuajitConfig)(
                void*,
                void*,
                int32_t,
                ecs_type_info_t const*);

static void ecs_copy(EcsLuajitConfig)(
                void*,
                void const*,
                int32_t,
                ecs_type_info_t const*);

static void ecs_dtor(EcsLuajitConfig)(
                void*,
                int32_t,
                ecs_type_info_t const*);

static void ecs_move(EcsLuajitSystem)(
                void*,
                void*,
                int32_t,
                ecs_type_info_t const*);

static void ecs_copy(EcsLuajitSystem)(
                void*,
                void const*,
                int32_t,
                ecs_type_info_t const*);

static void ecs_dtor(EcsLuajitSystem)(
                void*,
                int32_t,
                ecs_type_info_t const*);

static void ecs_move(EcsLuajitScript)(
                void*,
                void*,
                int32_t,
                ecs_type_info_t const*);

static void ecs_copy(EcsLuajitScript)(
                void*,
                void const*,
                int32_t,
                ecs_type_info_t const*);

static void ecs_dtor(EcsLuajitScript)(
                void*,
                int32_t,
                ecs_type_info_t const*);

static char const* s_lua_state_init_code =
        "local ffi = require 'ffi'\n"
        "ffi.cdef [[\n"
        "       typedef uint64_t ecs_entity_t;\n"
        "       typedef struct ecs_world_t ecs_world_t;\n"
        "       typedef struct ecs_iter_t ecs_iter_t;\n"
        "       bool ecs_iter_next(ecs_iter_t*);\n"
        "       void ecs_luajit_run(ecs_world_t*, char const*);\n"
        "       void ecs_luajit_run_on_stage(ecs_world_t*, int32_t, char const*);\n"
        "       int32_t ecs_luajit_iter_term_count(ecs_iter_t const*);\n"
        "       char* ecs_luajit_struct_cdef(ecs_world_t const*, ecs_entity_t);\n"
        "]]\n"
        "function ecs_luajit_system_runner(callback, iter)\n"
        "       iter = ffi.cast('ecs_iter_t*', iter)\n"
        "       if ffi.C.ecs_luajit_iter_term_count(iter) == 0 then\n"
        "               callback(iter)\n"
        "       else while ffi.C.ecs_iter_next(iter) do\n"
        "               callback(iter)\n"
        "       end end\n"
        "end\n"
;

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
                lua_State* l = luaL_newstate();
                luaL_openlibs(l);

                host->states[i] = l;

                s_luajit_run(l, i, s_lua_state_init_code, 0, 0);

                if (config->init_code) {
                        lua_pushlightuserdata(l, world);
                        s_luajit_run(l, i, config->init_code, 1, 0);
                }

                if (config->init_file) {
                        s_luajit_run_init_file(l, i, config->init_file, world);
                }
        }

        // Use separate loop to execute callbacks such that the states are all
        // initialised. Then functions operating on thos can be called.
        for (int32_t i = 0; i < host->count; ++ i) {
                s_luajit_run_callbacks(host->states[i], i);
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

void ecs_luajit_run(
                ecs_world_t* world,
                char const* code) {
        EcsLuajitHost const* host = ecs_singleton_get(world, EcsLuajitHost);
        // TODO: Assert `host->count` equals `ecs_stage_count`

        for (int32_t i = 0; i < host->count; ++ i) {
                s_luajit_run(host->states[i], i, code, 0, 0);
        }
}

void ecs_luajit_run_on_stage(
                ecs_world_t* world,
                int32_t stage_id,
                char const* code) {
        // TODO: Check parameters
        EcsLuajitHost const* host = ecs_singleton_get(world, EcsLuajitHost);
        s_luajit_run(host->states[stage_id], stage_id, code, 0, 0);
}

int32_t ecs_luajit_iter_term_count(
                ecs_iter_t const* iter) {
        ecs_filter_t const* filter = ecs_query_get_filter(iter->priv.iter.query.query);
        return filter->term_count;
}

char* ecs_luajit_struct_cdef(
                ecs_world_t const* world,
                ecs_entity_t type) {
        EcsMetaTypeSerialized const* serialized = ecs_get(world, type, EcsMetaTypeSerialized);

        if (!serialized) {
                // TODO: Maybe return empty string instead
                return NULL;
        }

        ecs_strbuf_t buf = ECS_STRBUF_INIT;

        char const* identifier = s_luajit_identifier(world, type);
        ecs_strbuf_append(&buf, "typedef struct %s", identifier);

        for (int32_t i = 0; i < ecs_vector_count(serialized->ops); ++ i) {
                ecs_meta_type_op_t const* op = ecs_vector_get(
                        serialized->ops, ecs_meta_type_op_t, i
                );

                // TODO: EcsOp{Array,Vector,Scope,Enum,Bitmask,Primitive}
                switch (op->kind) {
                case EcsOpPush:
                        ecs_strbuf_appendstr(&buf, "{");
                        break;
                case EcsOpPop:
                        ecs_strbuf_appendstr(&buf, "}");
                        break;
                case EcsOpBool:
                        ecs_strbuf_append(&buf, "ecs_bool_t %s;", op->name);
                        break;
                case EcsOpChar:
                        ecs_strbuf_append(&buf, "ecs_char_t %s;", op->name);
                        break;
                case EcsOpByte:
                        ecs_strbuf_append(&buf, "ecs_byte_t %s;", op->name);
                        break;
                case EcsOpU8:
                        ecs_strbuf_append(&buf, "ecs_u8_t %s;", op->name);
                        break;
                case EcsOpU16:
                        ecs_strbuf_append(&buf, "ecs_u16_t %s;", op->name);
                        break;
                case EcsOpU32:
                        ecs_strbuf_append(&buf, "ecs_u32_t %s;", op->name);
                        break;
                case EcsOpU64:
                        ecs_strbuf_append(&buf, "ecs_u64_t %s;", op->name);
                        break;
                case EcsOpI8:
                        ecs_strbuf_append(&buf, "ecs_i8_t %s;", op->name);
                        break;
                case EcsOpI16:
                        ecs_strbuf_append(&buf, "ecs_i16_t %s;", op->name);
                        break;
                case EcsOpI32:
                        ecs_strbuf_append(&buf, "ecs_i32_t %s;", op->name);
                        break;
                case EcsOpI64:
                        ecs_strbuf_append(&buf, "ecs_i64_t %s;", op->name);
                        break;
                case EcsOpF32:
                        ecs_strbuf_append(&buf, "ecs_f32_t %s;", op->name);
                        break;
                case EcsOpF64:
                        ecs_strbuf_append(&buf, "ecs_f64_t %s;", op->name);
                        break;
                case EcsOpUPtr:
                        ecs_strbuf_append(&buf, "ecs_uptr_t %s;", op->name);
                        break;
                case EcsOpIPtr:
                        ecs_strbuf_append(&buf, "ecs_iptr_t %s;", op->name);
                        break;
                case EcsOpString:
                        ecs_strbuf_append(&buf, "ecs_string_t %s;", op->name);
                        break;
                case EcsOpEntity:
                        ecs_strbuf_append(&buf, "ecs_entity_t %s;", op->name);
                        break;
                default:
                        // TODO: Remove
                        break;
                }
        }

        ecs_strbuf_append(&buf, "%s;", identifier);
        return ecs_strbuf_get(&buf);
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

        ecs_set_hooks(world, EcsLuajitConfig, {
                .ctor = ecs_default_ctor,
                .move = ecs_move(EcsLuajitConfig),
                .copy = ecs_copy(EcsLuajitConfig),
                .dtor = ecs_dtor(EcsLuajitConfig),
        });
}

void FlecsLuajitImport(
                ecs_world_t* world) {
        ECS_MODULE(world, FlecsLuajit);
        ECS_IMPORT(world, FlecsConfigLuajit);

        ecs_set_name_prefix(world, "EcsLuajit");
        ecs_atfini(world, flecs_luajit_fini, NULL);

        ECS_COMPONENT_DEFINE(world, EcsLuajitHost);
        ECS_COMPONENT_DEFINE(world, EcsLuajitSystem);
        ECS_COMPONENT_DEFINE(world, EcsLuajitScript);
        ECS_COMPONENT_DEFINE(world, EcsLuajitOnStage);
        ECS_ENTITY_DEFINE(world, EcsLuajitLoaded, EcsTag);

        ecs_set_hooks(world, EcsLuajitSystem, {
                .ctor = ecs_default_ctor,
                .move = ecs_move(EcsLuajitSystem),
                .copy = ecs_copy(EcsLuajitSystem),
                .dtor = ecs_dtor(EcsLuajitSystem),
        });

        ecs_set_hooks(world, EcsLuajitScript, {
                .ctor = ecs_default_ctor,
                .move = ecs_move(EcsLuajitScript),
                .copy = ecs_copy(EcsLuajitScript),
                .dtor = ecs_dtor(EcsLuajitScript),
        });

        // TODO: Should the second term be [out] as it is removed?
        ECS_OBSERVER(world, s_luajit_loadable_on_set, EcsOnSet,
                [in] $Loadable,
                [filter] (flecs.luajit.Loaded, $Loadable)
        );

        ecs_system(world, {
                .entity = ecs_entity(world, {
                        .name = "EcsLuajitSystemOnLoad",
                        .add = { ecs_dependson(EcsOnLoad) },
                }),
                .query.filter.expr =
                        "[in] flecs.luajit.System,"
                        "!(flecs.luajit.Loaded, flecs.luajit.System)"
                ,
                .callback = s_luajit_system_on_load,
                .no_readonly = true,
        });

        ecs_system(world, {
                .entity = ecs_entity(world, {
                        .name = "EcsLuajitScriptOnLoad",
                        .add = { ecs_dependson(EcsOnLoad) },
                }),
                .query.filter.expr =
                        "flecs.luajit.Host($),"
                        "[in] flecs.luajit.Script,"
                        "!(flecs.luajit.Loaded, flecs.luajit.Script),"
                        "?(flecs.luajit.OnStage, flecs.luajit.Script)"
                ,
                .callback = s_luajit_script_on_load,
                .no_readonly = true,
        });

        if (!ecs_has(world, ecs_id(EcsLuajitConfig), EcsLuajitConfig)) {
                ecs_singleton_set(world, EcsLuajitConfig, {
                        .init_file = "init.lua",
                });
        }

        ecs_singleton_set(world, EcsLuajitHost, { 0 });
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

static void s_luajit_run_init_file(
                lua_State* l,
                int32_t index,
                char const* init_file,
                ecs_world_t* world) {
        luaX_stack_guard_prolog(l);

        if (luaL_loadfile(l, init_file)) {
                ecs_warn("ecs_luajit: load init file on stage %d: %s\n",
                        index, lua_tostring(l, -1));
                lua_pop(l, 1);

                luaX_stack_guard_epilog(l, 0);
                return;
        }

        lua_pushlightuserdata(l, world);

        if (lua_pcall(l, 1, 0, 0)) {
                ecs_warn("ecs_luajit: run init file on stage %d: %s\n",
                        index, lua_tostring(l, -1));
                lua_pop(l, 1);
        }

        luaX_stack_guard_epilog(l, 0);
}

static void s_luajit_run_callbacks(
                lua_State* l,
                int32_t index) {
        luaX_stack_guard_prolog(l);

        if (index == 0) {
                lua_getglobal(l, "on_load");

                if (lua_isfunction(l, -1)) { if (lua_pcall(l, 0, 0, 0)) {
                        ecs_warn("ecs_luajit: call on_load on stage %d: %s\n",
                                index, lua_tostring(l, -1));
                        lua_pop(l, 1);
                } } else {
                        lua_pop(l, 1);
                }
        }

        lua_getglobal(l, "on_load_stage");
        lua_pushinteger(l, index);

        if (lua_isfunction(l, -2)) { if (lua_pcall(l, 1, 0, 0)) {
                ecs_warn("ecs_luajit: call on_load_stage on stage %d: %s\n",
                        index, lua_tostring(l, -1));
                lua_pop(l, 1);
        } } else {
                lua_pop(l, 2);
        }

        luaX_stack_guard_epilog(l, 0);
}

static void s_luajit_system_run(
                ecs_iter_t* iter) {
        EcsLuajitHost const* host = ecs_singleton_get(iter->world, EcsLuajitHost);

        int32_t stage_id = ecs_get_stage_id(iter->world);
        // TODO: Assert stage_id is in valid range
        lua_State* l = host->states[stage_id];
        ecs_luajit_binding_t const* binding = iter->binding_ctx;

        luaX_stack_guard_prolog(l);

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

        luaX_stack_guard_epilog(l, 0);
}

static void s_luajit_loadable_on_set(
                ecs_iter_t* iter) {
        ecs_id_t loadable_id = ecs_field_id(iter, 1);

        for (int32_t i = 0; i < iter->count; ++ i) {
                ecs_remove_pair(iter->world, iter->entities[i], EcsLuajitLoaded, loadable_id);
        }
}

static void s_luajit_system_on_load(
                ecs_iter_t* iter) {
        EcsLuajitSystem const* system = ecs_field(iter, EcsLuajitSystem, 1);

        for (int32_t i = 0; i < iter->count; ++ i) {
                ecs_luajit_system_init(iter->world, &(ecs_luajit_system_desc_t) {
                        .entity = iter->entities[i],
                        .query.filter.expr = system[i].query_expr,
                        .callback = system[i].callback,
                        .interval = system[i].interval,
                        .rate = system[i].rate,
                        .tick_source = system[i].tick_source,
                        .multi_threaded = system[i].multi_threaded,
                        .no_readonly = system[i].no_readonly,
                });

                ecs_add_pair(iter->world, iter->entities[i], EcsLuajitLoaded,
                                ecs_id(EcsLuajitSystem));
        }
}

static bool s_luajit_run(
                lua_State* l,
                int32_t stage_id,
                char const* code,
                int32_t args,
                int32_t results) {
        luaX_stack_guard_prolog(l);

        if (luaL_loadstring(l, code)) {
                ecs_warn("ecs_luajit: load code on stage %d: %s\n",
                        stage_id, lua_tostring(l, -1));
                lua_pop(l, 1 + args);

                luaX_stack_guard_epilog(l, -args);
                return false;
        }

        lua_insert(l, -(args + 1)); // Move function below arguments

        if (lua_pcall(l, args, results, 0)) {
                ecs_warn("ecs_luajit: run code on stage %d: %s\n",
                        stage_id, lua_tostring(l, -1));
                lua_pop(l, 1);

                luaX_stack_guard_epilog(l, -args);
                return false;
        }

        luaX_stack_guard_epilog(l, results - args);
        return true;
}

static void s_luajit_run_and_set(
                lua_State* l,
                int32_t stage_id,
                char const* code,
                char const* name) {
        luaX_stack_guard_prolog(l);

        if (s_luajit_run(l, stage_id, code, 0, 1)) {
                if (!lua_isnil(l, -1) && name) {
                        lua_setglobal(l, name);
                } else {
                        lua_pop(l, 1);
                }
        }

        luaX_stack_guard_epilog(l, 0);
}

static void s_luajit_script_on_load(
                ecs_iter_t* iter) {
        EcsLuajitHost const* host = ecs_field(iter, EcsLuajitHost, 1);
        EcsLuajitScript const* script = ecs_field(iter, EcsLuajitScript, 2);
        EcsLuajitOnStage const* on_stage = NULL;

        if (ecs_field_is_set(iter, 4)) {
                 on_stage = ecs_field(iter, EcsLuajitOnStage, 4);
        }

        for (int32_t i = 0; i < iter->count; ++ i) {
                char const* name = ecs_get_name(iter->world, iter->entities[i]);

                if (on_stage) {
                        lua_State* l = host->states[on_stage[i].stage_id];
                        s_luajit_run_and_set(l, on_stage[i].stage_id, script[i].code, name);
                } else { for (int32_t stage_id = 0; stage_id < host->count; ++ stage_id) {
                        lua_State* l = host->states[stage_id];
                        s_luajit_run_and_set(l, stage_id, script[i].code, name);
                } }

                ecs_add_pair(iter->world, iter->entities[i], EcsLuajitLoaded, ecs_id(EcsLuajitScript));
        }
}

static char const* s_luajit_identifier(
                ecs_world_t const* world,
                ecs_entity_t entity) {
        char const* symbol = ecs_get_symbol(world, entity);
        return symbol ? symbol : ecs_get_name(world, entity);
}

static ECS_COPY(EcsLuajitConfig, dst, src, {
        ecs_os_strset(&dst->init_code, src->init_code);
        ecs_os_strset(&dst->init_file, src->init_file);
})

static ECS_MOVE(EcsLuajitConfig, dst, src, {
        ecs_os_free(dst->init_code);
        dst->init_code = src->init_code;
        src->init_code = NULL;
        ecs_os_free(dst->init_file);
        dst->init_file = src->init_file;
        src->init_file = NULL;
})

static ECS_DTOR(EcsLuajitConfig, ptr, {
        ecs_os_free(ptr->init_code);
        ecs_os_free(ptr->init_file);
})

static ECS_COPY(EcsLuajitSystem, dst, src, {
        ecs_os_strset(&dst->query_expr, src->query_expr);
        ecs_os_strset(&dst->callback, src->callback);
        dst->interval = src->interval;
        dst->rate = src->rate;
        dst->tick_source = src->tick_source;
        dst->multi_threaded = src->multi_threaded;
        dst->no_readonly = src->no_readonly;
})

static ECS_MOVE(EcsLuajitSystem, dst, src, {
        ecs_os_free(dst->query_expr);
        dst->query_expr = src->query_expr;
        src->query_expr = NULL;
        ecs_os_free(dst->callback);
        dst->callback = src->callback;
        src->callback = NULL;
        dst->interval = src->interval;
        src->interval = 0;
        dst->rate = src->rate;
        src->rate = 0;
        dst->tick_source = src->tick_source;
        src->tick_source = 0;
        dst->multi_threaded = src->multi_threaded;
        src->multi_threaded = 0;
        dst->no_readonly = src->no_readonly;
        src->no_readonly = 0;
})

static ECS_DTOR(EcsLuajitSystem, ptr, {
        ecs_os_free(ptr->query_expr);
        ecs_os_free(ptr->callback);
})

static ECS_COPY(EcsLuajitScript, dst, src, {
        ecs_os_strset(&dst->code, src->code);
})

static ECS_MOVE(EcsLuajitScript, dst, src, {
        ecs_os_free(dst->code);
        dst->code = src->code;
        src->code = NULL;
})

static ECS_DTOR(EcsLuajitScript, ptr, {
        ecs_os_free(ptr->code);
})
