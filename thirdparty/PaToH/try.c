#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "patoh.h"


#define NCONST        2
#define WDIFF         10


#define umax(a, b)	(((a) >= (b)) ? (a) : (b))

#define usRandom(s)     srand(s)
#define uRandom(l)      (rand() % l)



void usage(char *name)
{
 printf("usage: %s <partitioner-char> <hypergraph-filename> <#parts>\n", name);
 printf("\t<partitioner-char> can be:\n");
 printf("\t\tP: regular partition\n");
 printf("\t\tF: with first #parts cells are fixed to a different part\n");
 printf("\t\tM: %d-constraint partition\n", NCONST);
 printf("\t\tA: run all of them\n");
 printf("\t\tT: run all of them with skewed target weights\n");
 exit(1);
}


void PrintInfo(char *pname, int _k, int *partweights, int cut, int _nconst, float *targetweights)
{
    double             *tot, *maxi;
    int                i, j;


    tot = (double *) malloc(sizeof(double)*_nconst);
    maxi = (double *) malloc(sizeof(double)*_nconst);
    
    /* normalize target weight sum to 1.0 */
    for (j=0; j<_nconst; ++j)
        tot[j] = 0.0;
    for (i=0; i<_k; ++i)
        for (j=0; j<_nconst; ++j)
            tot[j] += targetweights[i*_nconst+j];
    for (i=0; i<_k; ++i)
        for (j=0; j<_nconst; ++j)
            targetweights[i*_nconst+j] /= tot[j];
    
    printf("\n-------------------------------------------------------------------");
    printf("\n Partitioner: %s", pname);
    
    printf("\n %d-way cutsize = %d \n", _k, cut);
    
    printf("\nPart Weights are:\n");
    for (i=0; i<_nconst; ++i)
        maxi[i] = tot[i] = 0.0;     
    for (i=0; i<_k; ++i)
        for (j=0; j<_nconst; ++j)
            tot[j] += partweights[i*_nconst+j];
    for (i=0; i<_nconst; ++i) 
        maxi[i] = 0.0;     

    for (i=0; i<_k; ++i) {
        printf("\n %3d :", i);
        for (j=0; j<_nconst; ++j) {
            double im= (double)((double)partweights[i*_nconst+j] - tot[j]*targetweights[i*_nconst+j]) / (tot[j]*targetweights[i*_nconst+j]);
         
            maxi[j] = umax(maxi[j], im);
            printf("%10d (TrgtR:%.3f  imbal:%6.3lf%%) ", partweights[i*_nconst+j], targetweights[i*_nconst+j], 100.0*im);
        }     
    }

    printf("\n MaxImbals are:");
    printf("\n      ");
    for (i=0; i<_nconst; ++i)
        printf("%10.1lf%% ", 100.0*maxi[i]);
    printf("\n"); 
    free(maxi);
    free(tot);
}


int main(int argc, char *argv[])
{
 PaToH_Parameters args;
 int             _c, _n, _nconst, *cwghts, *nwghts, 
     *xpins, *pins, *partvec, cut, *partweights,
     i, j, *cw;
 char           PT;
 float  *targetweights, *skewedtweights;


 if (argc!=4)
     usage(argv[0]);     

 PT = toupper(*argv[1]);
 if (PT!='P' && PT!='F' && PT!='M' && PT!='A' && PT!='T')
     usage(argv[0]);
 usRandom(time(NULL));
 
 
 PaToH_Read_Hypergraph(argv[2], &_c, &_n, &_nconst, &cwghts, 
                       &nwghts, &xpins, &pins);

 printf("===============================================================================\n");
 
 printf("Hypergraph %10s -- #Cells=%6d  #Nets=%6d  #Pins=%8d #Const=%2d\n",
        argv[2], _c, _n, xpins[_n], _nconst);
 
 PaToH_Initialize_Parameters(&args, PATOH_CONPART, 
                             PATOH_SUGPARAM_DEFAULT);

 args._k = atoi(argv[3]);
 args._k = (args._k>2) ? args._k : 2;
 args.seed = 1;
 
 partvec =  (int *) malloc(_c*sizeof(int));
 partweights  = (int *) malloc(NCONST*args._k*sizeof(int));
 targetweights = (float *) malloc(args._k*NCONST*sizeof(float));
 skewedtweights = (float *) malloc(args._k*NCONST*sizeof(float));

 for (i=0; i<args._k; ++i) {
     targetweights[i] = 1.0/(float)args._k;
     skewedtweights[i] = args._k-i;
 }
 
 PaToH_Alloc(&args, _c, _n, NCONST, cwghts, nwghts, 
             xpins, pins);

 

 if (PT=='P' || PT=='A') {     
     PaToH_Partition(&args, _c, _n, cwghts, nwghts, 
                     xpins, pins, partvec, partweights, &cut);
     PrintInfo("Single Constraint", args._k, partweights, cut, 1, targetweights);
     }
 if (PT=='T') {     
     PaToH_Part(&args, _c, _n, 1, 0, cwghts, nwghts, 
                xpins, pins, skewedtweights, partvec, partweights, &cut);

     printf("Skewed tweights:");
     for (i=0; i<args._k; ++i)
         printf("%.3f ", skewedtweights[i]);
     printf("\n");
     PrintInfo("Single Constraint Skewed", args._k, partweights, cut, 1, skewedtweights);
     }
     
 

 if (PT=='F' || PT=='A') {
     memset(partvec, 0xff, _c*sizeof(int));
     for (i=0; i<args._k; ++i)
         partvec[i] = i;
     PaToH_Partition_with_FixCells(&args, _c, _n, cwghts, nwghts, 
                                   xpins, pins, partvec, partweights, &cut);
     PrintInfo("With FixCells", args._k, partweights, cut, 1, targetweights);
     }
 if (PT=='T') {     
     memset(partvec, 0xff, _c*sizeof(int));
     for (i=0; i<args._k; ++i)
         partvec[i] = i;
     PaToH_Part(&args, _c, _n, 1, 1, cwghts, nwghts, 
                xpins, pins, skewedtweights, partvec, partweights, &cut);
     PrintInfo("With FixCells Skewed", args._k, partweights, cut, 1, skewedtweights);
     }


 if (PT=='M' || PT=='A' || PT=='T') {
     _nconst = NCONST;
     
     cw =  (int *) malloc(_c*_nconst*sizeof(int));
     for (i=0; i<_c; ++i) {
         for (j=0; j<_nconst; ++j) {
             int w=cwghts[i]+(uRandom(WDIFF)-WDIFF/2)*j;
             cw[i*_nconst+j] = umax(w, 1);
             }
     }

      for (i=0; i<args._k; ++i)
          for (j=0; j<_nconst;++j) {
              targetweights[i*_nconst+j] = 1.0/(float)args._k;
              skewedtweights[i*_nconst+j] = args._k-i;
          }


     if (PT=='T') {
         PaToH_Part(&args, _c, _n, _nconst, 0, cw, nwghts, 
                    xpins, pins, skewedtweights, partvec, partweights, &cut);
         PrintInfo("MultiConstraint Skewed", args._k, partweights, cut, _nconst, skewedtweights);
     } else {
         PaToH_MultiConst_Partition(&args, _c, _n, _nconst, cw, xpins, pins, partvec, partweights, &cut);
         PrintInfo("MultiConstraint", args._k, partweights, cut, _nconst, targetweights);
     }
     free(cw);
     }

 
 
 free(targetweights);
 free(partweights);
 free(partvec);
 free(cwghts);
 free(nwghts);
 free(xpins);
 free(pins);
  
PaToH_Free();
printf("\n");
 
return 0;
}

