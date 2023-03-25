header_files = files(
    'external/expected.h',
    'utils/evaluation_error.h',
    'utils/evaluation.h',
    'utils/parsing_error.h',
    'utils/parser.h',
    'utils/syntax_tree.h',
    'utils/token.h',
    'utils/utils.h',
    'builtin_function.h',
    'mathworld.h',
)

if not meson.is_subproject()
    install_headers(header_files, subdir: 'zecalculator', preserve_path: true)
endif
