project('ZeCalculator', 'cpp',
    version : '0.12.1',
    default_options : [
        'cpp_std=c++20',
        'buildtype=release',
        'optimization=3',
        'default_library=both',
        'cpp_args= -march=native -pipe',
        'warning_level=3',
        'werror=true'
    ]
)

fs = import('fs')

zecalculator_inc = include_directories('include')

subdir('include/zecalculator')

zecalculator_dep = declare_dependency(
    include_directories : zecalculator_inc,
)

meson.override_dependency('zecalculator', zecalculator_dep)

if get_option('test')
    subdir('test')
endif
