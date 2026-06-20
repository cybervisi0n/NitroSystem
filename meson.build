project('NitroSystem', 'c',
    default_options: {
        'default_library': 'static',
        'prefer_static':    true
    },
    meson_version: '>=1.2.0',
    version: '071126.1'
)

buildtype = get_option('buildtype')
build_target = get_option('build_target')

############################################################
###                    COMPILER FLAGS                    ###
############################################################

if not meson.is_subproject() and buildtype != 'plain' and build_target == 'arm'
c_args = [
    '-O4,p',
    '-proc', 'arm946e',
    '-enum', 'int',
    '-lang', 'c99',
    '-Cpp_exceptions', 'off',
    '-gccext,on',
    '-msgstyle', 'gcc',
    '-ipa', 'file',
    '-interworking',
    '-inline', 'on,noauto',
    '-char', 'signed',
    '-nosyspath',
    '-stdinc'
]
else
c_args = []
endif

mwrap_ver = get_option('mwrap_ver')
if mwrap_ver != 'default' and build_target == 'arm'
    c_args += ['-wrap:ver', mwrap_ver]
endif

mwrap_sdk = get_option('mwrap_sdk')
if mwrap_sdk != 'default' and build_target == 'arm'
    c_args += ['-wrap:sdk', mwrap_sdk]
endif

add_project_arguments(c_args,
    language: 'c',
    native: (build_target == 'linux' or build_target == 'win64')
)

nitrosystem_args = [
    '-DSDK_CW_FORCE_EXPORT_SUPPORT',
    '-DSDK_TS',
    '-DSDK_4M',
    '-DSDK_FINALROM',
    '-DNNS_FINALROM'
]

if (build_target == 'arm')
    nitrosystem_args += [
        '-DSDK_ARM9',
        '-DSDK_CW',
        '-DSDK_CODE_ARM',
        '-DSDK_BUILD_ARM'
    ]
endif

if (build_target == 'win64')
    nitrosystem_args += [
        '-DSDK_PORT',
        '-DSDK_X86',
        '-DSDK_CODE_X86',
        '-DSDK_BUILD_WIN64',
        '-D_NITRO',
        '-w'
    ]
endif

if (build_target == 'linux')
    nitrosystem_args += [
        '-DSDK_PORT',
        '-DSDK_X86',
        '-DSDK_CODE_X86',
        '-DSDK_BUILD_LINUX',
        '-D_NITRO',
        '-w'
    ]
endif

add_project_arguments(nitrosystem_args,
    language: 'c',
    native: (build_target == 'linux' or build_target == 'win64')
)

############################################################
###                    INCLUDE PATHS                     ###
############################################################
nitrosystem_includes = include_directories('include')

############################################################
###                     DEPENDENCIES                     ###
############################################################
nitrosdk_dep = dependency('NitroSDK')

############################################################
###                      LIBRARIES                       ###
############################################################
subdir('libraries')

############################################################
###                    FOR CONSUMERS                     ###
############################################################
nitrosystem_dep = declare_dependency(
    compile_args: nitrosystem_args,
    include_directories: nitrosystem_includes,
    link_with: nitrosystem_libs
)

meson.override_dependency('NitroSystem', nitrosystem_dep)
