#ifndef SEQUENCING_CHANNEL_H
#define SEQUENCING_CHANNEL_H

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <dirent.h>
#include <stdbool.h>
#include "ktypes.h"

#ifndef Log2Q
#define Log2Q 3         // GF(2^4)
#endif

#if Log2Q < 1 || Log2Q > 8
#error "Log2Q must be 1..8"
#endif

#define Q (1<<Log2Q)      // GF(Q)

#define Qchannel 4

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

#define GF_add(a, b) ((a)^(b))
#define GF_sub(a, b) ((a)^(b))
#define exp2(x) pow(2.0,x)
#define log2(x) (log(x)/log(2.0))

#define fix2float(x) ((x)*PREC)

#define INT	8/*6*/              // int part
#define DECI	15/*14-13*/          // fraction part
#define FMUL	(1 <<DECI)       // multiplier
#define PREC	(1.0/FMUL)      // precision
#define LEVELS	(1 <<(INT+DECI))

//TMP #define MIN_PROB (1e-10)

#ifdef LONGLONG
typedef long long int NTT;    // 8-byte int (for both VC and gcc)
#else
typedef int NTT;
#endif

#if Q==4
const int logq[4] = {0,0,1,2};
const int expq[3] = {1,2,3};
#elif Q==8
const int logq[8] = {0,0,1,3,2,6,4,5};
const int expq[7] = {1,2,4,3,6,7,5};
#elif Q==16
const int logq[16] = {0,0,1,4,2,8,5,10,3,14,9,7,6,13,11,12};
const int expq[15] = {1,2,4,8,3,6,12,11,5,10,7,14,15,13,9};
#endif


#if Q==2                        // please do not try this (i.e., Log2Q=1)
#define GF_mul(a, b) ((a)&(b))
#else
int GF_mul(int a, int b);
#endif

void wmk_init(programArguments *pArgs);
void wmk_config_init(const programArguments *pArgs, wmk_config_t *c);
void wmk_free();
void wmk_config_free(wmk_config_t *c);

void printUsage();
int defaultArgs(programArguments *pArgs);
int printArguments(const programArguments *pArgs);
int scanCodes(const programArguments *pArgs, const char *ThreePrimeAdapter, const char *FivePrimeAdapter, const char *inputBuffer, const char *qualBuffer, const int offset, demux_res_t *output, wmk_config_t *c);
bool areEqual(const int x[], const int y[], const int length);
int float2fix(double x);
void Q16toQ4(int source[], int dest[], int sizeSource);
void Q4toQ16(int source[], int dest[], int sizeDest);
unsigned int float2fixu(double x);
void loadQualParams(const char* qualParamsFile, wmk_config_t *c);

#endif // SEQUENCING_CHANNEL_H
