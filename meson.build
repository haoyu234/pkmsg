project(
    'pkmsg', 
    'c',
    version: 'v0.0.1',
    default_options: 'c_std=gnu99',
    subproject_dir: 'external'
)

columns = subproject('columns')

pkmsg_dep = declare_dependency(
    include_directories: 'include',
    link_with: static_library(
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
)

executable(
    'example',
    sources: [
        'example/main.c',
        'example/messages_def.c',
    ],
    dependencies: [
        pkmsg_dep,
        columns.get_variable('columns_dep'),
    ]
)
