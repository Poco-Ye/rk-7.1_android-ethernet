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
#ifndef CONFIGURATION_INCLUDED
#define CONFIGURATION_INCLUDED
#include <stdint.h>
#include <utils/Vector.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <utils/Log.h>
#include <utils/Errors.h>
#include <cutils/properties.h>
#include <binder/IPCThreadState.h>
#include <binder/ProcessState.h>
#include "tinyxml.h"
#include "tinystr.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>


namespace android {

typedef struct {
    char *name;
    char *value;
} keycode_value;

typedef struct {
    int16_t pid;
    int16_t tid;
} pid_tid;

typedef struct {
    int16_t usercode;
    Vector<keycode_value> keycode_value_vector;
    Vector<pid_tid> pid_tid_vector;
} parsedata;



class configuration {
public:
    Vector<keycode_value> keycode_value_vector;
    Vector<pid_tid> pid_tid_vector;
    parsedata *pparsedata = new parsedata;
    keycode_value *pkeycode_value = new keycode_value;
    pid_tid *ppid_tid = new pid_tid;

    int loadXML();

};


};

#endif
