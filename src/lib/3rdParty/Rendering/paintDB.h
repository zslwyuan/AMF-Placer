#ifndef _paintDB_
#define _paintDB_

#include <assert.h>
#include <fstream>
#include <iostream>
#include <map>
#include <mutex>
#include <semaphore.h>
#include <set>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

class PaintDataBase
{
  public:
    PaintDataBase()
    {
        Xs.clear();
        Ys.clear();
        elementTypes.clear();
        sem_init(&dataWritten, 0, 0);
    };
    std::vector<float> Xs, Ys;
    std::vector<int> elementTypes;
    std::mutex dataRWLock;
    sem_t dataWritten;

    inline void writeElementInfo(std::vector<float> &_Xs, std::vector<float> &_Ys, std::vector<int> &_elementTypes)
    {
        dataRWLock.lock();
        assert(_Xs.size() == _Ys.size() && _Xs.size() == _elementTypes.size());
        Xs = _Xs;
        Ys = _Ys;
        elementTypes = _elementTypes;
        dataRWLock.unlock();
        sem_post(&dataWritten);
    }

    inline bool readElementInfo(std::vector<float> &_Xs, std::vector<float> &_Ys, std::vector<int> &_elementTypes)
    {
        if (sem_trywait(&dataWritten) == 0)
        {
            dataRWLock.lock();
            _Xs = Xs;
            _Ys = Ys;
            _elementTypes = elementTypes;
            assert(_Xs.size() == _Ys.size() && _Xs.size() == _elementTypes.size());
            dataRWLock.unlock();
            return true;
        }
        return false;
    }
};

#endif