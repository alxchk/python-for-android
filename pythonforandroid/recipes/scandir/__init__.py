import os
from pythonforandroid.recipe import CompiledComponentsPythonRecipe


class ScandirRecipe(CompiledComponentsPythonRecipe):
    name = 'scandir'
    version = '1.9'
    url = 'https://github.com/benhoyt/scandir/archive/v{version}.0.zip'

    depends = [('python2', 'python3crystax')]
    patches = ['setup.py.patch']

    call_hostpython_via_targetpython = False

    def get_recipe_env(self, arch=None):
        env = super(ScandirRecipe, self).get_recipe_env(arch)
        # sets linker to use the correct gcc (cross compiler)
        env['LDSHARED'] = env['CC'] + ' -pthread -shared -Wl,-O1 -Wl,-Bsymbolic-functions'
        env['LDFLAGS'] = (env.get('CFLAGS', '') + ' -L' +
                          self.ctx.get_libs_dir(arch.arch))
        env['LDFLAGS'] += ' -L{}'.format(os.path.join(self.ctx.bootstrap.build_dir, 'libs', arch.arch))

        python_version = self.ctx.python_recipe.version[0:3]
        ndk_dir_python = os.path.join(self.ctx.ndk_dir, 'sources/python/', python_version)
        env['LDFLAGS'] += ' -L{}'.format(os.path.join(ndk_dir_python, 'libs', arch.arch))
        env['LDFLAGS'] += " --sysroot={}".format(self.ctx.ndk_platform)
        env['PYTHONPATH'] = ':'.join([
            self.ctx.get_site_packages_dir(),
            env['BUILDLIB_PATH'],
        ])

        if self.ctx.ndk == 'crystax':
            # only keeps major.minor (discards patch)
            env['LDFLAGS'] += ' -lpython{}m'.format(python_version)
            # until `pythonforandroid/archs.py` gets merged upstream:
            # https://github.com/kivy/python-for-android/pull/1250/files#diff-569e13021e33ced8b54385f55b49cbe6
        else:
            env['LDFLAGS'] += ' -lpython{}'.format(python_version)

        env['CFLAGS'] += ' -I{}/include/python/'.format(ndk_dir_python)

        return env


recipe = ScandirRecipe()
