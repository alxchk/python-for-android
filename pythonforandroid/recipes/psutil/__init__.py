from pythonforandroid.recipe import CompiledComponentsPythonRecipe
from os.path import join
import sh

class PsutilRecipe(CompiledComponentsPythonRecipe):
    name = 'psutil'
    version = '5.2.2'
    url = 'https://github.com/giampaolo/psutil/archive/release-{version}.tar.gz'
    call_hostpython_via_targetpython = False
    depends = ['python2']
    patches = ['ethtool.patch', 'mntent.patch', 'sched.patch', 'ifaddrs.patch', 
        'posix.patch', 'compat.patch', 'setup.py.patch']

    def get_recipe_env(self, arch):
        env = super(PsutilRecipe, self).get_recipe_env(arch)
        env['PYTHON_ROOT'] = self.ctx.get_python_install_dir()
        env['CFLAGS'] += ' -I' + env['PYTHON_ROOT'] + '/include/python2.7' + ' -Ipsutil/'
        env['LDSHARED'] = env['CC'] + ' -pthread -shared -Wl,-O1 -Wl,-Bsymbolic-functions'
        env['LDFLAGS'] += ' -L' + env['PYTHON_ROOT'] + '/lib' + \
                          ' -lpython2.7'
	return env


recipe = PsutilRecipe()
