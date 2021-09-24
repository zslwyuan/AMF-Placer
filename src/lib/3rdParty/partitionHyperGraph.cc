
#include "../utils/ProcessFuncInterface.h"
#include "PaToH/patoh.h"
#include <iostream>

int main(int argc, const char **argv)
{

    key_t shmId = std::stoull(std::string(argv[1]));
    unsigned int shmSize = std::stoul(std::string(argv[2]));

    // std::cout << "receive shmId: " << shmId << " shmSize:" << shmSize << std::endl;

    ProcessFuncInterface *processFuncInterface = new ProcessFuncInterface(shmSize, shmId);

    char *shmBlcok = (char *)processFuncInterface->getSharedMemory();

    int *int_shmBlcok = (int *)shmBlcok;
    int numHyperNodes = *int_shmBlcok;
    int numPlacementNets = *(int_shmBlcok + 1);
    int numPlacementPins = *(int_shmBlcok + 2);
    double final_imbal = *(double *)(int_shmBlcok + 3);
    int *cut = int_shmBlcok + 5;
    int *cwghts = int_shmBlcok + 6;
    int *xpins = cwghts + numHyperNodes;
    int *pins = xpins + numPlacementNets + 1;
    int *partvec = pins + numPlacementPins;
    int *partweights = partvec + numHyperNodes; // 6 + numHyperNodes + numPlacementNets + 1 + numPlacementPins +

    // std::cout << "receive final_imbal: " << final_imbal << " numPlacementNets:" << numPlacementNets
    //           << " numHyperNodes:" << numHyperNodes << " numPlacementPins:" << numPlacementPins << std::endl;

    // Preprocessing
    PaToH_Parameters args;
    // Init
    PaToH_Initialize_Parameters(&args, PATOH_CONPART, PATOH_SUGPARAM_QUALITY);
    args.seed = 0;
    args._k = 2;
    args.final_imbal = final_imbal;
    args.seed = 20213654;
    PaToH_Check_User_Parameters(&args, true);
    PaToH_Alloc(&args, numHyperNodes, numPlacementNets, 1, cwghts, NULL, xpins, pins);
    // Processing
    PaToH_Part(&args, numHyperNodes, numPlacementNets, 1, 1, cwghts, NULL, xpins, pins, NULL, partvec, partweights,
               cut);
    // Free
    PaToH_Free();

    delete processFuncInterface;

    return 0;
}