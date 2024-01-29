project(
    'pkmsg', 
    'c',
    version: 'v0.0.1',
    default_options: 'c_std=gnu99',
)

cmp = subproject('cmp', required: true)
columns = subproject('columns', required: true)

pkmsg_dep = declare_dependency(
    include_directories: 'include',
    link_whole: static_library(
        'pkmsg',
        sources: [
            'src/decoder.c',
            'src/encoder.c',
            'src/internal.c',
        ],
        include_directories: 'include',
        dependencies: [
            cmp.get_variable('cmp_dep'),
            columns.get_variable('columns_dep')
        ],
    )
)

if get_option('BUILD_EXAMPLE')
    executable(
        'example',
        sources: [
            'example/main.c',
            'example/messages_def.c',
        ],
        c_args: ['-pg'],
        link_args: ['-pg'],
        dependencies: [
            pkmsg_dep,
            columns.get_variable('columns_dep')
        ]
    )
endif

if get_option('BUILD_TESTS')
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