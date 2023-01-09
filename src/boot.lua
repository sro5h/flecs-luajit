local flecs = require 'flecs'
local ffi = require 'ffi'

flecs.luajit = require 'flecs.luajit'
flecs:init({ cdef = flecs.luajit.cdef, world = flecs.luajit.world })

if flecs.luajit.stage_id == 0 then
    function flecs.luajit.once(fn)
        fn()
    end
else
    function flecs.luajit.once(fn)
    end
end

local clib = ffi.C

function flecs.luajit.system_run(fn_name, iter)
    local fn = _G[fn_name]
    iter = ffi.cast('ecs_iter_t*', iter)

    if clib.ecs_ext_iter_term_count(iter) == 0 then
        fn(iter)
        iter:fini()
    else while iter:next() do
        fn(iter)
    end end
end

function flecs.luajit.observer_callback(fn_name, iter)
    local fn = _G[fn_name]
    fn(ffi.cast('ecs_iter_t*', iter))
end
