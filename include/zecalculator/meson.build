subdir('external')
subdir('utils')
subdir('parsing')
subdir('evaluation')
subdir('math_objects')
subdir('mathworld')

if not meson.is_subproject()
  install_headers(
    files(
      'zecalculator.h',
      'error.h'
    ),
    subdir: 'zecalculator',
    preserve_path: true
  )
endif
