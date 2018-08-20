
#include <pthread.h>
#include <jni.h>

#include "android/log.h"

/* These JNI management functions are taken from SDL2, but modified to refer to pyjnius */

/* #define LOG(n, x) __android_log_write(ANDROID_LOG_INFO, (n), (x)) */
/* #define LOGP(x) LOG("python", (x)) */
#define LOG_TAG "Python_android"
#define LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)


/* Function headers */
JNIEnv* Android_JNI_GetEnv(void);
static void Android_JNI_ThreadDestroyed(void*);
static void Android_JNI_SetupGlobalClassLoader(JNIEnv *j_env);

static pthread_key_t mThreadKey;
static JavaVM* mJavaVM;

jobject gClassLoader = NULL;
jmethodID gFindClassMethod = NULL;

/* Library init */
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved)
{
    JNIEnv *env;
    mJavaVM = vm;

    LOGI("JNI_OnLoad called [badservice] (JVM: %p)", vm);
    if ((*mJavaVM)->GetEnv(mJavaVM, (void**) &env, JNI_VERSION_1_4) != JNI_OK) {
        LOGE("Failed to get the environment using GetEnv()");
        return -1;
    }
    /*
     * Create mThreadKey so we can keep track of the JNIEnv assigned to each thread
     * Refer to http://developer.android.com/guide/practices/design/jni.html for the rationale behind this
     */
    if (pthread_key_create(&mThreadKey, Android_JNI_ThreadDestroyed) != 0) {
        __android_log_print(ANDROID_LOG_ERROR, "pyjniusjni", "Error initializing pthread key");
    }

	Android_JNI_SetupGlobalClassLoader(env);
	Android_JNI_GetEnv();

    return JNI_VERSION_1_6;
}

void Android_JNI_SetupGlobalClassLoader(JNIEnv *j_env)
{
	LOGI("SetupGlobalClassLoader #1");

    jclass PythonServiceClass = (*j_env)->FindClass(j_env, "org/kivy/android/PythonService");
	if ((*j_env)->ExceptionCheck(j_env)) {
		LOGE("SetupGlobalClassLoader: org/kivy/android/PythonService not found");
		return;
	}

	LOGI("SetupGlobalClassLoader #2 (%p)", PythonServiceClass);

	jclass ClassOfPythonServiceClass = (*j_env)->GetObjectClass(j_env, PythonServiceClass);
	if ((*j_env)->ExceptionCheck(j_env)) {
		LOGE("SetupGlobalClassLoader: org/kivy/android/PythonService GetObjectClass failed");
		return;
	}

	LOGI("SetupGlobalClassLoader #4 (%p)", ClassOfPythonServiceClass);

    jmethodID getClassLoader = (*j_env)->GetMethodID(
		j_env, ClassOfPythonServiceClass, "getClassLoader", "()Ljava/lang/ClassLoader;");

	if ((*j_env)->ExceptionCheck(j_env)) {
		LOGE("SetupGlobalClassLoader: getClassLoader not found");
		return;
	}

	LOGI("SetupGlobalClassLoader #5 (%p)", getClassLoader);

    gClassLoader = (*j_env)->CallObjectMethod(j_env, PythonServiceClass, getClassLoader);
	if ((*j_env)->ExceptionCheck(j_env)) {
		LOGE("SetupGlobalClassLoader: getClassLoader failed");
		gClassLoader = NULL;
		return;
	}

	LOGI("SetupGlobalClassLoader #6 ! (%p)", gClassLoader);

	gClassLoader = (*j_env)->NewGlobalRef(j_env, gClassLoader);
	if ((*j_env)->ExceptionCheck(j_env)) {
		LOGE("SetupGlobalClassLoader: NewGlobalRef failed");
		gClassLoader = NULL;
		return;
	}

	LOGI("SetupGlobalClassLoader #6  - NEW REF! (%p)", gClassLoader);

	jclass ClassLoaderClass = (*j_env)->GetObjectClass(j_env, gClassLoader);
	if ((*j_env)->ExceptionCheck(j_env)) {
		LOGE("SetupGlobalClassLoader: org/kivy/android/PythonService GetObjectClass failed");
		return;
	}

	LOGI("SetupGlobalClassLoader #7  - ClassLoaderClass (%p)", ClassLoaderClass);

	gFindClassMethod = (*j_env)->GetMethodID(
		j_env, ClassLoaderClass, "findClass", "(Ljava/lang/String;)Ljava/lang/Class;");

	if ((*j_env)->ExceptionCheck(j_env)) {
		LOGE("SetupGlobalClassLoader: gFindClassMethod not found");
		gFindClassMethod = NULL;
		return;
	}

	LOGI("SetupGlobalClassLoader #8  - FindClassMethod (%p)", gFindClassMethod);

	jclass testClass = (*j_env)->CallObjectMethod(
		j_env, gClassLoader, gFindClassMethod, (*j_env)->NewStringUTF(j_env, "org/kivy/android/PythonService"));
	if ((*j_env)->ExceptionCheck(j_env)) {
		LOGE("TEST FAILED");
		return;
	}

	LOGI("SetupGlobalClassLoader: success (%p, %p, %d, %08x)",
		gClassLoader, gFindClassMethod, (*j_env)->IsSameObject(j_env, PythonServiceClass, testClass), 0xb16b00b5);
}

JNIEnv* Android_JNI_GetEnv(void)
{
    /* From http://developer.android.com/guide/practices/jni.html
     * All threads are Linux threads, scheduled by the kernel.
     * They're usually started from managed code (using Thread.start), but they can also be created elsewhere and then
     * attached to the JavaVM. For example, a thread started with pthread_create can be attached with the
     * JNI AttachCurrentThread or AttachCurrentThreadAsDaemon functions. Until a thread is attached, it has no JNIEnv,
     * and cannot make JNI calls.
     * Attaching a natively-created thread causes a java.lang.Thread object to be constructed and added to the "main"
     * ThreadGroup, making it visible to the debugger. Calling AttachCurrentThread on an already-attached thread
     * is a no-op.
     * Note: You can call this function any number of times for the same thread, there's no harm in it
     */

    JNIEnv *env;
    int status = (*mJavaVM)->AttachCurrentThread(mJavaVM, &env, NULL);
    if(status < 0) {
        LOGE("failed to attach current thread");
        return 0;
    }

    /* From http://developer.android.com/guide/practices/jni.html
     * Threads attached through JNI must call DetachCurrentThread before they exit. If coding this directly is awkward,
     * in Android 2.0 (Eclair) and higher you can use pthread_key_create to define a destructor function that will be
     * called before the thread exits, and call DetachCurrentThread from there. (Use that key with pthread_setspecific
     * to store the JNIEnv in thread-local-storage; that way it'll be passed into your destructor as the argument.)
     * Note: The destructor is not called unless the stored value is != NULL
     * Note: You can call this function any number of times for the same thread, there's no harm in it
     *       (except for some lost CPU cycles)
     */
    pthread_setspecific(mThreadKey, (void*) env);

    LOGI("Android_JNI_GetEnv: %p", env);
    return env;
}

static void Android_JNI_ThreadDestroyed(void* value)
{
    /* The thread is being destroyed, detach it from the Java VM and set the mThreadKey value to NULL as required */
    JNIEnv *env = (JNIEnv*) value;
    if (env != NULL) {
        (*mJavaVM)->DetachCurrentThread(mJavaVM);
        pthread_setspecific(mThreadKey, NULL);
    }
}

void *WebView_AndroidGetJNIEnv()
{
    return Android_JNI_GetEnv();
}
