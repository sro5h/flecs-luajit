print('init.lua executed')

function on_load()
    print('on_load()')
end

function on_load_stage(i)
    print('on_load_stage(' .. i .. ')')
end

function ecs_luajit_system_runner(iter)
    update(iter)
end

function update(iter)
    print('update()')
end
