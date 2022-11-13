print('init.lua executed')

function on_load()
    print('on_load()')
end

function on_load_stage(i)
    print('on_load_stage(' .. i .. ')')
end
