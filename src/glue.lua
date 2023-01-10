local flecs = require 'flecs'
local ffi = require 'ffi'

local clib = ffi.C

function flecs.World:observer(descOrNil)
    local desc = descOrNil or {}
    flecs.aux.fix_array(desc.events, 0)

    return clib.ecs_luajit_observer_init(self, ffi.new('ecs_luajit_observer_desc_t', desc))
end

function flecs.World:system(descOrNil)
    local desc = descOrNil or {}
    flecs.aux.fix_array(desc.query.filter.terms, {})

    return clib.ecs_luajit_system_init(self, ffi.new('ecs_luajit_system_desc_t', desc))
end

function flecs.World:cdef(entity)
    local identifier = self:identifier(entity)

    if identifier == nil or pcall(ffi.typeof, identifier) then
        return
    end

    local serialized = self:get(entity, flecs.g.MetaTypeSerialized)

    if serialized == nil then
        return
    end

    local cdef = 'typedef struct ' .. identifier

    for i = 0, clib.ecs_vector_count(serialized.ops) - 1 do
        local op = ffi.cast('ecs_meta_type_op_t*', clib._ecs_vector_get(
            serialized.ops,
            ffi.sizeof('ecs_meta_type_op_t'),
            clib.ecs_ext_vector_offset(ffi.alignof('ecs_meta_type_op_t')),
            i
        ))

        local op_name = flecs.aux.string(op.name)

        if op.kind == flecs.g.OpPush then
            cdef = cdef .. '{'
        elseif op.kind == flecs.g.OpPop then
            cdef = cdef .. '}'
        elseif op.kind == flecs.g.OpBool then
            cdef = cdef .. 'ecs_bool_t ' .. op_name .. ';'
        elseif op.kind == flecs.g.OpChar then
            cdef = cdef .. 'ecs_char_t ' .. op_name .. ';'
        elseif op.kind == flecs.g.OpByte then
            cdef = cdef .. 'ecs_byte_t ' .. op_name .. ';'
        elseif op.kind == flecs.g.OpU8 then
            cdef = cdef .. 'ecs_u8_t ' .. op_name .. ';'
        elseif op.kind == flecs.g.OpU16 then
            cdef = cdef .. 'ecs_u16_t ' .. op_name .. ';'
        elseif op.kind == flecs.g.OpU32 then
            cdef = cdef .. 'ecs_u32_t ' .. op_name .. ';'
        elseif op.kind == flecs.g.OpU64 then
            cdef = cdef .. 'ecs_u64_t ' .. op_name .. ';'
        elseif op.kind == flecs.g.OpI8 then
            cdef = cdef .. 'ecs_i8_t ' .. op_name .. ';'
        elseif op.kind == flecs.g.OpI16 then
            cdef = cdef .. 'ecs_i16_t ' .. op_name .. ';'
        elseif op.kind == flecs.g.OpI32 then
            cdef = cdef .. 'ecs_i32_t ' .. op_name .. ';'
        elseif op.kind == flecs.g.OpI64 then
            cdef = cdef .. 'ecs_i64_t ' .. op_name .. ';'
        elseif op.kind == flecs.g.OpF32 then
            cdef = cdef .. 'ecs_f32_t ' .. op_name .. ';'
        elseif op.kind == flecs.g.OpF64 then
            cdef = cdef .. 'ecs_f64_t ' .. op_name .. ';'
        elseif op.kind == flecs.g.OpUPtr then
            cdef = cdef .. 'ecs_uptr_t ' .. op_name .. ';'
        elseif op.kind == flecs.g.OpIPtr then
            cdef = cdef .. 'ecs_iptr_t ' .. op_name .. ';'
        elseif op.kind == flecs.g.OpString then
            cdef = cdef .. 'ecs_string_t ' .. op_name .. ';'
        elseif op.kind == flecs.g.OpEntity then
            cdef = cdef .. 'ecs_entity_t ' .. op_name .. ';'
        end
    end

    ffi.cdef(cdef .. identifier .. ';')
end

flecs:bind_metatypes()
