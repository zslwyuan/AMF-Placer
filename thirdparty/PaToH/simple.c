#include <stdio.h>
#include <stdlib.h>
#include "patoh.h"


void PrintInfo(int _k, int *partweights, int cut, int _nconst)
{
 double             *avg, *maxi, maxall=-1.0;
 int                i, j;

 printf("\n-------------------------------------------------------------------");
 printf("\n Partitioner: %s", (_nconst>1) ? "Multi-Constraint" : "Single-Constraint");

 printf("\n %d-way cutsize = %d \n", _k, cut);

 printf("\nPartWeights are:\n");
 avg = (double *) malloc(sizeof(double)*_nconst);
 maxi = (double *) malloc(sizeof(double)*_nconst);
 for (i=0; i<_nconst; ++i)
     maxi[i] = avg[i] = 0.0;
 for (i=0; i<_k; ++i)
       for (j=0; j<_nconst; ++j)
         avg[j] += partweights[i*_nconst+j];
  for (i=0; i<_nconst; ++i)
     {
     maxi[i] = 0.0;
     avg[i] /= (double) _k;
     }

 for (i=0; i<_k; ++i)
     {
     printf("\n %3d :", i);
     for (j=0; j<_nconst; ++j)
         {
         double im= (double)((double)partweights[i*_nconst+j] - avg[j]) / avg[j];

         maxi[j] = (maxi[j] > im) ? maxi[j] : im;
         printf("%10d ", partweights[i*_nconst+j]);
         }
     }
 for (j=0; j<_nconst; ++j)
     maxall = (maxi[j] > maxall) ? maxi[j] : maxall;
 printf("\n MaxImbals are (as %%): %.3lf", 100.0*maxall);
 printf("\n      ");
 for (i=0; i<_nconst; ++i)
     printf("%10.1lf ", 100.0*maxi[i]);
 printf("\n");
 free(maxi);
 free(avg);
}


int main(int argc, char *argv[])
{
PaToH_Parameters args;
int             _c, _n, _nconst, *cwghts, *nwghts,
                *xpins, *pins, *partvec, cut, *partweights;


 if (argc<3) {
     fprintf(stderr, "usage: %s <filename> <#parts>\n", argv[0]);
     exit(1);
 }


PaToH_Read_Hypergraph(argv[1], &_c, &_n, &_nconst, &cwghts,
                      &nwghts, &xpins, &pins);

printf("Hypergraph %10s -- #Cells=%6d  #Nets=%6d  #Pins=%8d #Const=%2d\n",
       argv[1], _c, _n, xpins[_n], _nconst);

PaToH_Initialize_Parameters(&args, PATOH_CONPART,
                            PATOH_SUGPARAM_DEFAULT);

args._k = atoi(argv[2]);
partvec = (int *) malloc(_c*sizeof(int));
partweights = (int *) malloc(args._k*_nconst*sizeof(int));
PaToH_Alloc(&args, _c, _n, _nconst, cwghts, nwghts,
            xpins, pins);


/* args.crs_alg = PATOH_CRS_ABS;
args.crs_useafter = 2;
  args.crs_useafteralg = PATOH_CRS_MNC;
  args.crs_coarsenper = 2; */
args.crs_maxallowedcellwmult = 0.80;
args.initp_ghg_trybalance = 0;
args.outputdetail = PATOH_OD_HIGH;


PaToH_Part(&args, _c, _n, _nconst, 0, cwghts, nwghts,
           xpins, pins, NULL, partvec, partweights, &cut);

printf("%d-way cutsize is: %d\n", args._k, cut);


PrintInfo(args._k, partweights,  cut, _nconst);


free(cwghts);      free(nwghts);
free(xpins);       free(pins);
free(partweights); free(partvec);

PaToH_Free();
return 0;
}
