sources = files(
    'utils/parser.cpp',
    'utils/syntax_tree.cpp',
    'utils/token.cpp',
    'builtin_binary_functions.cpp',
    'function.cpp',
    'sequence.cpp',
)

zecalculator_lib = library('zecalculator',
    sources,
    include_directories : zecalculator_inc,
    version : meson.project_version(),
    soversion : meson.project_version(),
    install : not meson.is_subproject()
)
