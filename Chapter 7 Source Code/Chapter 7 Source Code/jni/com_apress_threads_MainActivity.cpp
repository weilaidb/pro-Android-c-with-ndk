#include <stdio.h>
#include <unistd.h>

#include <pthread.h>

#include "com_apress_threads_MainActivity.h"

//log print printlog printdebug tiaoshidayin
#include <android/log.h>
#define LOG_TAG "MYJNI"
#define LOGI(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN , LOG_TAG, __VA_ARGS__)


// Native worker thread arguments
struct NativeWorkerArgs
{
	jint id;
	jint iterations;
};

// Method ID can be cached
static jmethodID gOnNativeMessage = NULL;

// Java VM interface pointer
static JavaVM* gVm = NULL;

// Global reference to object
static jobject gObj = NULL;

// Mutex instance
static pthread_mutex_t mutex;

jint JNI_OnLoad (JavaVM* vm, void* reserved)
{
	// Cache the JavaVM interface pointer
	gVm = vm;

	return JNI_VERSION_1_4;
}

void Java_com_apress_threads_MainActivity_nativeInit (
		JNIEnv* env,
		jobject obj)
{
	// Initialize mutex
	if (0 != pthread_mutex_init(&mutex, NULL))
	{
		// Get the exception class
		jclass exceptionClazz = env->FindClass(
				"java/lang/RuntimeException");

		// Throw exception
		env->ThrowNew(exceptionClazz, "Unable to initialize mutex");
		goto exit;
	}

	// If object global reference is not set
	if (NULL == gObj)
	{
		// Create a new global reference for the object
		gObj = env->NewGlobalRef(obj);

		if (NULL == gObj)
		{
			goto exit;
		}
	}

	// If method ID is not cached
	if (NULL == gOnNativeMessage)
	{
		// Get the class from the object
		jclass clazz = env->GetObjectClass(obj);

		// Get the method id for the callback
		gOnNativeMessage = env->GetMethodID(clazz,
				"onNativeMessage",
				"(Ljava/lang/String;)V");

		// If method could not be found
		if (NULL == gOnNativeMessage)
		{
			// Get the exception class
			jclass exceptionClazz = env->FindClass(
					"java/lang/RuntimeException");

			// Throw exception
			env->ThrowNew(exceptionClazz, "Unable to find method");
		}
	}

exit:
	return;
}

void Java_com_apress_threads_MainActivity_nativeFree (
		JNIEnv* env,
		jobject obj)
{
	// If object global reference is set
	if (NULL != gObj)
	{
		// Delete the global reference
		env->DeleteGlobalRef(gObj);
		gObj = NULL;
	}

	// Destory mutex
	if (0 != pthread_mutex_destroy(&mutex))
	{
		// Get the exception class
		jclass exceptionClazz = env->FindClass(
				"java/lang/RuntimeException");

		// Throw exception
		env->ThrowNew(exceptionClazz, "Unable to destroy mutex");
	}
}

void Java_com_apress_threads_MainActivity_nativeWorker (
		JNIEnv* env,
		jobject obj,
		jint id,
		jint iterations)
{
	// Lock mutex
	if (0 != pthread_mutex_lock(&mutex))
	{
		// Get the exception class
		jclass exceptionClazz = env->FindClass(
				"java/lang/RuntimeException");

		// Throw exception
		env->ThrowNew(exceptionClazz, "Unable to lock mutex");
		goto exit;
	}

	// Loop for given number of iterations
	for (jint i = 0; i < iterations; i++)
	{
		// Prepare message
		char message[256];// may be sizeof(message) overflow,it's may be small
//		char *message = new char[26];

//		memset(message,0, sizeof(message));
		sprintf(message, "LMR Worker %d: Iteration %d", id, i);

		// Message from the C string
		jstring messageString = env->NewStringUTF(message);
	    if (messageString == NULL) {
	    	LOGE("messageString is NULL" );
	        continue;
	    }
//		LOGI("message----------: %s" ,  messageString);//jstring not direct print
		LOGE("message -->:%s" , message);


//		LOGI("MyJNI is called!");
		// Call the on native message method
		env->CallVoidMethod(obj, gOnNativeMessage, messageString);
		if (messageString != NULL) {
			env->DeleteLocalRef(messageString);
		}
//		delete (message);
		/* delete local reference */
//		(*env)->DeleteLocalRef (env, str);

//		jclass ref= (env)->FindClass("java/lang/String");
//		env->DeleteLocalRef(ref);

		// Check if an exception occurred
		if (NULL != env->ExceptionOccurred())
			break;

		// Sleep for a second
//		sleep(1);
		usleep(5000);
	}

	// Unlock mutex
	if (0 != pthread_mutex_unlock(&mutex))
	{
		// Get the exception class
		jclass exceptionClazz = env->FindClass(
				"java/lang/RuntimeException");

		// Throw exception
		env->ThrowNew(exceptionClazz, "Unable to unlock mutex");
	}

exit:
	return;
}

static void* nativeWorkerThread (void* args)
{
	JNIEnv* env = NULL;

	// Attach current thread to Java virtual machine
	// and obrain JNIEnv interface pointer
	if (0 == gVm->AttachCurrentThread(&env, NULL))
	{
		// Get the native worker thread arguments
		NativeWorkerArgs* nativeWorkerArgs = (NativeWorkerArgs*) args;

		// Run the native worker in thread context
		Java_com_apress_threads_MainActivity_nativeWorker(env,
				gObj,
				nativeWorkerArgs->id,
				nativeWorkerArgs->iterations);

		// Free the native worker thread arguments
		delete nativeWorkerArgs;

		// Detach current thread from Java virtual machine
		gVm->DetachCurrentThread();
	}

	return (void*) 1;
}

void Java_com_apress_threads_MainActivity_posixThreads (
		JNIEnv* env,
		jobject obj,
		jint threads,
		jint iterations)
{
	// Create a POSIX thread for each worker
	for (jint i = 0; i < threads; i++)
	{
		// Thread handle
		pthread_t thread;

		// Native worker thread arguments
		NativeWorkerArgs* nativeWorkerArgs = new NativeWorkerArgs();
		nativeWorkerArgs->id = i;
		nativeWorkerArgs->iterations = iterations;

		// Create a new thread
		int result = pthread_create(
				&thread,
				NULL,
				nativeWorkerThread,
				(void*) nativeWorkerArgs);

		if (0 != result)
		{
			// Get the exception class
			jclass exceptionClazz = env->FindClass(
					"java/lang/RuntimeException");

			// Throw exception
			env->ThrowNew(exceptionClazz, "Unable to create thread");
		}
	}
}
