header_files = files(
    'external/expected.h',
    'utils/error.h',
    'utils/parser.h',
    'utils/syntax_tree.h',
    'utils/token.h'
)

if not meson.is_subproject()
    install_headers(header_files, subdir: 'zecalculator')
endif