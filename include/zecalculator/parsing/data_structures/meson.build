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
    subdir: fs.relative_to(meson.current_source_dir(), meson.project_source_root() / 'include'),
    preserve_path: true
  )
endif
subdir('decl')
subdir('impl')
