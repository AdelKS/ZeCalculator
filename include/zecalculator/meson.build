header_files = files(
    'external/expected.h',
    'utils/evaluation_error.h',
    'utils/evaluation.h',
    'utils/name_map.h',
    'utils/parsing_error.h',
    'utils/slotted_vector.h',
    'utils/parser.h',
    'utils/syntax_tree.h',
    'utils/token.h',
    'utils/utils.h',
    'builtin_binary_functions.h',
    'builtin_unary_functions.h',
    'global_constant.h',
    'mathworld.h',
)

if not meson.is_subproject()
    install_headers(header_files, subdir: 'zecalculator', preserve_path: true)
endif
