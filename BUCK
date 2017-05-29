with allow_unsafe_import():
    from subprocess import check_output

LLVM_BIN = read_config('llvm', 'bin', '/usr/bin')

def shell(*cmd):
    output = check_output(cmd)
    return output.replace('\n', ' ').split(' ')

def llvm_config(*args):
    return shell(LLVM_BIN + '/llvm-config', *args)

def llvm_src(path):
    return (path, llvm_config('--cxxflags'))

def llvm_binary(llvm_libs = [], linker_flags = [], **kwargs):
    llvm_flags = llvm_config('--system-libs', '--ldflags', '--libs', *llvm_libs)
    return cxx_binary(linker_flags = linker_flags + llvm_flags, **kwargs)

llvm_binary(
    name = 'calc',
    srcs = [
        'parser.cpp',
        llvm_src('calc.cpp'),
    ],
)
