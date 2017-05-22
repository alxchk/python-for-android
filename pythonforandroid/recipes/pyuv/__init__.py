from pythonforandroid.toolchain import (
    CompiledComponentsPythonRecipe,
    Recipe,
    current_directory,
    info,
    shprint,
)
from os.path import join, basename
import sh

class PyuvRecipe(CompiledComponentsPythonRecipe):
    name = 'pyuv'
    version = '1.3.0'
    url = 'https://github.com/alxchk/pyuv/archive/v1.x.zip'
    call_hostpython_via_targetpython = False
    depends = ['python2']
    patches = ['setup_libuv.patch']

    def get_recipe_env(self, arch):
        env = super(PyuvRecipe, self).get_recipe_env(arch)
        env['PYTHON_ROOT'] = self.ctx.get_python_install_dir()
        env['HOST'] = '-'.join(basename(env['CC'].split()[1]).split('-')[:-1])
        env['CFLAGS'] += ' -I' + env['PYTHON_ROOT'] + '/include/python2.7'
        env['LDSHARED'] = env['CC'] + ' -pthread -shared -Wl,-O1 -Wl,-Bsymbolic-functions'
        env['LDFLAGS'] += ' -L' + env['PYTHON_ROOT'] + '/lib' + \
                          ' -lpython2.7'
        return env

recipe = PyuvRecipe()
