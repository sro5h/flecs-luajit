function on_load()
    print('on_load()')
end

function on_load_stage(i)
    print('on_load_stage(' .. i .. ')')
end

function update(iter)
    print('update()')
end

print('init.lua executed')
