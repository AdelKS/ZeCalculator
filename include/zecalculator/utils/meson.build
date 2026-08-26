if not meson.is_subproject()
  install_headers(
    files(
      'name_map.h',
      'non_unique_ptr.h',
      'refs.h',
      'slotted_deque.h',
      'tuple.h',
      'utils.h',
    ),
    subdir: fs.relative_to(meson.current_source_dir(), meson.project_source_root() / 'include'),
    preserve_path: true
  )
endif
