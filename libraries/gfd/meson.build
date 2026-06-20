subdir('src')

libnnsgfd_includes = include_directories('include')

libnnsgfd = library('nnsgfd',
    dependencies: nitrosdk_dep,
    include_directories: [nitrosystem_includes, libnnsgfd_includes],
    pic: build_target == 'nx',
    sources: libnnsgfd_srcs,
    native: (build_target == 'linux' or build_target == 'win64')
)

nitrosystem_libs += libnnsgfd
