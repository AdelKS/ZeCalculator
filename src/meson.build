sources = files(
    'builtin_binary_functions.cpp',
    'function.cpp',
    'parsing/parser.cpp',
    'sequence.cpp',
    'tree.cpp',
    'utils/token.cpp',
)

zecalculator_lib = library('zecalculator',
    sources,
    include_directories : zecalculator_inc,
    version : meson.project_version(),
    soversion : meson.project_version(),
    install : not meson.is_subproject()
)
