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
        paths.clear();
        sem_init(&dataWritten, 0, 0);
    };
    std::vector<float> Xs, Ys;
    std::vector<int> elementTypes;
    std::vector<std::string> cellNames;
    std::vector<std::vector<int>> paths;
    std::mutex dataRWLock, demandRWLock;
    sem_t dataWritten;
    int criticalPathNum = 1;

    inline void getPaintDemand(int &_criticalPathNum)
    {
        demandRWLock.lock();
        _criticalPathNum = criticalPathNum;
        demandRWLock.unlock();
    }

    inline void setPaintDemand(int &_criticalPathNum)
    {
        demandRWLock.lock();
        criticalPathNum = _criticalPathNum;
        demandRWLock.unlock();
    }

    inline void writeElementInfo(std::vector<float> &_Xs, std::vector<float> &_Ys, std::vector<int> &_elementTypes,
                                 std::vector<std::vector<int>> &_paths, std::vector<std::string> &_cellNames)
    {
        dataRWLock.lock();
        assert(_Xs.size() == _Ys.size() && _Xs.size() == _elementTypes.size());
        Xs = _Xs;
        Ys = _Ys;
        elementTypes = _elementTypes;
        paths = _paths;
        cellNames = _cellNames;
        dataRWLock.unlock();
        sem_post(&dataWritten);
    }

    inline bool readElementInfo(std::vector<float> &_Xs, std::vector<float> &_Ys, std::vector<int> &_elementTypes,
                                std::vector<std::vector<int>> &_paths, std::vector<std::string> &_cellNames)
    {
        if (sem_trywait(&dataWritten) == 0)
        {
            dataRWLock.lock();
            _Xs = Xs;
            _Ys = Ys;
            _elementTypes = elementTypes;
            _paths = paths;
            _cellNames = cellNames;
            assert(_Xs.size() == _Ys.size() && _Xs.size() == _elementTypes.size());
            dataRWLock.unlock();
            return true;
        }
        return false;
    }
};

#endif