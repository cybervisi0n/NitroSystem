subdir('src')

libnnssnd = library('nnssnd',
    dependencies: nitrosdk_dep,
    include_directories: nitrosystem_includes,
    pic: build_target == 'nx',
    sources: libnnssnd_srcs,
    native: (build_target == 'linux' or build_target == 'win64')
)

nitrosystem_libs += libnnssnd
