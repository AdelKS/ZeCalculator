sources = files(
    'print-utils.cpp',
)

testing_include_dir = include_directories('../include/')

zecalculator_testing_lib = library('zecalculator_testing',
    sources,
    include_directories : testing_include_dir,
    dependencies: zecalculator_dep,
    version : meson.project_version(),
    soversion : meson.project_version(),
    install : not meson.is_subproject()
)

zecalculator_testing_dep = declare_dependency(
    include_directories : testing_include_dir,
    link_with : zecalculator_testing_lib
)