local ffi = require 'ffi'

ffi.cdef [[
    typedef struct ecs_iter_t ecs_iter_t;
    bool ecs_iter_next(ecs_iter_t*);
    int32_t ecs_luajit_iter_term_count(ecs_iter_t const*);
]]

function on_load()
    print('on_load()')
end

function on_load_stage(i)
    print('on_load_stage(' .. i .. ')')
end

function ecs_luajit_system_runner(callback, iter)
    iter = ffi.cast('ecs_iter_t*', iter)

    if ffi.C.ecs_luajit_iter_term_count(iter) == 0 then
        callback(iter)
    else while ffi.C.ecs_iter_next(iter) do
        callback(iter)
    end end
end

function update(iter)
    print('update()')
end

print('init.lua executed')
