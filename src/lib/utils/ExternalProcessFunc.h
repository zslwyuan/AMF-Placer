#ifndef _EXTERNALPROCESSFUNC
#define _EXTERNALPROCESSFUNC

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

class ExternalProcessFunc
{
  public:
    ExternalProcessFunc(std::string executablePath, unsigned int shmSize, bool verbose)
        : executablePath(executablePath), shmSize(shmSize), verbose(verbose)
    {
        createShareMemory();
    }

    ~ExternalProcessFunc()
    {
        deleteShareMemory();
    }

    void createShareMemory()
    {
        sharedid = rand() + getpid();
        shmid = shmget((key_t)sharedid, shmSize, 0666 | IPC_CREAT);

        if (shmid == -1)
        {
            fprintf(stderr, "shmget failed\n");
            exit(EXIT_FAILURE);
        }

        shm = shmat(shmid, (void *)0, 0);
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

        if (shmctl(shmid, IPC_RMID, 0) == -1)
        {
            fprintf(stderr, "shmctl(IPC_RMID) failed\n");
            exit(EXIT_FAILURE);
        }
    }

    inline void *getSharedMemory()
    {
        return shm;
    }

    void execute()
    {
        if (verbose)
            print_cmd((executablePath + " " + std::to_string(sharedid) + " " + std::to_string(shmSize)));
        int rc = system((executablePath + " " + std::to_string(sharedid) + " " + std::to_string(shmSize)).c_str());
        assert(rc >= 0 && "the execution should be ended successfully but not.");
    }

  private:
    int shmid;
    void *shm = NULL;
    std::string executablePath;
    unsigned int shmSize;
    bool verbose;
    key_t sharedid;
};

#endif