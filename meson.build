project('ZeCalculator', 'cpp',
    version : '0.0.1',
    default_options : [
        'cpp_std=c++20',
        'buildtype=release',
        'optimization=3',
        'default_library=shared',
        'c_args= -march=native -pipe',
        'cpp_args= -march=native -pipe',
        'warning_level=3',
        'werror=true'
    ]
)

fs = import('fs')

zecalculator_inc = include_directories('include')

subdir('src')

zecalculator_dep = declare_dependency(
    include_directories : zecalculator_inc,
    link_with : zecalculator_lib
)

subdir('test')
