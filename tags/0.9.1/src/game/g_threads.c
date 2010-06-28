#include "g_local.h"
#include "q_shared.h"

#ifdef __linux__
	#include <gnu/lib-names.h>
#elif defined( __MACOS__ )
	#define LIBPTHREAD_SO "/usr/lib/libpthread.dylib"
#elif defined( __APPLE__ )
	#define LIBPTHREAD_SO "/usr/lib/libpthread.dylib"
#endif

#ifndef WIN32
#include <pthread.h>
#include <dlfcn.h>


// tjw: handle for libpthread.so
void *g_pthreads = NULL;

// tjw: pointer to pthread_create() from libpthread.so
static int (*g_pthread_create)
	(pthread_t  *,
	__const pthread_attr_t *,
	void * (*)(void *),
	void *) = NULL;


void G_InitThreads(void)
{
	if(g_pthreads != NULL) {
		G_Printf("pthreads already loaded\n");
		return;
	}
	g_pthreads = dlopen(LIBPTHREAD_SO, RTLD_NOW);
	if(g_pthreads == NULL) {
		G_Printf("could not load libpthread\n%s\n",
			dlerror());
		return;
	}
	G_Printf("loaded libpthread\n");
	g_pthread_create = dlsym(g_pthreads,"pthread_create");
	if(g_pthread_create == NULL) {
		G_Printf("could not locate pthread_create\n%s\n",
			dlerror());
		return;
	}
	G_Printf("found pthread_create\n");	
}

int
create_thread(void *(*thread_function)(void *),void *arguments) {
	pthread_t thread_id;

	if(g_pthread_create == NULL) {
		// tjw: pthread_create() returns non 0 for failure
		//      but I don't know what's proper here.
		return -1;
	}
	return g_pthread_create(&thread_id, NULL, thread_function,arguments);
}

#else //WIN32
#include <process.h>

void G_InitThreads(void)
{
	// forty - we can have thread support in win32 we need to link with the MT runtime and use _beginthread
	G_Printf("Threading enabled.\n");
}

int create_thread(void *(*thread_function)(void *),void *arguments) {
	void *(*func)(void *) = /*(void *)*/thread_function;

	//Yay - no complaining
	_beginthread((void ( *)(void *))func, 0, arguments);
	return 0;
}
#endif
