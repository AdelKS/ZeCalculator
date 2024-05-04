if not meson.is_subproject()
  install_headers(
    files(
      'ast.h',
      'deps.h',
      'fast.h',
      'rpn.h',
      'shared.h',
      'token.h',
      'utils.h',
    ),
    subdir: 'zecalculator',
    preserve_path: true
  )
endif
subdir('decl')
subdir('impl')
