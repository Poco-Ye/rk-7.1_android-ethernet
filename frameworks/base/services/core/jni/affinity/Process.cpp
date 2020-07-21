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

#define LOG_TAG "Process"
//#define LOG_NDEBUG 0
//#define DEBUG_AFFINITY

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <dirent.h>
#include <ctype.h>
#include <pwd.h>
#include <stdlib.h>
#include <poll.h>
#include <sys/stat.h>
#include <signal.h>



#include <android/log.h>
#include <sys/types.h>
#define BUF_SIZE 1024
#include "Process.h"




using namespace android;

//能够获得更完整的列表，但是一些apk记录的/proc/%d/status不完整
void getNameByPid(pid_t pid, char *task_name)
{
    char proc_pid_path[BUF_SIZE];
    char buf[BUF_SIZE];

    sprintf(proc_pid_path, "/proc/%d/status", pid);
    FILE* fp = fopen(proc_pid_path, "r");
    if(NULL != fp) {
        if( fgets(buf, BUF_SIZE-1, fp)== NULL ) {
            fclose(fp);
        }
        fclose(fp);
        sscanf(buf, "%*s %s", task_name);
    }
}

int Process::getPid(const char *s)
{
    int result = 0;
    while (*s) {
        if (!isdigit(*s)) return -1;
        result = 10 * result + (*s++ - '0');
    }
    return result;
}

void Process::getProcessName(int pid, char *buffer, size_t max)
{
    int fd;
    snprintf(buffer, max, "/proc/%d/cmdline", pid);
    fd = open(buffer, O_RDONLY | O_CLOEXEC);
    if (fd < 0) {
        strcpy(buffer, "???");
    } else {
        int length = read(fd, buffer, max - 1);
        buffer[length] = 0;
        close(fd);
    }
}

void Process::getThreadName(int pid, int tid, char *buffer, size_t max)
{
    FILE * fp = NULL;
    snprintf(buffer, max, "/proc/%d/task/%d/status",pid,tid);
    fp = fopen(buffer, "r");
    if (fp == NULL) {
        strcpy(buffer, "???");
    } else {
        //t length = read(fp, buffer, max - 1);
        //buffer[length] = 0;
        char * line = NULL;
        size_t len = 0;
        ssize_t read;
        if((read = getline(&line, &len, fp)) != -1) {
			#ifdef DEBUG_AFFINITY
            SLOGD("Retrieved line of length %zu %zu:\n", read,len);
			#endif
            snprintf(buffer, read, line);
			#ifdef DEBUG_AFFINITY
            SLOGD("zj add [FILE] : %s; [Line] : %d; [Func] : %s() buffer =%s" , __FILE__,__LINE__, __FUNCTION__,buffer);
			#endif
        }

        if (line)
            free(line);
        if (fp != NULL)
            fclose(fp);
    }
}
void Process::getThreadStatus(int pid, int tid, char *buffer, size_t max)
{
    int fd;
    snprintf(buffer, max, "/proc/%d/task/%d/status",pid,tid);
    fd = open(buffer, O_RDONLY | O_CLOEXEC);
    if (fd < 0) {
        strcpy(buffer, "???");
    } else {
        int length = read(fd, buffer, max - 1);
        buffer[length] = 0;
        close(fd);
    }
}

/*
 * get linux pid from apk's packagename
 */
int Process::getPidFromProcessname(char *processname)
{
    DIR* dir;
    struct dirent* de;

    if (!(dir = opendir("/proc"))) {
        SLOGE("opendir failed (%s)", strerror(errno));
        return 0;
    }

    while ((de = readdir(dir))) {
        int pid = getPid(de->d_name);
        char name[PATH_MAX]= {0};

        if (pid == -1)
            continue;
        getProcessName(pid, name, sizeof(name));
        //getNameByPid(pid, name);
        #ifdef DEBUG_AFFINITY
        SLOGD("zj add [FILE] : %s; [Line] : %d; [Func] : %s() process pid = %d name = %s" , __FILE__,__LINE__, __FUNCTION__, pid,name);
		#endif
        if(strcmp(name,processname) ==0) {
			#ifdef DEBUG_AFFINITY
            SLOGD("zj add match process name = %s ,pid = %d",processname,pid);
			#endif
            closedir(dir);
            return pid;
        }
    }
    closedir(dir);
    return -1;
}

/*
 * get linux tid from apk's packagename and threadname
 */
int Process::getTidFromThreadname(int pid, char *threadname)
{
    DIR* dir;
    struct dirent* de;
    char proc_pid_task[PATH_MAX];
    char name[PATH_MAX];
    int tid;
    snprintf(proc_pid_task, sizeof(name), "/proc/%d/task",pid);

    if (!(dir = opendir(proc_pid_task))) {
		#ifdef DEBUG_AFFINITY
        SLOGE("opendir failed (%s)", strerror(errno));
		#endif
        return 0;
    }
    while ((de = readdir(dir))) {
        tid = getPid(de->d_name);
        char name[PATH_MAX];
        if (tid == -1)
            continue;
        getThreadName(pid, tid , name, sizeof(name));
#ifdef DEBUG_AFFINITY
        SLOGD("zj add [FILE] : %s; [Line] : %d; [Func] : %s() process pid = %d tid = %d name = %s" , __FILE__,__LINE__, __FUNCTION__, pid,tid,name);
#endif
        if(strstr(name,threadname) !=NULL) {
#ifdef DEBUG_AFFINITY
            SLOGD("zj add match thread name = %s ,tid = %d",threadname,tid);
#endif
            closedir(dir);
            return tid;
        }
    }
    closedir(dir);
    return -1;
}


/*
 * get linux tid from apk's packagename and threadname.
 There are situations where there are multiple threads of the same name
 */
int Process::getTidsFromThreadname(int pid, char *threadname,Vector<pid_tid> *pid_tid_vector,pid_tid *ppid_tid)
{
    DIR* dir;
    struct dirent* de;
    char proc_pid_task[PATH_MAX];
    char name[PATH_MAX];
    int tid;
    snprintf(proc_pid_task, sizeof(name), "/proc/%d/task",pid);

    if (!(dir = opendir(proc_pid_task))) {
		#ifdef DEBUG_AFFINITY
        SLOGE("opendir failed (%s)", strerror(errno));
		#endif
        return 0;
    }
    while ((de = readdir(dir))) {
        tid = getPid(de->d_name);
        char name[PATH_MAX];
        if (tid == -1)
            continue;
        getThreadName(pid, tid , name, sizeof(name));
		#ifdef DEBUG_AFFINITY
        SLOGD("zj add [FILE] : %s; [Line] : %d; [Func] : %s() process pid = %d tid = %d name = %s" , __FILE__,__LINE__, __FUNCTION__, pid,tid,name);
		#endif
        if(strstr(name,threadname) !=NULL) {
			#ifdef DEBUG_AFFINITY
            SLOGD("zj add match thread name = %s ,tid = %d",threadname,tid);
			#endif
            ppid_tid->pid = pid;
            ppid_tid->tid = tid;
            if(ppid_tid->pid > 0 && ppid_tid->tid >0 ) {
                pid_tid_vector->push_back(*ppid_tid);
            }
        }
    }
    closedir(dir);
    return -1;
}

