subdir('src')

libnnsg2d_includes = include_directories('include')

libnnsg2d = library('nnsg2d',
    dependencies: nitrosdk_dep,
    include_directories: [nitrosystem_includes, libnnsg2d_includes],
    pic: build_target == 'nx',
    sources: libnnsg2d_srcs,
    native: (build_target == 'linux' or build_target == 'win64')
)

nitrosystem_libs += libnnsg2d
