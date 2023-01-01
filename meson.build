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

flecs_luajit_binding_srcs = subproject('flecs-luajit-binding').get_variable('srcs')

# Generate resource headers for lua files
file_to_header = find_program('scripts/file_to_header.py')

gen_srcs = custom_target(
  'resource-headers',
  input: [
    flecs_luajit_binding_srcs,
    'src/boot.lua',
  ],
  output: [
    'flecs.lua.h',
    'flecs.cdef.lua.h',
    'boot.lua.h',
  ],
  command: [
    file_to_header,
    '-i', '@INPUT@',
    '-o', '@OUTPUT@',
    '-d', '@SOURCE_ROOT@/src/res',
  ],
)

srcs = [
  gen_srcs,
  'src/module_base.c',
  'src/host.c',
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
