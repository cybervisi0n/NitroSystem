subdir('src')

libnnsg2d_includes = include_directories('include')

libnnsg2d = library('nnsg2d',
    dependencies: nitrosdk_dep,
    include_directories: [nitrosystem_includes, libnnsg2d_includes],
    pic: false,
    sources: libnnsg2d_srcs,
    native: build_target != 'arm'
)

nitrosystem_libs += libnnsg2d
