subdir('src')

libnnsfnd_includes = include_directories('include')

libnnsfnd = library('nnsfnd',
    dependencies: nitrosdk_dep,
    include_directories: [nitrosystem_includes, libnnsfnd_includes],
    pic: build_target == 'nx',
    sources: libnnsfnd_srcs,
    native: (build_target == 'linux' or build_target == 'win64')
)

nitrosystem_libs += libnnsfnd
