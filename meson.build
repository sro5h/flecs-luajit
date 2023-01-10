project(
  'flecs-luajit',
  'c',
  version: '0.1.0',
  license: 'mit',
  default_options: [
    'c_std=c99',
    'warning_level=3'
  ],
  subproject_dir: 'external'
)

flecs_luajit_binding_srcs = subproject('flecs-luajit-binding').get_variable('srcs_flecs_lua')

# Generate resource headers for lua files
generate_file_header = find_program('scripts/generate_file_header.py')

gen_srcs = custom_target(
  'resource-headers',
  input: [
    flecs_luajit_binding_srcs,
    'src/cdef.lua',
    'src/module_base.lua',
  ],
  output: [
    'flecs.lua.h',
    'cdef.lua.h',
    'module_base.lua.h',
  ],
  command: [
    generate_file_header,
    '-i', '@INPUT@',
    '-o', '@OUTPUT@',
    '-d', '@SOURCE_ROOT@/src/res',
  ],
)

srcs = [
  gen_srcs,
  'src/flecs_ext.c',
  'src/host.c',
  'src/module_base.c',
]

deps = []

if build_machine.system() == 'linux'
  deps += meson.get_compiler('c').find_library('m')
endif

deps += subproject('flecs').get_variable('flecs_dep')
deps += dependency('luajit', version: '>= 2.0')

args = []

if get_option('default_library') == 'shared'
  args += '-DFLECS_LUAJIT_EXPORTS'
endif

flecs_luajit_lib = library(
  'flecsluajit',
  srcs,
  c_args: args,
  dependencies: deps,
)

flecs_luajit_dep = declare_dependency(
  link_with: flecs_luajit_lib,
  include_directories: include_directories('include'),
)

subdir('example')
