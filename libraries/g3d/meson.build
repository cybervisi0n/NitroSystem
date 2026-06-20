subdir('src')

libnnsg3d = library('nnsg3d',
    dependencies: nitrosdk_dep,
    include_directories: nitrosystem_includes,
    pic: build_target == 'nx',
    sources: libnnsg3d_srcs,
    native: (build_target == 'linux' or build_target == 'win64')
)

nitrosystem_libs += libnnsg3d
