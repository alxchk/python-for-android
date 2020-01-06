from pythonforandroid.recipe import CompiledComponentsPythonRecipe
from os.path import join, basename
import sh

class PyuvRecipe(CompiledComponentsPythonRecipe):
    name = 'pyuv'
    version = '9d226dd61162a998745681eb87ef34e1a7d8586a'
    url = 'https://github.com/alxchk/pyuv/archive/{version}.zip'
    call_hostpython_via_targetpython = False
    depends = ['python2']
    patches = ['setup_libuv.patch']

    def get_recipe_env(self, arch):
        env = super(PyuvRecipe, self).get_recipe_env(arch)
        env['PYTHON_ROOT'] = self.ctx.get_python_install_dir()
        env['CFLAGS'] += ' -I' + env['PYTHON_ROOT'] + '/include/python2.7'
        env['LDSHARED'] = env['CC'] + ' -pthread -shared -Wl,-O1 -Wl,-Bsymbolic-functions'
        env['LDFLAGS'] += ' -L' + env['PYTHON_ROOT'] + '/lib' + \
                          ' -lpython2.7'
        return env

recipe = PyuvRecipe()
