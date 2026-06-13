subdir('src')

libnnssnd = library('nnssnd',
    dependencies: nitrosdk_dep,
    include_directories: nitrosystem_includes,
    pic: false,
    sources: libnnssnd_srcs,
    native: build_target != 'arm'
)

nitrosystem_libs += libnnssnd
