project('ZeCalculator', 'cpp',
    version : '0.13.0',
    default_options : [
        'cpp_std=c++23',
        'buildtype=release',
        'optimization=3',
        'default_library=both',
        'cpp_args= -march=native -pipe',
        'warning_level=3',
        'werror=true'
    ]
)

fs = import('fs')

compiler = meson.get_compiler('cpp')

cpp23_code = '''
#include <flat_map>
#include <expected>

void func() {
  [[maybe_unused]] std::expected<bool, int> v = std::unexpected(3);
  [[maybe_unused]] std::flat_map<int, int> m = {{2, 3}, {4, 5}};
}
'''

if not compiler.compiles(cpp23_code, name : 'C++23 STL support check')
  error('C++ STL doesn\'t implement std::flat_map or std::expected')
endif

zecalculator_inc = include_directories('include')

subdir('include/zecalculator')

zecalculator_dep = declare_dependency(
    include_directories : zecalculator_inc,
)

meson.override_dependency('zecalculator', zecalculator_dep)

if get_option('test')
    subdir('test')
endif
