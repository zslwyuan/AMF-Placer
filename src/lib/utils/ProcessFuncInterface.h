/**
 * @file ProcessFuncInterface.h
 * @author Tingyuan LIANG (tliang@connect.ust.hk)
 * @brief
 * @version 0.1
 * @date 2021-10-02
 *
 * @copyright Copyright (c) 2021 Reconfiguration Computing Systems Lab, The Hong Kong University of Science and
 * Technology. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#ifndef _PROCESSFUNCINTERFACE
#define _PROCESSFUNCINTERFACE

#include "strPrint.h"
#include <assert.h>
#include <fcntl.h>
#include <functional>
#include <iomanip>
#include <iostream>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <unordered_set>

class ProcessFuncInterface
{
  public:
    ProcessFuncInterface(unsigned int shmSize, key_t sharedid) : shmSize(shmSize), sharedid(sharedid)
    {
        createShareMemory();
    }

    ~ProcessFuncInterface()
    {
        deleteShareMemory();
    }

    void createShareMemory()
    {
        shmid = shmget((key_t)sharedid, shmSize, 0666 | IPC_CREAT);
        if (shmid == -1)
        {
            fprintf(stderr, "shmget failed\n");
            exit(EXIT_FAILURE);
        }
        shm = shmat(shmid, 0, 0);
        if (shm == (void *)-1)
        {
            fprintf(stderr, "shmat failed\n");
            exit(EXIT_FAILURE);
        }
        // printf("Memory attached at %llX\n", (unsigned long long)shm);
    }

    void deleteShareMemory()
    {
        if (shmdt(shm) == -1)
        {
            fprintf(stderr, "shmdt failed\n");
            exit(EXIT_FAILURE);
        }
    }

    inline void *getSharedMemory()
    {
        return shm;
    }

  private:
    int shmid;
    void *shm = NULL;
    unsigned int shmSize;
    key_t sharedid;
};

#endif