from pythonforandroid.recipe import PythonRecipe


class RpycRecipe(PythonRecipe):
    version = '3.4.4'
    url = 'https://github.com/tomerfiliba/rpyc/archive/{version}.zip'
    depends = [('python2', 'python3crystax'), 'setuptools']
    site_packages_name = 'rpyc'
    call_hostpython_via_targetpython = False


recipe = RpycRecipe()
