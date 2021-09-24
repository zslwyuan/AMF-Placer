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