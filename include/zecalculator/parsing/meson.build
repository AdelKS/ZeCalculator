if not meson.is_subproject()
  install_headers(
    files(
      'parser.h',
      'types.h',
    ),
    subdir: fs.relative_to(meson.current_source_dir(), meson.project_source_root() / 'include'),
    preserve_path: true
  )
endif

subdir('data_structures')
subdir('decl')
subdir('impl')
