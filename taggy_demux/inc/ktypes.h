#ifndef _KTPYES_H_
#define _KTPYES_H_

#include <inttypes.h>
#include "time.h"

#define INPUT_TYPE_NONE                    0
#define INPUT_TYPE_FASTQ                   1
#define INPUT_TYPE_SAM                     2

#define OUTPUT_TYPE_FLAMES                 0
#define OUTPUT_TYPE_SCNANO                 1
#define OUTPUT_TYPE_SAM                    2

#define JOIN_LEVEL_0                       0
#define JOIN_LEVEL_1                       1
#define JOIN_LEVEL_2                       2
#define JOIN_LEVEL_3                       3

#define MAX_PRIMER_SIZE                 1024

typedef struct{
    const char *inputFile;
    const char *outputFile;
    const char *fileH;
    const char *watermarkFile;
    const char *innerCodebookFile;
    const char *qualParametersFile;
    const char *substitutionMatrixFile;
    double Ps, Pd, Pi;
    double thresh;
    int q,n,k,m;
    int u;
    int iter;
    int verbosity;
    int Imax;
}programArguments;

typedef struct{
    double qualScale;
    double qual1Offset;
    double avgQual;
}qualParameters;

typedef struct{
    int imax, u, messageLength, fivePrimeLength, threePrimeLength, decoderInputLength;
    int *watermark;
    int *fivePrimeAdapter;
    int *threePrimeAdapter;
    int **innerCodebook;
    double substitutionMatrix[16];
    double pd;
    double **emissionMatrix;
    qualParameters qParams;
}wmk_config_t;

typedef struct{
    int start_pos;
    int end_pos;
    int reverse;
    char *cigar;
}hit_t;

typedef struct{
    char   *dec;
    double  conf;
    double  norm;
    int     best3p;
    double  best3p_norm;
    int     best5p;
    double  best5p_norm;
}demux_res_t;

typedef struct{
  ssize_t         seq_index;
  char           *line;
  size_t          linelen;
  char           *defline;
  char           *defline_short;
  char           *seq;
  char           *qv;
  char           *tags;
  long int        average_qv;
  ssize_t         seq_size;
  clock_t         start_read_time;
}read_t;

typedef struct{
  ssize_t   reads_total;
}stats_t;

typedef struct{
  FILE     *ifp;
  FILE     *ofp[512];
  char     *ofnames[512];
}files_t;

typedef struct{
  size_t    linelen;
  char     *seq, *fivePrime, *threePrime, *id;
  ssize_t   seq_size;
  int       num;
  int       anchor_5p;
}primer_t;

typedef struct{
  char     *args[2];
  int       VERBOSE;
  int       INPUT_TYPE, OUTPUT_TYPE, NUM_PRIMERS, NUM_THREADS;
  int       PRINT_START, START_FOR_REV, JOIN_LEVEL;
  int       PARTIAL_ENDS;
  int       PRESERVE_TAGS;
  int       MAX_READ_BASES;
  double    MAX_EDIT_DISTANCE;
  primer_t *primers;
  char     *OUTPUT_DIR;
  char     *BC_WHITELIST_FILE;
  int       MIN_OUT_LEN, UMI_NOM_LEN, UMI_CONTEXT_FROM, UMI_CONTEXT_TO;
  int      *TRUE_BC[3];
//int       REVERSE_TSO;
//int       COLLAPSE_UMI;
//int       COMPRESS_UMI;
//int       TRIM_T;
}argopts_t;

typedef struct{
  stats_t                  stats;
  files_t                 *files;
  const read_t            *read;
  const argopts_t         *p;
  const programArguments  *pArgs;
  wmk_config_t            *wcfg;
  int                      thread_num;
}globals_t;

#endif
