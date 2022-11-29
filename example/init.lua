local ecs = require 'external.flecs-luajit-binding.flecs' {
    cdef = require 'external.flecs-luajit-binding.flecs_cdef',
}

function on_load()
    print('on_load()')
end

function on_load_stage(i)
    print('on_load_stage(' .. i .. ')')
end

print('init.lua executed')
