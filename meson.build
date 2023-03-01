project(
    'pkmsg', 
    'c',
    version: 'v0.0.1',
    default_options: 'c_std=gnu89',
    subproject_dir: 'external'
)

columns = subproject('columns')

static_library(
    'pkmsg',
    sources: [
        'src/decoder.c',
        'src/encoder.c',
        'src/internal.c',
    ],
    include_directories: 'include',
    dependencies: [
        dependency('msgpack'),
        columns.get_variable('columns_dep'),
    ]
)
