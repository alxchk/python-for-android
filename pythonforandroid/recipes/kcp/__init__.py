from pythonforandroid.recipe import CompiledComponentsPythonRecipe
from os.path import join, basename
import sh

class KCPRecipe(CompiledComponentsPythonRecipe):
    name = 'kcp'
    version = 'master'
    url = 'git+https://github.com/alxchk/pykcp.git'
    call_hostpython_via_targetpython = False
    depends = ['python2']
    patches = ['setup.py.patch']

    def get_recipe_env(self, arch):
        env = super(KCPRecipe, self).get_recipe_env(arch)
        env['PYTHON_ROOT'] = self.ctx.get_python_install_dir()
        env['CFLAGS'] += ' -I' + env['PYTHON_ROOT'] + '/include/python2.7'
        env['LDSHARED'] = env['CC'] + ' -pthread -shared -Wl,-O1 -Wl,-Bsymbolic-functions'
        env['LDFLAGS'] += ' -L' + env['PYTHON_ROOT'] + '/lib' + \
                          ' -lpython2.7'
        return env

recipe = KCPRecipe()
