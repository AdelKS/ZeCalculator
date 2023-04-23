subdir('external')
subdir('utils')

header_files = files(
    'builtin_binary_functions.h',
    'builtin_unary_functions.h',
    'expression.h',
    'function.h',
    'global_constant.h',
    'global_variable.h',
    'mathworld.h',
)

if not meson.is_subproject()
    install_headers(
        [external_header_files, utils_header_files, header_files],
        subdir: 'zecalculator', preserve_path: true
    )
endif
