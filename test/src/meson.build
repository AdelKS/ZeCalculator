sources = files(
    'print-utils.cpp',
)

cmake = import('cmake')

boost_ut_proj = cmake.subproject('boost-ut')
boost_ut_inc = boost_ut_proj.include_directories('ut')

testing_include_dir = include_directories('../include/')

zecalculator_testing_lib = library('zecalculator_testing',
    sources,
    include_directories : [testing_include_dir, boost_ut_inc],
    dependencies: zecalculator_dep,
    version : meson.project_version(),
    soversion : meson.project_version(),
    install : not meson.is_subproject()
)

zecalculator_testing_dep = declare_dependency(
    include_directories : [zecalculator_inc, boost_ut_inc, testing_include_dir],
    dependencies: zecalculator_dep,
    link_with : zecalculator_testing_lib
)
