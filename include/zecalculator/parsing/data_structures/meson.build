if not meson.is_subproject()
  install_headers(
    files(
      'fast.h',
      'rpn.h',
      'shared.h',
      'token.h',
      'uast.h',
      'utils.h',
    ),
    subdir: 'zecalculator',
    preserve_path: true
  )
endif
subdir('decl')
subdir('impl')
