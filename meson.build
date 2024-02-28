project(
    'pkmsg', 
    'c',
    version: 'v0.0.1',
    default_options: 'c_std=gnu99',
)

enable_fuzz = get_option('enable-fuzz')
enable_example = get_option('enable-example')

cmp = subproject('cmp', required: true)
columns = subproject('columns', required: true)

pkmsg_lib = library(
    'pkmsg',
    sources: [
        'src/decoder.c',
        'src/encoder.c',
        'src/common.c',
    ],
    include_directories: 'include',
    gnu_symbol_visibility: 'hidden',
    c_shared_args: ['-DBUILD_PKMSG_DLL'],
    dependencies: [
        cmp.get_variable('cmp_dep'),
        columns.get_variable('columns_dep'),
    ],
)

pkmsg_dep = declare_dependency(
    include_directories: 'include',
    link_with: [
        pkmsg_lib,
    ],
    dependencies: [
        columns.get_variable('columns_dep'),
    ],
)

if enable_example
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
endif

if enable_fuzz
    if meson.get_compiler('c').get_id() == 'clang'
        executable(
            'fuzz',
            sources: [
                'example/fuzz.c',
                'example/messages_def.c',
            ],
            c_args: ['-fsanitize=fuzzer'],
            link_args: ['-fsanitize=fuzzer'],
            dependencies: [
                pkmsg_dep,
                columns.get_variable('columns_dep'),
            ]
        )
    endif
endif
