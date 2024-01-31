project(
    'pkmsg', 
    'c',
    version: 'v0.0.1',
    default_options: 'c_std=gnu99',
)

enable_fuzz = get_option('enable-fuzz')
enable_example = get_option('enable-example')
enable_shared_library = get_option('enable-shared-library')

cmp = subproject('cmp', required: true)
columns = subproject('columns', required: true)

sources = [
    'src/decoder.c',
    'src/encoder.c',
    'src/internal.c',
]

pkmsg_a = static_library(
    'pkmsg',
    sources: sources,
    include_directories: 'include',
    gnu_symbol_visibility: 'hidden',
    dependencies: [
        cmp.get_variable('cmp_dep'),
        columns.get_variable('columns_dep')
    ],
)

if enable_shared_library
    shared_library(
        'pkmsg',
        sources: sources,
        include_directories: 'include',
        c_args: ['-DBUILD_PKMSG_DLL'],
        gnu_symbol_visibility: 'hidden',
        dependencies: [
            cmp.get_variable('cmp_dep'),
            columns.get_variable('columns_dep')
        ],
    )
endif

pkmsg_dep = declare_dependency(
    include_directories: 'include',
    link_with: pkmsg_a
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
            columns.get_variable('columns_dep')
        ]
    )
endif

if enable_fuzz
    if meson.get_compiler('c').get_id() == 'clang'
        executable(
            'fuzz',
            sources: [
                'fuzz/fuzz.c',
                'fuzz/messages_def.c',
            ],
            c_args: ['-fsanitize=fuzzer'],
            link_args: ['-fsanitize=fuzzer'],
            dependencies: [
                pkmsg_dep,
                columns.get_variable('columns_dep')
            ]
        )
    endif
endif
