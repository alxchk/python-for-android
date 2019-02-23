from pythonforandroid.recipe import PythonRecipe


class TinyECRecipe(PythonRecipe):
    version = '0.4'
    url = 'https://github.com/alxchk/tinyec/archive/master.zip'
    depends = [('python2', 'python3crystax'), 'setuptools']
    site_packages_name = 'tinyec'
    call_hostpython_via_targetpython = False


recipe = TinyECRecipe()
