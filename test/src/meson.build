sources = files(
    'print-utils.cpp',
)

cmake = import('cmake')

ut_dep = dependency('ut', method: 'cmake', modules: 'Boost::ut', fallback: ['boost-ut', 'ut_dep'])

testing_include_dir = include_directories('../include/')

zecalculator_testing_lib = static_library('zecalculator_testing',
    sources,
    include_directories : testing_include_dir,
    dependencies: [zecalculator_dep, ut_dep],
    install : not meson.is_subproject()
)

zecalculator_testing_dep = declare_dependency(
    include_directories : [zecalculator_inc, testing_include_dir],
    dependencies: [zecalculator_dep, ut_dep],
    link_with : zecalculator_testing_lib
)
