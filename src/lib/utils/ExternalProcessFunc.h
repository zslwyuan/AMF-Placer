/**
 * @file ExternalProcessFunc.h
 * @author Tingyuan LIANG (tliang@connect.ust.hk)
 * @brief This header file contains the definitions of ExternalProcessFunc class and its internal modules and APIs
 * which acts as a wrapper of an external exectable for multi-process scenario with shared memory
 * @version 0.1
 * @date 2021-10-02
 *
 * @copyright Copyright (c) 2021 Reconfiguration Computing Systems Lab, The Hong Kong University of Science and
 * Technology. All rights reserved.
 *
 */

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

/**
 * @brief ExternalProcessFunc is a wrapper of an external exectable for multi-process scenario with shared memory
 *
 * With this wrapper, the external executable can communicate with this placer process via shared memeory.
 *
 */
class ExternalProcessFunc
{
  public:
    /**
     * @brief Construct a new External Process Func object
     *
     * @param executablePath the absolute path of the external executable
     * @param shmSize the total size of the shared memory
     * @param verbose whether the wrapper dumps information of this invocation.
     */
    ExternalProcessFunc(std::string executablePath, unsigned int shmSize, bool verbose)
        : executablePath(executablePath), shmSize(shmSize), verbose(verbose)
    {
        createShareMemory();
    }

    ~ExternalProcessFunc()
    {
        deleteShareMemory();
    }

    /**
     * @brief Create a Share Memory object with random id
     *
     */
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