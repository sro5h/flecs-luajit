local flecs = require 'flecs'
local ffi = require 'ffi'

flecs.luajit = require 'flecs.luajit'
flecs:init({ cdef = require 'flecs.cdef', world = flecs.luajit.world })

if flecs.luajit.stage_id == 0 then
    function flecs.luajit.once(fn)
        fn()
    end
else
    function flecs.luajit.once(fn)
    end
end

ffi.cdef [[
    int32_t ecs_ext_iter_term_count(struct ecs_iter_t const*);
    int16_t ecs_ext_vector_offset(ecs_size_t alignment);
]]

local clib = ffi.C

function flecs.luajit.system_run(fn_name, iter)
    local fn = _G[fn_name]
    iter = ffi.cast('ecs_iter_t*', iter)

    if clib.ecs_ext_iter_term_count(iter) == 0 then
        fn(iter)
    else while clib.ecs_iter_next(iter) do
        fn(iter)
    end end
end
