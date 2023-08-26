subdir('decl')
subdir('impl')

if not meson.is_subproject()
  install_headers(
    files(
      'evaluation.h',
      'function.h',
      'variable.h',
    ),
    subdir: 'zecalculator',
    preserve_path: true
  )
endif
