/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "com_android_server_affinity_AffinityService"
//#define LOG_NDEBUG 0


#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include <pthread.h>
#include <errno.h>
#include <sys/types.h>
#include <dirent.h>
#include <grp.h>
#include <inttypes.h>
#include <pwd.h>
#include <time.h>
#include <poll.h>
#include <fcntl.h>

#include <sys/stat.h>
#include <sys/syscall.h>

#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <hardware/hardware.h>
#include <android/log.h>
#include "affinity/configuration.h"
#include "JNIHelp.h"
#include "jni.h"
#include <sched.h>

#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)


//#define ENABLE_DEBUG_LOG

/* ���Դ�ӡ��. */
#ifdef ENABLE_DEBUG_LOG
#define D(fmt, args...) \
    { LOGD("[Line] : %d; [Func] : %s() : " fmt, __LINE__, __FUNCTION__, ## args); }
#else
#define D(fmt, args...)
#endif

#define I(fmt, args...) \
    { LOGI("[Line] : %d; [Func] : %s() ; ! Info : " fmt, __LINE__, __FUNCTION__, ## args); }

/* Error Log. */
#define E(fmt, args...) \
    { LOGE("[Line] : %d; [Func] : %s() ; !!! Error : " fmt, __LINE__, __FUNCTION__, ## args); }

// ----------------------------------------------------------------------------

namespace android {
/*
 * Field/method IDs and class object references.
 *
 * You should not need to store the JNIEnv pointer in here.  It is
 * thread-specific and will be passed back in on every call.
 */
static struct {
    jclass      platformLibraryClass;
} gCachedState;

static configuration *mconfiguration = nullptr;
// ----------------------------------------------------------------------------

/*
 * Helper function to throw an arbitrary exception.
 *
 * Takes the exception class name, a format string, and one optional integer
 * argument (useful for including an error code, perhaps from errno).
 */
static void throwException(JNIEnv* env, const char* ex, const char* fmt,
                           int data)
{

    if (jclass cls = env->FindClass(ex)) {
        if (fmt != NULL) {
            char msg[1000];
            // snprintf(msg, sizeof(msg), fmt, data);
            env->ThrowNew(cls, msg);
        } else {
            env->ThrowNew(cls, NULL);
        }

        /*
         * This is usually not necessary -- local references are released
         * automatically when the native code returns to the VM.  It's
         * required if the code doesn't actually return, e.g. it's sitting
         * in a native event loop.
         */
        env->DeleteLocalRef(cls);
    }
}

jint com_android_server_affinity_AffinityService_init(JNIEnv *env, jclass clazz)
{
    int result=0;
	#ifdef DEBUG_AFFINITY
    D("zj add android_AffinityService_init\n");
    #endif
    if(mconfiguration == NULL) {
		#ifdef DEBUG_AFFINITY
        D("new configuration\n");
		#endif
        mconfiguration =new configuration();
    }
    result = mconfiguration->loadXML();
    return result;
}

static inline void print_cpu_mask(cpu_set_t cpu_mask)
{
    unsigned char flag = 0;
    D("Cpu affinity is ");
    for (unsigned int i = 0; i < sizeof(cpu_set_t); i ++) {
        if (CPU_ISSET(i, &cpu_mask)) {
            if (flag == 0) {
                flag = 1;
                D("%d", i);
            } else {
                D(",%d", i);
            }
        }
    }
    D(".\n");
}

void set_cur_thread_affinity(cpu_set_t *mask)
{
    int err, syscallres;
    pid_t tid = gettid();
	#ifdef DEBUG_AFFINITY
    print_cpu_mask(*mask);
	#endif
    syscallres = syscall(__NR_sched_setaffinity, tid, sizeof(cpu_set_t),mask);
    if (syscallres) {
        err = errno;
		#ifdef DEBUG_AFFINITY
        E("Error in the syscall setaffinity: mask = %d, err=%d",mask,errno);
		#endif
    }else{
        #ifdef DEBUG_AFFINITY
		D("mask = %d tid = %d has setted affinity success",mask,tid);
		#endif
    	}
}
/*
void set_thread_affinity(cpu_set_t *mask,int tidint)
{
    int err, syscallres;
	pid_t tid = tidint;
    print_cpu_mask(*mask);
    syscallres = syscall(__NR_sched_setaffinity,tid, sizeof(cpu_set_t),mask);
    if (syscallres) {
        err = errno;
        LOGE("Error in the syscall setaffinity: mask = %d, err=%d",mask,errno);
    }else{
		LOGD("mask = %d tid = %d has setted affinity success",mask,tid);
    	}
}
*/

void set_thread_affinity(cpu_set_t *mask,int tid)
{
    int err, syscallres;
	int ret = -1;
    print_cpu_mask(*mask);

    ret = sched_setaffinity(tid, sizeof(mask),mask);
    if(ret < 0) {
		#ifdef DEBUG_AFFINITY
        E("bind thread tid = %d sched_setaffinity return %d, errno = %d\n",tid,ret,errno);
		#endif
    } else {
        #ifdef DEBUG_AFFINITY
        D("bind thread tid = %d sched_setaffinity success!",tid);
		#endif
    }
}

static int getCores()
{
    return sysconf(_SC_NPROCESSORS_CONF);
}
static jint com_android_server_affinity_AffinityService_getCores(JNIEnv *env, jclass type)
{
    return getCores();
}

static void com_android_server_affinity_AffinityService_bindToCpu(JNIEnv *env, jclass type, jint cpu)
{
    int cores = getCores();
	#ifdef DEBUG_AFFINITY
    D("get cpu number = %d\n",cores);
	#endif
    if (cpu >= cores) {
		#ifdef DEBUG_AFFINITY
        E("your set cpu is beyond the cores,exit...");
		#endif
        return;
    }
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(cpu,&mask);
    set_cur_thread_affinity(&mask);
	#ifdef DEBUG_AFFINITY
    D("set affinity to %d success",cpu);
	#endif
}

static jint com_android_server_affinity_AffinityService_bindProcessToCpu45(JNIEnv* env, jobject obj)
{
    DIR *task_dir;
    struct dirent *tid_dir;
    int pid,tid;
    int ret;
    char filename[64];
	#ifdef DEBUG_AFFINITY
    D("BindThreadsToCpu45 : Current platform = rk3399");
	#endif
    cpu_set_t mask;
    struct sched_param param;
    CPU_ZERO(&mask);
    CPU_SET(4, &mask);
    CPU_SET(5, &mask);
// bind whole processId to cpu 4 & 5
    pid = getpid();
    #ifdef DEBUG_AFFINITY
    D("pid = %d",pid);
	#endif
    ret = sched_setaffinity(pid, sizeof(mask), &mask);
    if(ret < 0) {
		#ifdef DEBUG_AFFINITY
        E("BindThreadsToCpu45 -> bind process sched_setaffinity return %d, errno = %d\n",ret,errno);
		#endif
        return -1;
    } else {
        #ifdef DEBUG_AFFINITY
        D("BindThreadsToCpu45 -> bind process sched_setaffinity success!");
		#endif
    }
// bind all threads before
    /*
        sprintf(filename, "/proc/%d/task", pid);
        task_dir = opendir(filename);
        if (!task_dir) E("opendir failed!");
        while ((tid_dir = readdir(task_dir))) {
            if (!isdigit(tid_dir->d_name[0]))
                continue;
            tid = atoi(tid_dir->d_name);
            ret = sched_setaffinity(tid, sizeof(mask), &mask);
            if(ret < 0){
                E("BindThreadsToCpu45 -> bind thread %d sched_setaffinity return %d, errno = %d\n",tid,ret,errno);
                return -1;
            }else{
                D("BindThreadsToCpu45 -> bind thread %d sched_setaffinity success!",tid);
            }
        }*/
    return 0;
}

static void com_android_server_affinity_AffinityService_bindThreadToCpu45(JNIEnv *env, jclass type)
{
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(4,&mask);
    CPU_SET(5,&mask);
    if(mconfiguration->pid_tid_vector.size() == 0) {
		#ifdef DEBUG_AFFINITY
        E("com_android_server_affinity_AffinityService_bindThreadToCpu45 fail to get tids");
		#endif
        return;
    } else {
        for(Vector<pid_tid>::iterator it = mconfiguration->pid_tid_vector.begin(); it != mconfiguration->pid_tid_vector.end(); ++it ) {
			#ifdef DEBUG_AFFINITY
            D("set_thread_affinity tid = %d",(*it).tid);
			#endif
            set_thread_affinity(&mask,(*it).tid);
        }
    }
}

// ----------------------------------------------------------------------------

/*
 * Array of methods.
 *
 * Each entry has three fields: the name of the method, the method
 * signature, and a pointer to the native implementation.
 */
/****

Android JNI ʹ�õ����ݽṹJNINativeMethod���Java&Android 2009-09-06 20:42:56 �Ķ�234 ����0 �ֺţ�����С
Andoird ��ʹ����һ�ֲ�ͬ��ͳJava JNI�ķ�ʽ��������native�ĺ��������к���Ҫ��������Andoridʹ����һ��Java �� C ������ӳ������飬�������������˺����Ĳ����ͷ���ֵ����������������JNINativeMethod���������£�


typedef struct {
const char* name;
const char* signature;
void* fnPtr;
} JNINativeMethod;

��һ������name��Java�к��������֡�

�ڶ�������signature�����ַ����������˺����Ĳ����ͷ���ֵ

����������fnPtr�Ǻ���ָ�룬ָ��C������


���бȽ����������ǵڶ�������������

"()V"

"(II)V"

"(Ljava/lang/String;Ljava/lang/String;)V"


ʵ������Щ�ַ����뺯���Ĳ�������һһ��Ӧ�ġ�

"()" �е��ַ���ʾ������������������ֵ������"()V" �ͱ�ʾvoid Func();

"(II)V" ��ʾ void Func(int, int);


�����ÿһ���ַ��Ķ�Ӧ��ϵ����


�ַ� Java���� C����

V		void			void
Z		 jboolean	  boolean
I		  jint				int
J		 jlong			  long
D		jdouble 	  double
F		jfloat			  float
B		jbyte			 byte
C		jchar			char
S		jshort			short


��������"["��ʼ���������ַ���ʾ


[I 	  jintArray 	 int[]
[F 	jfloatArray    float[]
[B 	jbyteArray	  byte[]
[C    jcharArray	 char[]
[S    jshortArray	 short[]
[D    jdoubleArray double[]
[J 	jlongArray	   long[]
[Z    jbooleanArray boolean[]


����Ķ��ǻ������͡����Java�����Ĳ�����class������"L"��ͷ����";"��β�м�����"/" �����İ��������������Ӧ��C�������Ĳ�����Ϊjobject. һ��������String�࣬���Ӧ����Ϊjstring


Ljava/lang/String; String jstring
Ljava/net/Socket; Socket jobject


���JAVA����λ��һ��Ƕ���࣬����$��Ϊ������ķָ�����

���� "(Ljava/lang/String;Landroid/os/FileUtils$FileStatus;)Z"
****/

static const JNINativeMethod gMethods[] = {
    /* name,                        signature,      funcPtr */
    { "init_native","()I",(void *)com_android_server_affinity_AffinityService_init  },
    { "bindToCpu_native","(I)V",(void *)com_android_server_affinity_AffinityService_bindToCpu  },
    { "bindProcessToCpu45_native","()V",(void *)com_android_server_affinity_AffinityService_bindProcessToCpu45  },
    { "bindThreadToCpu45_native","()V",(void *)com_android_server_affinity_AffinityService_bindThreadToCpu45  },
};

/*
 * Do some (slow-ish) lookups now and save the results.
 *
 * Returns 0 on success.
 */
static int cacheIds(JNIEnv* env, jclass clazz)
{
    /*
     * Save the class in case we want to use it later.  Because this is a
     * reference to the Class object, we need to convert it to a JNI global
     * reference.
     */
    gCachedState.platformLibraryClass = (jclass) env->NewGlobalRef(clazz);      // .! : sample �е�ʵ��, �� HDMI ��ʵ��, ����Ҫ�ص� Java.
    if (clazz == NULL) {
        E("Can't create new global ref\n");
        return -1;
    }

    return 0;
}

/*
 * Explicitly register all methods for our class.
 *
 * While we're at it, cache some class references and method/field IDs.
 *
 * Returns 0 on success.
 */
int register_android_server_AffinityService(JNIEnv* env)
{
    static const char* const kClassName = "com/android/server/affinity/AffinityService";
    jclass clazz;

    /* look up the class */
    clazz = env->FindClass(kClassName);
    if (clazz == NULL) {
        E("Can't find class %s\n", kClassName);
        return -1;
    }
    I("have find class %s\n", kClassName);

    /* register all the methods */
    if (env->RegisterNatives(clazz, gMethods,
                             sizeof(gMethods) / sizeof(gMethods[0])) != JNI_OK) {
        E("Failed registering methods for %s\n", kClassName);
        return -1;
    }
    I("registering methods for %s\n", kClassName);

    /* fill out the rest of the ID cache */ // .! : �� affinity ʵ����, û�б�Ҫ cache Java �� or field �� ID.
    return cacheIds(env, clazz);
}
};
