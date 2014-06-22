#include <jni.h>
#include <errno.h>
#include <android/log.h>
#include <android_native_app_glue.h>

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "python", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "python", __VA_ARGS__))

#define PY_SSIZE_T_CLEAN
#include "Python.h"
#ifndef Py_PYTHON_H
    #error Python headers needed to compile C extensions, please install development version of Python.
#endif

static PyObject *androidembed_log(PyObject *self, PyObject *args) {
    char *logstr = NULL;
    if (!PyArg_ParseTuple(args, "s", &logstr)) {
        return NULL;
    }
    __android_log_print(ANDROID_LOG_INFO, "python", "%s", logstr);
    Py_RETURN_NONE;
}

static PyMethodDef AndroidEmbedMethods[] = {
    {"log", androidembed_log, METH_VARARGS,
     "Log on android platform"},
    {NULL, NULL, 0, NULL}
};

PyMODINIT_FUNC initandroidembed(void) {
    (void) Py_InitModule("androidembed", AndroidEmbedMethods);
}

int file_exists(const char * filename)
{
	FILE *file;
    if (file = fopen(filename, "r")) {
        fclose(file);
        return 1;
    }
    return 0;
}


void android_main(struct android_app* state) {
    app_dummy();
    LOGI("Starting minimal bootstrap.");

    char *env_argument = NULL;
    FILE *fd;

    LOGI("Initialize Python for Android");
    //env_argument = getenv("ANDROID_ARGUMENT");
    //setenv("ANDROID_APP_PATH", env_argument, 1);
	//setenv("PYTHONVERBOSE", "2", 1);
    Py_SetProgramName("python-android");
    Py_Initialize();
    //PySys_SetArgv(argc, argv);

    /* ensure threads will work.
     */
    PyEval_InitThreads();

    /* our logging module for android
     */
    initandroidembed();

    /* inject our bootstrap code to redirect python stdin/stdout
     * replace sys.path with our path
     */
    #if 0
    PyRun_SimpleString(
        "import sys, posix\n" \
        "private = posix.environ['ANDROID_PRIVATE']\n" \
        "argument = posix.environ['ANDROID_ARGUMENT']\n" \
        "sys.path[:] = [ \n" \
		"    private + '/lib/python27.zip', \n" \
		"    private + '/lib/python2.7/', \n" \
		"    private + '/lib/python2.7/lib-dynload/', \n" \
		"    private + '/lib/python2.7/site-packages/', \n" \
		"    argument ]\n")
    #endif

    PyRun_SimpleString(
        "import sys, androidembed\n" \
        "class LogFile(object):\n" \
        "    def __init__(self):\n" \
        "        self.buffer = ''\n" \
        "    def write(self, s):\n" \
        "        s = self.buffer + s\n" \
        "        lines = s.split(\"\\n\")\n" \
        "        for l in lines[:-1]:\n" \
        "            androidembed.log(l)\n" \
        "        self.buffer = lines[-1]\n" \
        "    def flush(self):\n" \
        "        return\n" \
        "sys.stdout = sys.stderr = LogFile()\n" \
		"import site; print site.getsitepackages()\n"\
		"print 'Android path', sys.path\n" \
        "print 'Android kivy bootstrap done. __name__ is', __name__");

    /* run it !
     */
    LOGI("Run user program, change dir and execute main.py");
    //chdir(env_argument);

	/* search the initial main.py
	 */
	char *main_py = "main.pyo";
	if ( file_exists(main_py) == 0 ) {
		if ( file_exists("main.py") )
			main_py = "main.py";
		else
			main_py = NULL;
	}

	if ( main_py == NULL ) {
		LOGW("No main.pyo / main.py found.");
		return;
	}

    fd = fopen(main_py, "r");
    if ( fd == NULL ) {
        LOGW("Open the main.py(o) failed");
        return;
    }

    /* run python !
     */
    PyRun_SimpleFile(fd, main_py);

    if (PyErr_Occurred() != NULL) {
        PyErr_Print(); /* This exits with the right code if SystemExit. */
        if (Py_FlushLine())
			PyErr_Clear();
    }

    /* close everything
     */
	Py_Finalize();
    fclose(fd);

    LOGW("Python for android ended.");
}
